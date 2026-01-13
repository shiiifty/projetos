#include "es.hpp"

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "../common/common.hpp"
#include "../common/constants.hpp"

using namespace std;

// Flag global usada para terminar o servidor de forma controlada
extern bool is_shutting_down;

// Contador global para IDs de eventos
int event_id = 0;

bool parse_user_password(string user_password) {
  if (user_password.length() != 8) return false;

  for (char c : user_password)
    if (!isdigit(c) && !isalpha(c)) return false;

  return true;
}

time_t to_time_t(const string& date, const string& hour) {
    tm tm{};
    memset(&tm, 0, sizeof(tm));

    tm.tm_mday = stoi(date.substr(0, 2));
    tm.tm_mon  = stoi(date.substr(3, 2)) - 1;   // meses começam em 0
    tm.tm_year = stoi(date.substr(6, 4)) - 1900;

    tm.tm_hour = stoi(hour.substr(0, 2));
    tm.tm_min  = stoi(hour.substr(3, 2));
    tm.tm_sec = 0;

    return mktime(&tm);
}

bool write_all(int fd, const char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, buf + total, len - total);
        if (n <= 0) return false;
        total += n;
    }
    return true;
}


int open_udp_socket(const std::string& port) {
    int fd;
    struct addrinfo hints, *res;
    int errcode;

    // Criação do socket UDP IPv4
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket UDP");
        exit(1);
    }

    // Configuração do getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM;   // UDP
    hints.ai_flags    = AI_PASSIVE;   // aceita ligações em qualquer IP local

    // Obtenção do endereço associado à porta
    errcode = getaddrinfo(NULL, port.c_str(), &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    // Associação do socket à porta
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind UDP");
        exit(1);
    }

    freeaddrinfo(res);
    return fd;
}

