#include "commands.hpp"

#include <cstring>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <filesystem>

using namespace std;

// Variável global que indica se o servidor está em processo de encerramento
extern bool is_shutting_down;

// Valida o identificador do utilizador (UID)
// Um UID é válido se tiver exatamente 6 caracteres
// e se todos os caracteres forem dígitos
bool parse_user_id(string user_id) {
  if (user_id.length() != 6) return false;
  
  for (char c : user_id) if (!isdigit(c)) return false;

  return true;
}

// Valida a palavra-passe do utilizador
// Uma palavra-passe válida tem exatamente 8 caracteres
// e pode conter apenas dígitos e letras (maiúsculas ou minúsculas)
bool parse_user_password(string user_password) {
  if (user_password.length() != 8) return false;
  
  for (char c : user_password) 
    if (!isdigit(c) && !isalpha(c)) return false;
  
  return true;
}

// Lê todo o conteúdo de um ficheiro (modo binário) para uma string
// Retorna uma string vazia caso o ficheiro não possa ser aberto
string read_file(const string& filename) {
  ifstream file(filename, ios::binary);
  if (!file) {
      cerr << "Error opening file: " << filename << "\n";
      return "";
  }

  // Move o cursor para o fim do ficheiro para obter o seu tamanho
  file.seekg(0, ios::end);
  size_t size = file.tellg();

  // Reposiciona o cursor no início do ficheiro
  file.seekg(0);

  string content(size, '\0');
  // Lê todo o conteúdo do ficheiro para a string
  file.read(&content[0], size);
  return content;
}

// Valida uma data e uma hora nos formatos DD-MM-AAAA e HH:MM
// Retorna true se ambos os formatos e valores forem válidos
bool validate_date_time(const std::string& date_str, const std::string& time_str) {
    if (date_str.size() != 10) return false;

    if (date_str[2] != '-' || date_str[5] != '-')
        return false;

    int date_digits[] = {0,1, 3,4, 6,7,8,9};
    for (int idx : date_digits) {
        if (!std::isdigit((unsigned char)date_str[idx]))
            return false;
    }

    int day   = std::stoi(date_str.substr(0, 2));
    int month = std::stoi(date_str.substr(3, 2));
    int year  = std::stoi(date_str.substr(6, 4));

    if (day   < 1 || day   > 31) return false;
    if (month < 1 || month > 12) return false;
    if (year  < 0) return false;

    if (time_str.size() != 5)
        return false;

    if (time_str[2] != ':')
        return false;

    int time_digits[] = {0,1, 3,4};
    for (int idx : time_digits) {
        if (!std::isdigit((unsigned char)time_str[idx]))
            return false;
    }

    int hour   = std::stoi(time_str.substr(0, 2));
    int minute = std::stoi(time_str.substr(3, 2));

    if (hour   < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;

    return true;
}

// Constrói a mensagem de login a enviar ao servidor
// Valida previamente o UID e a palavra-passe do utilizador
// Retorna a mensagem LIN formatada ou uma string vazia em caso de erro
string loginHandler(string user_id, string password){

  bool ok = true;

  if (!parse_user_id(user_id)) {
    cout << "Invalid UID. It must be a 6-digit IST student number\n";
    ok = false;
  }

  if (!parse_user_password(password)) {
    cout << "Invalid password. It must composed of 8 alphanumeric characters\n";
    ok = false;
  }

  if (!ok) return "";

  return "LIN " + user_id + " " + password + "\n";
}

// Constrói a mensagem de logout a enviar ao servidor
// Retorna a mensagem LOU formatada
string logoutHandler(string user_id, string password) {
  return "LOU " + user_id + " " + password + "\n";
}

// Constrói a mensagem de unregister a enviar ao servidor
// Retorna a mensagem UNR formatada
string unregisterHandler(string user_id, string password) {
  return "UNR " + user_id + " " + password + "\n";
}

// Constrói a mensagem de myEvents enviar ao servidor
// Retorna a mensagem LME formatada
string myEventsHandler(string user_id, string password) {
  return "LME " + user_id + " " + password + "\n";
}

// Constrói a mensagem de criação de evento CRE a enviar ao servidor
// Valida a data/hora do evento e o tamanho do ficheiro associado
// Retorna a mensagem formatada ou uma string vazia em caso de erro
string createHandler(string user_id, string password, string name, string event_fname, 
                     string event_date, string event_hours, string num_attendees) {
  bool ok = true;
  
  if (!validate_date_time(event_date, event_hours)) {
    cout << "Date format is incorrect, it should be dd-mm-yyyy hh:mm\n";
    ok = false;
  }

  uintmax_t f_size_um;
  try {
      f_size_um = filesystem::file_size(event_fname);
  } catch (const filesystem::filesystem_error& e) {
      cout << "Error getting file size: " << e.what() << "\n";
      return "";
  }


  int f_size_int = filesystem::file_size(event_fname);
  
  if (f_size_int > 10*1024*1024) {
    cout << "File size must be at max 10MB\n";
    ok = false;
  } 

  if (!ok) return "";

  string f_size = to_string(f_size_int);
  string f_data;

  f_data = read_file(event_fname);        
    
  string msg = "CRE " + user_id + " " + password + " " + name  + " " + event_date 
              + " " + event_hours + " " + num_attendees + " " + event_fname
              + " " + f_size + " ";

  msg += f_data;                               

  msg += "\n";  

  return msg;
}


// Constrói a mensagem de reserva de lugares num evento (RID)
// Recebe o UID, a palavra-passe, o identificador do evento (EID)
// e o número de lugares a reservar
string reserveHandler(string user_id, string password, string eid, string value) {
  return "RID " + user_id + " " + password + " " + eid + " " + value + "\n";
}

// Constrói a mensagem para listar as reservas do utilizador (LMR)
// Inclui o UID e a palavra-passe para autenticação
string myReservationsHandler(string user_id, string password) {
  return "LMR " + user_id + " " + password + "\n";
}

// Constrói a mensagem para fechar um evento (CLS)
// Apenas o criador do evento pode executar esta operação
string closeHandler(string user_id, string password, string event_id) {
  return "CLS " + user_id + " " + password + " " + event_id + "\n";
}

// Constrói a mensagem para alterar a palavra-passe do utilizador (CPS)
// Inclui o UID, a palavra-passe antiga e a nova palavra-passe
string changePassHandler(string user_id, string old_password, string new_password) {
  return "CPS " + user_id + " " + old_password + " " + new_password + "\n";
}

// Constrói a mensagem para listar todos os eventos existentes (LST)
// Não requer autenticação
string listHandler() {
  return "LST\n";
}

// Constrói a mensagem para obter a informação detalhada de um evento (SED)
// Recebe o identificador do evento (EID)
string showHandler(string eid) {
  return "SED " + eid + "\n";
}
