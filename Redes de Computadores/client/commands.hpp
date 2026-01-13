#ifndef COMMANDS_H
#define COMMANDS_H

#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Valida o identificador do utilizador (UID)
// Um UID válido tem exatamente 6 dígitos numéricos
bool parse_user_id(string user_id);

// Valida a palavra-passe do utilizador
// Deve ter exatamente 8 caracteres alfanuméricos
bool parse_user_password(string user_password);

// Lê todo o conteúdo de um ficheiro (modo binário) e devolve-o numa string
// Retorna uma string vazia em caso de erro
string read_file(const string& filename);

// Constrói a mensagem de login (LIN) a enviar ao servidor
string loginHandler(string user_id, string password); 

// Constrói a mensagem de logout (LOU) a enviar ao servidor
string logoutHandler(string user_id, string password); 

// Constrói a mensagem de remoção de registo do utilizador (UNR)
string unregisterHandler(string user_id, string password); 

// Constrói a mensagem de criação de evento (CRE)
// Inclui informação do evento e o ficheiro associado
string createHandler(string uid, string password, string name, string event_fname,
                     string event_date, string event_hours, string num_attendees);

// Constrói a mensagem para listar os eventos criados pelo utilizador (LME)
string myEventsHandler(string user_id, string password); 

// Constrói a mensagem de reserva de lugares num evento (RID)
string reserveHandler(string uid, string password, string eid, string value);

// Constrói a mensagem para listar as reservas do utilizador (LMR)
string myReservationsHandler(string user_id, string password); 

// Constrói a mensagem para fechar um evento (CLS)
string closeHandler(string user_id, string password, string event_id);

// Constrói a mensagem para alterar a palavra-passe do utilizador (CPS)
string changePassHandler(string user_id, string old_pass, string new_pass);

// Constrói a mensagem para listar todos os eventos existentes (LST)
string listHandler();

// Constrói a mensagem para obter informação detalhada de um evento (SED)
string showHandler(string eid);


#endif