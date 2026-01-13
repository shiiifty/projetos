#include "common.hpp"

#include <csignal>


// Indica se o servidor já iniciou o processo de encerramento.
// Usado para garantir que o shutdown é feito apenas uma vez.
bool is_shutting_down = false;


// Configura os handlers de sinais do sistema operativo
// (ex.: CTRL+C ou encerramento do processo)
void setup_signal_handlers() {
  struct sigaction sa;
  sa.sa_handler = terminate_signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    throw UnrecoverableError("Setting SIGINT signal handler", errno);
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    throw UnrecoverableError("Setting SIGFAULT signal handler", errno);
  }
  signal(SIGPIPE, SIG_IGN);
}

// Handler chamado quando o processo recebe um sinal de terminação
void terminate_signal_handler(int sig) {
  (void)sig;
  if (is_shutting_down) {
    exit(EXIT_SUCCESS);
  }
  is_shutting_down = true;
}

// Verifica se a string da porta contém apenas dígitos
// Lança erro caso a porta não seja um número válido
void validate_port_number(std::string &port) {
  for (char c : port) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      throw UnrecoverableError("Invalid port: not a number");
    }
  }
}