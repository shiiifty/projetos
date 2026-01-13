#ifndef COMMON_H
#define COMMON_H

#include <cstring>
#include <stdexcept>


// Exceção usada para erros fatais e irrecuperáveis.
// Deve ser lançada quando o programa não consegue continuar a executar.
class UnrecoverableError : public std::runtime_error {
 public:
  UnrecoverableError(const std::string& __what) : std::runtime_error(__what) {}
  UnrecoverableError(const std::string& __what, const int __errno)
      : std::runtime_error(__what + ": " + strerror(__errno)) {}
};

// Valida se a string que representa a porta contém apenas dígitos.
// Lança uma exceção UnrecoverableError caso a porta não seja válida.
void validate_port_number(std::string& port);

// Configura os handlers de sinais do sistema operativo,
// permitindo um encerramento controlado do servidor.
void setup_signal_handlers();

// Handler de sinais responsável por iniciar o processo de encerramento
// quando o servidor recebe um sinal de terminação (ex.: SIGINT ou SIGTERM).
void terminate_signal_handler(int sig);

#endif