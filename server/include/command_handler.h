#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "protocol.h"
#include <stdio.h>

int should_close_session(const request_t *request);
void process_request(FILE *users, FILE *pets, const request_t *request, response_t *response);

#endif
