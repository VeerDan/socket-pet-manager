#ifndef CLIENT_LOGIC_H
#define CLIENT_LOGIC_H

#include "client.h"

return_code_t print_error_message(return_code_t rc);

return_code_t connect_to_server(const char *ip, int port, int *out_sock);

return_code_t send_request_and_receive(int sock, const request_t *req, response_t *res);

return_code_t close_connection(int sock);

#endif
