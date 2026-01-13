#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "../common/constants.hpp"

using namespace std;

// Lê exatamente n bytes de um descritor de ficheiro
// Continua a ler até obter todos os bytes pedidos ou ocorrer um erro
// Retorna o número total de bytes lidos ou um valor <= 0 em caso de erro/EOF
ssize_t read_n_bytes(int fd, char *buf, size_t n);

// Classe responsável por armazenar a configuração do cliente
// Obtém os parâmetros a partir da linha de comandos
class UserConfig {
 public:
  char* program_path;
  string host = DEFAULT_HOSTNAME;
  string port = DEFAULT_PORT;

  // Construtor da classe UserConfig
  // Processa os argumentos da linha de comandos para configurar o cliente
  // -n <hostname> : define o endereço do servidor
  // -p <porta>    : define a porta do servidor
  UserConfig(int argc, char* argv[]);
};

// Abre um socket UDP e resolve o endereço do servidor
// Devolve o descritor do socket e o endereço resolvido via udp_res_out
int open_udp_socket(const string &host, const string &port,
                    struct addrinfo **udp_res_out);

// Abre uma ligação TCP ao servidor e configura timeouts
// Retorna o descritor do socket TCP já ligado ao servidor
int open_tcp_connection(const string &host, const string &port);

// Envia uma mensagem UDP ao servidor e aguarda pela resposta
// Implementa reenvios em caso de timeout, até ao número máximo definido
// Retorna o número de bytes recebidos ou -1 em caso de erro
ssize_t send_udp_and_wait_reply(int udpfd,
                                const std::string &message,
                                std::string &reply,
                                struct sockaddr *server_addr,
                                socklen_t server_addr_len);

// Envia uma mensagem por TCP e aguarda pela resposta do servidor
// - Para comandos normais: lê até ao '\n' (resposta em linha)
// - Para o comando SED (show): lê um header com campos e depois lê os dados binários do ficheiro
//   guardando o ficheiro recebido em disco (received_<nome>)
// Retorna o tamanho da resposta colocada em 'reply' ou -1 em caso de erro
ssize_t send_tcp_and_wait_reply(int tcpfd,
                                const std::string &message,
                                std::string &reply);

// Lê exatamente 1 byte de um descritor de ficheiro (socket TCP)
// Retorna true se conseguiu ler 1 byte; false em caso de EOF/erro
static bool read_byte(int fd, char &c);

#endif