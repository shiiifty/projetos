#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>
#include <cmath>

// Nome do host por omissão utilizado nas ligações ao servidor
#define DEFAULT_HOSTNAME "localhost"

// Porta por omissão utilizada pelo servidor
#define DEFAULT_PORT "58016"

// Número máximo de tentativas de reenvio de mensagens UDP
#define UDP_RESEND_TRIES (3)

// Tempo máximo (em segundos) de espera para leitura numa ligação TCP
#define TCP_READ_TIMEOUT_SECONDS (15)

// Tempo máximo (em segundos) de espera para escrita numa ligação TCP
#define TCP_WRITE_TIMEOUT_SECONDS (20*60)

// Tempo máximo (em segundos) de espera por resposta numa ligação UDP
#define UDP_TIMEOUT_SECONDS (4)

#endif