#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "protocol.h"

int init_server_socket(int port);
int accept_client(int server_fd);
return_code_t recv_request(int client_fd, request_t *request);
return_code_t send_response(int client_fd, const response_t *response);
void close_socket(int fd);

#endif
