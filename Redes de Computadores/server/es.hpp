#ifndef ES_H
#define ES_H

#include <iostream>
#include <unordered_map>
#include <map>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>  
#include <vector>

#include "../common/constants.hpp"

using namespace std;

ssize_t read_n_bytes(int fd, char *buf, size_t n);

// Converte data (DD-MM-YYYY) e hora (HH:MM) para time_t
// Usado para comparar datas de eventos com o tempo atual
time_t to_time_t(const string& date, const string& hour);

// Garante que todos os bytes são escritos no socket
// write() pode escrever menos bytes do que os pedidos
bool write_all(int fd, const char *buf, size_t len);

// Estrutura que representa um evento no servidor
struct Event {
    string eid;                     
    string owner_uid;       
    string name;            
    string date_str;        
    string hour_str;       
    int attendance_size;        
    int seats_reserved;          

    bool closed_by_owner = false;
    bool sold_out = false;       

    string file_name;       
    vector<char> file_data;
};

// Estrutura que representa informação associada a um utilizador
struct UserInfo {
    string password;
    bool logged_in = false;
    struct sockaddr_in addr;
};

// Estrutura que representa uma reserva efetuada por um utilizador
struct ReservationInfo {
    string eid;
    string date_day;
    string date_hour;
    int number_of_tickets;
};

// Classe responsável por armazenar a configuração do servidor
class ServerConfig {
 public:
  char* programPath;
  string port = DEFAULT_PORT;
  bool help = false;
  bool verbose = false;
  bool random = false;

  ServerConfig(int argc, char* argv[]);
};

struct ServerState {
    int udp_fd = -1;
    int tcp_fd = -1;

    // Processa pedido de login
    string handle_login(const string& uid, const string& pass);

    // Processa pedido de logout
    string handle_logout(const string& uid, const string& pass);

    // Processa pedido de remoção de registo do utilizador
    string handle_unregister(const string& uid, const string& pass);

    // Processa pedido de criação de evento
    string handle_create(ServerState& state, const string& user_id, const string& password, const string& name, const string& event_date, const string& event_hours, 
        const string& num_attendees, const string& f_name, const string& fsize, const vector<char>& f_data);
    
    // Lista os eventos criados por um utilizador
    string handle_myEvents(ServerState& state, const string& uid, const string& password);
    
    // Processa pedido de reserva de bilhetes para um evento
    string handle_reserve(ServerState& state, const string& uid, const string& password, const string& eid, int value);
    
    // Lista as reservas de um utilizador
    string handle_myReservations(const string& uid, const string& pass);
    
    // Processa pedido de fecho de evento pelo organizador
    string handle_close(const string& uid, const string& password, const string& eid);
    
    // Processa pedido de alteração de password
    string handle_changePass(const string& uid, const string& old_password, const string& new_password);
    
    // Lista todos os eventos disponíveis no sistema
    string handle_list();
    
    // Mostra informação detalhada de um evento
    string handle_show(const string& eid);

    unordered_map<string, UserInfo> users;
    map<int, Event> events_by_eid;
    unordered_map<string, vector<ReservationInfo>> reservations_by_user;
};

// Cria e associa um socket UDP à porta indicada
int open_udp_socket();

// Cria e configura um socket TCP associado à porta indicada
int open_tcp_socket();

// Processa pedidos recebidos via UDP
void handle_udp(ServerState &state);

// Trata pedidos TCP (um cliente por vez)
void handle_tcp(ServerState &state);

// Verifica se a password do utilizador é válida:
// - exatamente 8 caracteres
// - apenas letras ou dígitos
bool parse_user_password(string user_password);

#endif