int open_tcp_socket(const std::string& port) {
    int fd;
    struct addrinfo hints, *res;
    int errcode;

    // Criação do socket TCP IPv4
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket TCP");
        exit(1);
    }

    // Configuração do getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_flags    = AI_PASSIVE;    // aceita ligações em qualquer IP local

    // Obtenção do endereço associado à porta
    errcode = getaddrinfo(NULL, port.c_str(), &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    // Associação do socket à porta
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind TCP");
        exit(1);
    }

    freeaddrinfo(res);

    // Coloca o socket em modo passivo (fila de conexões)
    if (listen(fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    return fd;
}

string ServerState::handle_login(const string& uid, const string& pass) {
    auto it = users.find(uid);

    // Utilizador não existe → regista e faz login
    if (it == users.end()) {
        UserInfo new_user;
        new_user.password = pass;
        new_user.logged_in = true;
        users[uid] = new_user;
        return "RLI REG\n";
    }

    UserInfo &user = it->second;

    // Password errada ou utilizador já autenticado
    if (user.password != pass || user.logged_in) {
        return "RLI NOK\n";
    }

    // Login bem sucedido
    user.logged_in = true;
    return "RLI OK\n";
}

string ServerState::handle_logout(const string& uid, const string& pass) {
    auto it = users.find(uid);

    // Utilizador não registado
    if (it == users.end()) return "RLO UNR\n";

    UserInfo &user = it->second;

    // Utilizador não está autenticado
    if (!user.logged_in) return "RLO NOK\n";

    // Password incorreta
    if (user.password != pass) return "RLO WRP\n";

    // Logout bem sucedido
    user.logged_in = false;
    return "RLO OK\n";
}

string ServerState::handle_unregister(const string& uid, const string& pass) {
    auto it = users.find(uid);

    // Utilizador não registado
    if (it == users.end()) return "RUR UNR\n";

    UserInfo &user = it->second;

    // Utilizador não está autenticado
    if (!user.logged_in) return "RUR NOK\n";

    // Password incorreta
    if (user.password != pass) return "RUR WRP\n";

    // Remove utilizador do sistema
    user.logged_in = false;
    users.erase(uid);
    return "RUR OK\n";
}

string ServerState::handle_create(ServerState& state, const string& user_id, const string& password,
        const string& name, const string& event_date, const string& event_hours,
        const string& num_attendees, const string& f_name, const string& fsize,
        const vector<char>& f_data) {

    // Geração de novo ID de evento
    event_id++;

    ostringstream eid_ss;
    eid_ss << setw(3) << setfill('0') << event_id;
    string eid = eid_ss.str();

    // Verifica se utilizador existe e está autenticado
    auto it = users.find(user_id);
    if (it == users.end() || !it->second.logged_in) {
        event_id--;
        return "RCE NLG " + eid + "\n";
    }

    // Password incorreta
    if (it->second.password != password) {
        event_id--;
        return "RCE WRP " + eid + "\n";
    }

    // Número inválido de participantes
    int number_attendees = stoi(num_attendees);
    if (number_attendees > 999 || number_attendees < 10) {
        event_id--;
        return "RCE NOK " + eid + "\n";
    }

    // Nome do evento inválido
    if (name.length() > 10) {
        event_id--;
        return "RCE NOK " + eid + "\n";
    }

    for (char c : name) {
        if (!isdigit(c) && !isalpha(c)) {
            event_id--;
            return "RCE NOK " + eid + "\n";
        }
    }

    // Verifica se a data do evento ainda não passou
    /*time_t now = time(nullptr);
    time_t ev_time = to_time_t(event_date, event_hours);
    if (ev_time < now) return "RCE NOK " + eid + "\n";*/

    // Criação e armazenamento do evento
    Event ev;
    ev.eid = eid;
    ev.owner_uid = user_id;
    ev.name = name;
    ev.date_str = event_date;
    ev.hour_str = event_hours;
    ev.attendance_size = number_attendees;
    ev.seats_reserved = 0;
    ev.file_name = f_name;
    ev.file_data = f_data;

    state.events_by_eid[stoi(eid)] = ev;

    return "RCE OK " + eid + "\n";
}

string ServerState::handle_myEvents(ServerState& state, const string& uid, const string& password) {
    string reply = "";

    // Percorre todos os eventos do sistema
    for (const auto& pair : events_by_eid) {
        const Event& ev = pair.second;

        // Apenas eventos do utilizador
        if (ev.owner_uid == uid) {
            time_t now = time(nullptr);
            time_t ev_time = to_time_t(ev.date_str, ev.hour_str);

            // Determina o estado do evento
            string state;
            if (ev_time < now) state = "0";
            else if (ev.closed_by_owner) state = "3";
            else if (!ev.sold_out) state = "1";
            else if (ev.sold_out) state = "2";
            
            reply += " " + ev.eid + " " + state;
        }
    }

    // Valida autenticação e existência de eventos
    if (reply.empty()) return "RME NOK\n";
    else if (!users[uid].logged_in) return "RME NLG\n";
    else if (users[uid].password != password) return "RME WRP\n";

    return "RME OK" + reply + "\n";
}

string ServerState::handle_reserve(ServerState& state, const string& uid,
            const string& password, const string& eid, int value) {

    // Utilizador não autenticado
    if (!users[uid].logged_in) return "RRI NLG\n";

    // Password incorreta
    if (password != users[uid].password) return "RRI WRP\n";

    // Evento não existe
    auto it = events_by_eid.find(stoi(eid));
    if (it == events_by_eid.end()) return "RRI NOK\n";

    Event& ev = events_by_eid[stoi(eid)];

    // Capacidade disponível no evento
    int available_capacity = ev.attendance_size - ev.seats_reserved;

    time_t now = time(nullptr);
    time_t ev_time = to_time_t(ev.date_str, ev.hour_str);

    // Evento esgotado
    if (ev.sold_out) return "RRI SLD\n";

    // Número de bilhetes inválido ou excede a capacidade
    if (available_capacity < value && 0 < value && value < 999) {
        return "RRI REJ " + to_string(available_capacity) + "\n";
    }

    // Evento já ocorreu
    if (ev_time < now) return "RRI PST\n";

    // Evento fechado pelo organizador
    if (ev.closed_by_owner) return "RRI CLS\n";

    // Reserva aceite
    ev.seats_reserved += value;

    // Marca evento como esgotado se atingir capacidade máxima
    if (ev.seats_reserved == ev.attendance_size)
        ev.sold_out = true;

    // Guarda informação da reserva do utilizador
    ReservationInfo reservevation;
    reservevation.eid = eid;
    reservevation.number_of_tickets = value;
    reservevation.date_day = ev.date_str;
    reservevation.date_hour = ev.hour_str;
    reservations_by_user[uid].push_back(reservevation);

    return "RRI ACC\n";
}

string ServerState::handle_myReservations(const string& uid, const string& password) {
    string reply = "";
    int counter = 0;

    // Percorre reservas do utilizador (máx. 50)
    for (const auto& pair : reservations_by_user) {
        if (pair.first == uid) {
            for (ReservationInfo reservation : pair.second) {
                if (counter >= 50) break;

                reply += reservation.eid + " " + reservation.date_day + " " +
                    reservation.date_hour + ":00 " +
                    to_string(reservation.number_of_tickets) + " ";
                counter++;
            }
        }
    }

    // Valida existência de reservas e autenticação
    if (reply.empty()) return "RMR NOK\n";
    else if (!users[uid].logged_in) return "RMR NLG\n";
    else if (users[uid].password != password) return "RMR WRP\n";

    // Remove espaço final
    reply.pop_back();
    return "RMR OK " + reply + "\n";
}

string ServerState::handle_close(const string& uid, const string& password, const string& eid) {
    auto event = events_by_eid.find(stoi(eid));
    auto user = users.find(uid);

    time_t now = time(nullptr);
    time_t ev_time = to_time_t(events_by_eid[stoi(eid)].date_str,
                               events_by_eid[stoi(eid)].hour_str);

    // Evento esgotado
    if (events_by_eid[stoi(eid)].sold_out) return "RCL SLD\n";

    // Utilizador não autenticado
    if (!users[uid].logged_in) return "RCL NLG\n";

    // Utilizador inexistente ou password incorreta
    if (user == users.end() || users[uid].password != password) return "RCL NOK\n";

    // Evento não existe
    if (event == events_by_eid.end()) return "RCL NOE\n";

    // Evento já ocorreu
    if (ev_time < now) return "RCL PST\n";

    // Utilizador não é o dono do evento
    if (events_by_eid[stoi(eid)].owner_uid != uid) return "RCL EOW\n";

    // Evento já fechado
    if (events_by_eid[stoi(eid)].closed_by_owner) return "RCL CLO\n";

    // Fecha o evento
    events_by_eid[stoi(eid)].closed_by_owner = true;

    return "RCL OK\n";
}

string ServerState::handle_changePass(const string& uid,
                                      const string& old_password,
                                      const string& new_password) {
    auto it = users.find(uid);

    // Utilizador não existe
    if (it == users.end()) return "RCP NID\n";

    // Utilizador não autenticado
    if (!users[uid].logged_in) return "RCP NLG\n";

    // Password antiga incorreta ou nova password inválida
    if (users[uid].password != old_password ||
        !parse_user_password(new_password))
        return "RCP NOK\n";

    // Atualiza a password
    users[uid].password = new_password;

    return "RCP OK\n";
}

string ServerState::handle_list() {
    string reply = "";

    // Percorre todos os eventos
    for (const auto& pair : events_by_eid) {
        Event ev = pair.second;
        time_t now = time(nullptr);
        time_t ev_time = to_time_t(ev.date_str, ev.hour_str);

        // Determina o estado do evento
        string state;
        if (!ev.sold_out && ev_time > now) state = "1";
        else if (ev_time < now) state = "0";
        else if (ev.sold_out && ev_time > now) state = "2";
        else if (ev.closed_by_owner) state = "3";

        reply += " " + ev.eid + " " + ev.name + " " +
                 state + " " + ev.date_str + " " + ev.hour_str;
    }

    // Nenhum evento disponível
    if (reply.empty()) return "RLS NOK\n";

    return "RLS OK" + reply + "\n";
}

string ServerState::handle_show(const string &eid_str) {
    int eid = stoi(eid_str);

    // Evento não existe
    auto it = events_by_eid.find(eid);
    if (it == events_by_eid.end()) {
        return "RSE NOK\n";
    }

    Event &ev = it->second;

    // Tamanho do ficheiro associado ao evento
    size_t fsize = ev.file_data.size();

    // Cabeçalho da resposta (metadados do evento)
    string header =
        "RSE OK " + ev.owner_uid + " " + ev.name + " " + ev.date_str + " " +
        ev.hour_str + " " + to_string(ev.attendance_size) + " " +
        to_string(ev.seats_reserved) + " " + ev.file_name + " " +
        to_string(fsize) + " ";

    string reply;
    reply.reserve(header.size() + fsize + 1);

    // Constrói resposta: header + ficheiro binário
    reply += header;
    reply.append(ev.file_data.data(), fsize);
    reply += "\n";

    return reply;
}

void handle_udp(ServerState &state, bool verbose) {
    char buffer[2048];
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Recebe mensagem UDP do cliente
    ssize_t n = recvfrom(state.udp_fd, buffer, sizeof(buffer)-1, 0,
        (struct sockaddr*)&client_addr, &addrlen);

    // Obtém IP e porto do cliente
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(client_addr.sin_port);

    if (n <= 0) return;

    buffer[n] = '\0';

    // Extrai comando, UID e password
    char cmd[8], uid[8], pass[32];
    sscanf(buffer, "%s %s %s", cmd, uid, pass);

    string reply;

    // Login
    if (strcmp(cmd, "LIN") == 0) {
        if (verbose)
            cout << string(uid) + " login " + string(ip) + " " + to_string(port) + "\n";

        reply = state.handle_login(uid, pass);
    }
    // Logout
    else if (strcmp(cmd, "LOU") == 0) {
        if (verbose)
            cout << string(uid) + " logout " + string(ip) + " " + to_string(port) + "\n";

        reply = state.handle_logout(uid, pass);
    }
    // Unregister
    else if (strcmp(cmd, "UNR") == 0) {
        if (verbose)
            cout << string(uid) + " unregister " + string(ip) + " " + to_string(port) + "\n";

        reply = state.handle_unregister(uid, pass);
    }
    // Lista eventos do utilizador
    else if (strcmp(cmd, "LME") == 0) {
        if (verbose)
            cout << string(uid) + " myEvents " + string(ip) + " " + to_string(port) + "\n";

        reply = state.handle_myEvents(state, uid, pass);
    }
    // Lista reservas do utilizador
    else if (strcmp(cmd, "LMR") == 0) {
        if (verbose)
            cout << string(uid) + " myReservations " + string(ip) + " " + to_string(port) + "\n";

        reply = state.handle_myReservations(uid, pass);
    }
    // Comando inválido
    else {
        reply = "ERR\n";
    }

    // Envia resposta ao cliente
    sendto(state.udp_fd, reply.c_str(), reply.size(), 0,
        (struct sockaddr*)&client_addr, addrlen);
}

void handle_tcp(ServerState &state, bool verbose) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    // Aceita uma nova conexão TCP
    int conn_fd = accept(state.tcp_fd, (struct sockaddr*)&addr, &addrlen);

    // Obtém IP e porto do cliente
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr.sin_port);

    if (conn_fd < 0) return;  // erro ao aceitar

    char buffer[4096];
    ssize_t n;

    string msg;

    // Lê dados do cliente até encontrar um '\n' (fim de mensagem)
    while ((n = read(conn_fd, buffer, sizeof(buffer))) > 0) {
        msg.append(buffer, n);                    
        if (!msg.empty() && msg.back() == '\n')  
            break;
    }

    if (n <= 0) {  // conexão fechada ou erro
        close(conn_fd);
        return;
    }

    // Extrai comando (3 primeiros caracteres)
    char cmd[4];
    sscanf(msg.c_str(), "%3s", cmd);

    string reply;

    // =============================
    // CRIAÇÃO DE EVENTO (CRE)
    // =============================
    if (strcmp(cmd, "CRE") == 0) {
        vector<size_t> spaces;

        // Função para identificar as posições dos espaços no pedido
        auto recompute_spaces = [&]() {
            spaces.clear();
            for (size_t i = 0; i < msg.size(); ++i) {
                if (msg[i] == ' ') {
                    spaces.push_back(i);
                    if (spaces.size() == 9) break;  // só precisamos dos primeiros 9 espaços
                }
            }
        };

        recompute_spaces();

        // Ler mais dados até termos todos os espaços (eventualmente inclui o ficheiro)
        while (spaces.size() < 9) {
            n = read(conn_fd, buffer, sizeof(buffer));
            if (n <= 0) { close(conn_fd); return; }
            msg.append(buffer, n);
            recompute_spaces();
        }

        auto sub = [&](size_t start, size_t end) { return msg.substr(start, end - start); };

        // Extrai campos individuais a partir das posições dos espaços
        string uid     = sub(spaces[0] + 1, spaces[1]);
        string pass    = sub(spaces[1] + 1, spaces[2]);
        string name    = sub(spaces[2] + 1, spaces[3]);
        string date    = sub(spaces[3] + 1, spaces[4]);
        string time    = sub(spaces[4] + 1, spaces[5]);
        string attendees_str = sub(spaces[5] + 1, spaces[6]);
        string fname   = sub(spaces[6] + 1, spaces[7]);
        string fsize_str = sub(spaces[7] + 1, spaces[8]);

        // Tamanho do ficheiro
        size_t fsize = stoul(fsize_str);
        size_t file_start = spaces[8] + 1;

        // Ler todo o conteúdo do ficheiro
        while (msg.size() < file_start + fsize + 1) {
            n = read(conn_fd, buffer, sizeof(buffer));
            if (n <= 0) { close(conn_fd); return; }
            msg.append(buffer, n);
        }

        vector<char> file_data(msg.begin() + file_start, msg.begin() + file_start + fsize);

        if (verbose) cout << uid << " create " << ip << " " << port << "\n";

        
        // Chama função que cria o evento e devolve resposta
        reply = state.handle_create(state, uid, pass, name, date, time,
                                    attendees_str, fname, fsize_str, file_data);
    }

    // =============================
    // RESERVA (RID)
    // =============================
    else if (strcmp(cmd, "RID") == 0) {
        stringstream ss(msg);
        string cmd, uid, pass, eid;
        int value;
        ss >> cmd >> uid >> pass >> eid >> value;

        if (verbose) cout << uid + " reserve " + ip + " " + to_string(port) + "\n";

        reply = state.handle_reserve(state, uid, pass, eid, value);
    }

    // =============================
    // FECHAR EVENTO (CLS)
    // =============================
    else if (strcmp(cmd, "CLS") == 0) {
        stringstream ss(msg);
        string cmd, uid, pass, eid;
        ss >> cmd >> uid >> pass >> eid;

        if (verbose) cout << uid + " close " + ip + " " + to_string(port) + "\n";

        reply = state.handle_close(uid, pass, eid);
    }

    // =============================
    // MUDAR PASSWORD (CPS)
    // =============================
    else if (strcmp(cmd, "CPS") == 0) {
        stringstream ss(msg);
        string cmd, uid, old_password, new_password;
        ss >> cmd >> uid >> old_password >> new_password;

        if (verbose) cout << uid + " changePass " + ip + " " + to_string(port) + "\n";

        reply = state.handle_changePass(uid, old_password, new_password);
    }

    // =============================
    // LISTAR EVENTOS (LST)
    // =============================
    else if (strcmp(cmd, "LST") == 0) {
        if (verbose) cout << "list " << ip << " " + to_string(port) << "\n";

        reply = state.handle_list();
    }

    // =============================
    // MOSTRAR DETALHES DE UM EVENTO (SED)
    // =============================
    else if (strcmp(cmd, "SED") == 0) {
        stringstream ss(msg);
        string cmd, eid;
        ss >> cmd >> eid;

        if (verbose) cout << eid + " show " + ip + " " + to_string(port) << "\n";

        reply = state.handle_show(eid);
    }

    // Escreve resposta TCP de volta para o cliente
    if (!write_all(conn_fd, reply.c_str(), reply.size())) {
        perror("write_all TCP");
    }

    close(conn_fd);  // Fecha a conexão
}


