#include "user.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>



#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "commands.hpp"


using namespace std;

ssize_t read_n_bytes(int fd, char *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t r = read(fd, buf + total, n - total);
        if (r <= 0) {
            return r;
        }
        total += r;
    }
    return total;
}

int open_udp_socket(const string &host, const string &port,
                    struct addrinfo **udp_res_out) {
    int fd, errcode;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;     
    hints.ai_socktype = SOCK_DGRAM;   
    hints.ai_flags    = AI_NUMERICSERV;

    errcode = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (errcode != 0) {
        cerr << "getaddrinfo UDP: " << gai_strerror(errcode) << endl;
        exit(1);
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        perror("socket UDP");
        freeaddrinfo(res);
        exit(1);
    }

    *udp_res_out = res;   
    return fd;
}


int open_tcp_connection(const string &host, const string &port) {
    int fd, errcode;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;       
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags    = AI_NUMERICSERV;

    errcode = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (errcode != 0) {
        cerr << "getaddrinfo TCP: " << gai_strerror(errcode) << endl;
        exit(1);
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        perror("socket TCP");
        freeaddrinfo(res);
        exit(1);
    }

    struct timeval read_timeout;
    read_timeout.tv_sec  = TCP_READ_TIMEOUT_SECONDS;
    read_timeout.tv_usec = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
                   sizeof(read_timeout)) < 0) {
        perror("setsockopt TCP read timeout");
        close(fd);
        freeaddrinfo(res);
        exit(1);
    }

    struct timeval write_timeout;
    write_timeout.tv_sec  = TCP_WRITE_TIMEOUT_SECONDS;
    write_timeout.tv_usec = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &write_timeout,
                   sizeof(write_timeout)) < 0) {
        perror("setsockopt TCP write timeout");
        close(fd);
        freeaddrinfo(res);
        exit(1);
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        close(fd);
        freeaddrinfo(res);
        exit(1);
    }

    freeaddrinfo(res);
    return fd;
}