int main(int argc, char *argv[]) {
    // Cria a configuração do servidor com base nos argumentos da linha de comando
    ServerConfig config(argc, argv);

    // Configura os handlers de sinais (ex: SIGINT) para permitir shutdown seguro
    setup_signal_handlers();

    // Cria o estado do servidor (users, events, reservas)
    ServerState state;

    // Abre sockets UDP e TCP na porta especificada
    state.udp_fd = open_udp_socket(config.port);
    state.tcp_fd = open_tcp_socket(config.port);

    // Determina o maior descritor de ficheiro para usar no select
    int max_fd = max(state.udp_fd, state.tcp_fd);

    // Loop principal do servidor
    while (!is_shutting_down) {
        fd_set readfds;
        FD_ZERO(&readfds);                  // Limpa o conjunto
        FD_SET(state.udp_fd, &readfds);     // Adiciona UDP
        FD_SET(state.tcp_fd, &readfds);     // Adiciona TCP

        // Espera por atividade em qualquer socket
        int r = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (r < 0) {
            if (errno == EINTR) continue;  // sinal interrompe select, apenas continuar
            perror("select");
            break;
        }

        // Se houve atividade no socket UDP
        if (FD_ISSET(state.udp_fd, &readfds)) {
            try {
                handle_udp(state, config.verbose);
            } catch (...) {
                cerr << "Error handling UDP" << endl;
            }
        }

        // Se houve atividade no socket TCP
        if (FD_ISSET(state.tcp_fd, &readfds)) {
            try {
                handle_tcp(state, config.verbose);
            } catch (...) {
                cerr << "Error handling TCP" << endl;
            }
        }
    }

    // Ao desligar, garante que todos os utilizadores logados são "logoutados"
    for (auto &pair: state.users) {
        auto &user = pair.second;
        if (user.logged_in) {
            user.logged_in = false;
        }
    }

    cout << "Server shutting down..." << endl;
    return EXIT_SUCCESS;
}

// Construtor da configuração do servidor, lê argumentos da linha de comando
ServerConfig::ServerConfig(int argc, char *argv[])
    : port(DEFAULT_PORT), verbose(false) {
    programPath = argv[0];
    int opt;

    // getopt processa opções: -p porta, -v verbose
    while ((opt = getopt(argc, argv, "-p:vhr")) != -1) {
        switch (opt) {
            case 'p':  // porta
                port = string(optarg);
                break;
            case 'v':  // modo verbose
                verbose = true;
                break;
            default:
                cerr << endl;
                exit(EXIT_FAILURE);
        }
    }

    // Valida se o número da porta é aceitável
    validate_port_number(port);
}