ssize_t send_udp_and_wait_reply(int udpfd, const std::string &message,
                                string &reply, struct sockaddr *server_addr, 
                                socklen_t server_addr_len) {
    char buffer[1024];

    int triesLeft = UDP_RESEND_TRIES;

    while (triesLeft > 0) {
        --triesLeft;

        ssize_t sent = sendto(udpfd, message.c_str(), message.size(),
                              0, server_addr, server_addr_len);
        if (sent < 0) {
            perror("sendto");
            return -1;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(udpfd, &readfds);

        struct timeval tv;
        tv.tv_sec  = UDP_TIMEOUT_SECONDS;
        tv.tv_usec = 0;

        // Aguarda por dados no socket UDP ou até ao timeout
        int r = select(udpfd + 1, &readfds, nullptr, nullptr, &tv);
        if (r < 0) {
            perror("select");
            return -1;
        }

        // Timeout: não foi recebida resposta
        if (r == 0) {

            if (triesLeft == 0) {

                return -1;
            }

            continue;
        }

        // Há dados disponíveis para leitura no socket UDP
        if (FD_ISSET(udpfd, &readfds)) {
            socklen_t from_len = server_addr_len;
            // Recebe a resposta do servidor
            ssize_t n = recvfrom(
                udpfd,
                buffer,
                sizeof(buffer) - 1,
                0,
                server_addr,
                &from_len
            );
            if (n < 0) {
                perror("recvfrom");
                return -1;
            }

            // Termina a resposta como string
            buffer[n] = '\0';
            // Copia a resposta para a string reply
            reply.assign(buffer, n);

            // Retorna o número de bytes recebidos
            return n;
        }
    }

    return -1;
}

static bool read_byte(int fd, char &c) {
    ssize_t r = read(fd, &c, 1);
    return r == 1;
}

ssize_t send_tcp_and_wait_reply(int tcpfd, const string &message, string &reply) {
    // 1) Enviar toda a mensagem (pode ser necessário escrever em várias chamadas a write)
    size_t left = message.size();
    const char *ptr = message.data();
    while (left > 0) {
        ssize_t w = write(tcpfd, ptr, left);
        if (w <= 0) { perror("write"); return -1; }
        left -= (size_t)w;
        ptr  += w;
    }

    // Limpa a resposta antes de começar a ler
    reply.clear();

    // 2) Caso NÃO seja o comando SED: a resposta é uma linha terminada por '\n'
    if (message.rfind("SED", 0) != 0) {
        char c;
        while (read_byte(tcpfd, c)) {
            reply += c;
            if (c == '\n') break;
        }
        return (ssize_t)reply.size();
    }

    // 3) Caso seja o comando SED:
    //    a resposta inclui um header com vários campos (separados por espaços)
    //    e, no caso OK, inclui também dados binários de um ficheiro

    // Buffer para guardar o header lido byte a byte
    string header;
    header.reserve(256);

    int spaces = 0;
    char c;

    // Lê até encontrar 10 espaços
    // (cmd, status, uid, name, date, hour, att, resv, fname, fsize)
    while (spaces < 10) {
        if (!read_byte(tcpfd, c)) { perror("read header"); return -1; }
        header += c;
        if (c == ' ') spaces++;
    }

    {
        // Faz parsing do header para extrair os campos
        istringstream iss(header);
        string cmd, status, uid, name, date, hour, fname;
        int att, resv;
        size_t fsize;

        // Lê o comando de resposta e o estado
        iss >> cmd >> status;

        // Se não for "RSE OK", devolve apenas o header como resposta
        // (ex.: "RSE NOK\n" ou "RSE ERR\n", dependendo do protocolo)
        if (cmd != "RSE" || status != "OK") {

            reply = header; 
            return (ssize_t)reply.size();
        }

        // Lê os restantes campos do header, incluindo o tamanho do ficheiro
        iss >> uid >> name >> date >> hour >> att >> resv >> fname >> fsize;

        // 4) Lê exatamente fsize bytes de dados binários do ficheiro
        vector<char> fdata(fsize);
        if (fsize > 0) {
            ssize_t rr = read_n_bytes(tcpfd, fdata.data(), fsize);
            if (rr != (ssize_t)fsize) { perror("read fdata"); return -1; }
        }

        // 5) Lê o '\n' final que termina a mensagem do servidor
        if (!read_byte(tcpfd, c)) { perror("read final newline"); return -1; }

        // 6) Constrói o nome do ficheiro de saída:
        //    remove eventuais caminhos (fica só o "basename")
        string base = fname;
        auto pos = base.find_last_of("/\\");
        if (pos != string::npos) base = base.substr(pos + 1);

        // Prefixa o ficheiro guardado com "received_"
        string outname = "received_" + base;

        // 7) Guarda o ficheiro recebido no disco (modo binário)
        ofstream out(outname, ios::binary);
        
        if (!out) {
            cerr << "Error creating output file: " << outname << "\n";
            return -1;
        }

        out.write(reinterpret_cast<const char*>(fdata.data()),
                static_cast<std::streamsize>(fdata.size()));

        // Verifica se houve erro na escrita
        if (!out) {
            cerr << "Error writing output file: " << outname << "\n";
            return -1;
        }
        out.close();

        // 8) Prepara uma resposta "amigável" para o cliente,
        //    incluindo o nome do ficheiro guardado localmente
        ostringstream oss;
        oss << "RSE OK " << uid << " " << name << " " << date << " " << hour
            << " " << att << " " << resv << " " << fname << " " << fsize
            << " " << outname << "\n";
        reply = oss.str();
    }

    // Retorna o tamanho da resposta final colocada em reply
    return (ssize_t)reply.size();
}

UserConfig::UserConfig(int argc, char* argv[])
    : host(DEFAULT_HOSTNAME), port(DEFAULT_PORT){
        // Guarda o caminho do programa (argv[0])
    program_path = argv[0];
    int opt;

    // Processa as opções da linha de comandos usando getopt
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
            // Opção -n: define o hostname do servidor
            case 'n':
                host = string(optarg);
                break;

            // Opção -p: define a porta do servidor
            case 'p':
                port = string(optarg);
                break; 
        }
    }
}

int main(int argc, char* argv[]) {
    // Variáveis para guardar o utilizador autenticado
    string user_id;
    string password;

    // Indica se o utilizador está autenticado
    bool logged_in = false;

    // Lê a configuração do utilizador (hostname e porta) a partir da linha de comandos
    UserConfig config(argc, argv);

    struct addrinfo *udp_res = nullptr;
    int udp_fd = open_udp_socket(config.host, config.port, &udp_res);

    cout << "User ligado ao ES em " << config.host << ":" << config.port << endl;

    // Ciclo principal de leitura de comandos do utilizador
    while (true) {
        cout << "> ";
        string line;

        // Lê uma linha da entrada padrão
        if (!getline(cin, line)) {
            break; 
        }

        // Ignora linhas vazias
        if (line.empty()) continue;

        // Separa o comando dos argumentos
        size_t pos = line.find(' '); 
        string command = line.substr(0, pos);      
        string args = line.substr(pos + 1);

        // ---------- LOGIN ----------
        if (command == "login") {
            auto space_pos = args.find(' ');
            user_id = args.substr(0, space_pos);
            password = args.substr(space_pos+1);

            // Constrói a mensagem de login
            string msg = loginHandler(user_id, password);

            if (msg.empty()) continue;
            
            string reply;
            // Envia o pedido via UDP e aguarda resposta
            ssize_t n = send_udp_and_wait_reply(
                udp_fd,
                msg,
                reply,
                udp_res->ai_addr,     
                udp_res->ai_addrlen    
            );

            if (n < 0) {
                std::cerr << "Timeout ou erro ao falar com o ES via UDP.\n";
                continue;
            }
            
            // Processa a resposta do servidor
            if (reply == "RLI OK\n")  {
                logged_in = true;
                cout << "successful login\n";
            }
            else if (reply == "RLI NOK\n") cout << "incorrect login attempt\n";
            else if (reply == "RLI REG\n") {
                logged_in = true;
                cout << "new user registered\n";
            }
        }   

        // ---------- LOGOUT ----------
        else if (command == "logout") {
            // Constrói a mensagem de logout 
            string msg = logoutHandler(user_id, password);
            string reply;
            ssize_t n = send_udp_and_wait_reply(
                udp_fd,
                msg,
                reply,
                udp_res->ai_addr,      
                udp_res->ai_addrlen    
            );
            
            if (reply == "RLO OK\n") {
                logged_in = false;
                cout << "successful logout\n";
            }
            else if (reply == "RLO NOK\n") cout << "user not logged in\n";
            else if (reply == "RLO UNR\n") cout << "unknown user\n";
            else if (reply == "RLO WRP\n") cout << "incorrect user password\n";
        }

        // ---------- UNREGISTER ---------
        else if (command == "unregister") {
            // Constrói a mensagem de elimiação de utilizador
            string msg = unregisterHandler(user_id, password);
            string reply;
            ssize_t n = send_udp_and_wait_reply(
                udp_fd,
                msg,
                reply,
                udp_res->ai_addr,     
                udp_res->ai_addrlen    
            );

            if (reply == "RUR OK\n") {
                logged_in = false;
                cout << "successful unregister\n";
            }
            else if (reply == "RUR NOK\n") cout << "user not logged in\n";
            else if (reply == "RUR UNR\n") cout << "unknown user\n";
            else if (reply == "RUR WRP\n") cout << "incorrect unregister attempt\n";
        }

        // ---------- LISTAR EVENTOS DO UTILIZADOR ----------
        else if (command == "myevents" || command == "mye") {
            // Constrói a mensagem de meus eventos
            string msg = myEventsHandler(user_id, password);
            string reply;
            ssize_t n = send_udp_and_wait_reply(
                udp_fd,
                msg,
                reply,
                udp_res->ai_addr,    
                udp_res->ai_addrlen   
            );

            // Verifica se a resposta indica sucesso
            string reply1 = reply.substr(0,6);
            if (reply1 == "RME OK") {
                string reply2 = reply.substr(7);
                istringstream iss(reply2);

                string eid, state;

                // Itera sobre os eventos devolvidos
                while (iss >> eid >> state) {
                    string state_str;
                    if (state == "0") {
                        state_str = "0: Past event (closed)";
                    } 
                    else if (state == "1") {
                        state_str = "1: Accepting reservations";
                    }
                    else if (state == "2") {
                        state_str = "2: To happen - sold out";
                    }
                    else if (state == "3") {
                        state_str = "3: To happen - closed by owner";
                    }
                    cout << eid << " " << state_str << "\n";
                }
            }
            else if (reply == "RME NOK\n") cout << "user did not create any events\n";
            else if (reply == "RME NLG\n") cout << "user not logged in\n";
            else if (reply == "RME WRP\n") cout << "incorrect password\n";
        }   

        // ---------- LISTAR RESERVAS DO UTILIZADOR ----------
        else if (command == "myreservations" || command == "myr") {
            // Constrói a mensagem de minhas reservas
            string msg = myReservationsHandler(user_id, password);
            string reply;
            ssize_t n = send_udp_and_wait_reply(
                udp_fd,
                msg,
                reply,
                udp_res->ai_addr,      
                udp_res->ai_addrlen    
            );

            string reply1 = reply.substr(0,6);
            if (reply1 == "RMR OK") {
                string reply2 = reply.substr(7);
                istringstream iss(reply2);

                string eid, date, hour, num_tickets;

                // Imprime todas as reservas
                while (iss >> eid >> date >> hour >> num_tickets) {
                    cout << eid << " " << date << " " << hour << " " << num_tickets << "\n";
                }
            }
            else if (reply == "RMR NOK\n") cout << "user did not make any reservations\n";
            else if (reply == "RMR NLG\n") cout << "user not logged in\n";
            else if (reply == "RMR WRP\n") cout << "incorrect password\n";
        }

        // ---------- CRIAR EVENTO ----------
        else if (command == "create") {
            string name, event_fname, event_date, event_hours, num_attendees, left;
            int tcp_fd = open_tcp_connection(config.host, config.port);

            istringstream iss(args); 
            iss >> name >> event_fname >> event_date >> event_hours >> num_attendees;
            
            // Constrói a mensagem de criação
            string msg = createHandler(user_id, password, name, event_fname, event_date, event_hours, num_attendees);

            if (msg.empty()) {
                close(tcp_fd);
                continue;
            }
            string reply;
            ssize_t n = send_tcp_and_wait_reply(tcp_fd, msg, reply);

            // Extrai o código de resposta e o EID
            auto space_pos = reply.find(' ');
            string reply1 = reply.substr(0, space_pos);
            string reply2 = reply.substr(space_pos + 1);
            
            space_pos = reply2.find(' ');
            string reply1_left= reply2.substr(0, space_pos);
            string eid = reply2.substr(space_pos+1);

            reply = reply1 + " " + reply1_left + "\n";

            if (reply == "RCE OK\n") cout << "the event was successfully created, EID = " + eid;
            else if (reply == "RCE NOK\n") cout << "the event was not created\n";
            else if (reply == "RCE NLG\n") cout << "user not logged in\n";
            else if (reply == "RCE WRP\n") cout << "incorrect password\n";

            close(tcp_fd);
        }

        // ---------- RESERVAR LUGARES ----------
        else if (command == "reserve") {
            int tcp_fd = open_tcp_connection(config.host, config.port);
            
            string uid, pass, eid, value;

            istringstream iss(args);
            iss >> eid >> value;

            // Constrói a mensagem de reserva
            string msg = reserveHandler(user_id, password, eid, value);
            string reply;
            ssize_t n = send_tcp_and_wait_reply(tcp_fd, msg, reply);


            string reply1 = reply.substr(0,7);

            
            if (reply == "RRI ACC\n") cout << "accepted\n";
            else if (reply1 == "RRI REJ") {
                if (!reply.empty() && reply.back() == '\n') {
                    reply.pop_back();
                }
                cout << "refused," + reply.substr(8) + " seats remaining\n";
            }
            else if (reply == "RRI NOK\n") cout << "event is no longer active\n";
            else if (reply == "RRI CLS\n") cout << "event is closed\n";
            else if (reply == "RRI NLG\n") cout << "user not logged in\n";
            else if (reply == "RRI SLD\n") cout << "event is sold out\n";
            else if (reply == "RRI WRP\n") cout << "incorrect password\n";
            else if (reply == "RRI PST\n") cout << "event has already happened\n";

            close(tcp_fd);
        }

        // ---------- FECHAR EVENTO ----------
        else if (command == "close") {
            int tcp_fd = open_tcp_connection(config.host, config.port);
            
            string eid;

            istringstream iss(args);
            iss >> eid;

            // Constrói a mensagem de fechar
            string msg = closeHandler(user_id, password, eid);
            string reply;
            ssize_t n = send_tcp_and_wait_reply(tcp_fd, msg, reply);

            if (reply == "RCL OK\n") cout << "event successfully closed\n";
            else if (reply == "RCL NOK\n") cout << "user does not exist or incorrect password\n";
            else if (reply == "RCL NLG\n") cout << "user not logged in\n";
            else if (reply == "RCL NOE\n") cout << "event does not exist\n";
            else if (reply == "RCL SLD\n") cout << "event is sold out\n";
            else if (reply == "RCL EOW\n") cout << "user is not the creator of the event\n";
            else if (reply == "RCL PST\n") cout << "event has already happened\n";
            else if (reply == "RCL CLO\n") cout << "event is already closed\n";

            close(tcp_fd);
        }

        // ---------- ALTERAR PALAVRA-PASSE ----------
        else if (command == "changePass") {
            int tcp_fd = open_tcp_connection(config.host, config.port);
            
            string old_pass, new_pass;

            istringstream iss(args);
            iss >> old_pass >> new_pass;

            // Constrói a mensagem de mudança de password
            string msg = changePassHandler(user_id, old_pass, new_pass);
            string reply;
            ssize_t n = send_tcp_and_wait_reply(tcp_fd, msg, reply);

            if (reply == "RCP OK\n") {
                cout << "successful password change\n";
                password = new_pass;
            }
            else if (reply == "RCP NOK\n") cout << "incorrect password\n";
            else if (reply == "RCP NLG\n") cout << "user not logged in\n";
            else if (reply == "RCP NID\n") cout << "unknown user\n";
            close(tcp_fd);
        }

        // ---------- SAIR ----------
        else if (command == "exit") {
            if (logged_in) cout << "User must log out first\n";
            else break;
        }

        // ---------- LISTAR TODOS OS EVENTOS ----------
        else if (command == "list") {
            int tcp_fd = open_tcp_connection(config.host, config.port);

            // Constrói a mensagem de listar
            string msg = listHandler();
            string reply;
            ssize_t n = send_tcp_and_wait_reply(tcp_fd, msg, reply);
            string reply1 = reply.substr(0,6);

            if (reply1 == "RLS OK") {
                string reply2 = reply.substr(7);
                istringstream iss(reply2);

                string eid, name, date, time, attendance_size, seats_reserved, state;

                while (iss >> eid >> name >> state >> date >> time) {
                    string state_str;
                    if (state == "0") {
                        state_str = "0: Past event (closed)";
                    } 
                    else if (state == "1") {
                        state_str = "1: Accepting reservations";
                    }
                    else if (state == "2") {
                        state_str = "2: To happen - sold out";
                    }
                    else if (state == "3") {
                        state_str = "3: To happen - closed by owner";
                    }

                    cout << eid << " " << name << " "<< state_str << " " << date << " " << time << "\n";
                }
            }

            else if (reply == "RLS NOK\n") cout << "there is no events\n";

            close(tcp_fd);
        }

        // ---------- MOSTRAR EVENTO ----------
        else if (command == "show") {
            int tcp_fd = open_tcp_connection(config.host, config.port);

            string eid;
            istringstream iss_args(args);
            iss_args >> eid;

            // Constrói a mensagem de mostrar
            string msg = showHandler(eid);
            string reply;
            ssize_t n = send_tcp_and_wait_reply(tcp_fd, msg, reply);

            if (reply.rfind("RSE OK", 0) == 0) {
                string uid, name, event_date, event_hour, fname, saved_file;
                int attendance_size, seats_reserved;
                size_t fsize;

                istringstream iss(reply.substr(7)); 
                iss >> uid >> name >> event_date >> event_hour
                    >> attendance_size >> seats_reserved >> fname >> fsize >> saved_file;

                cout << uid << " " << name << " " << event_date << " " << event_hour
                     << " " << attendance_size << " " << seats_reserved << " " << fsize << " " << saved_file << "\n";
            }
            else if (reply == "RSE NOK\n") cout << "there is no events\n";

            close(tcp_fd);
        }
    }

    // Liberta recursos UDP
    freeaddrinfo(udp_res);
    close(udp_fd);
    
    return 0;
}