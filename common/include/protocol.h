#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SERVER_PORT 8000
#define MAX_NAME_LEN 50
#define RESPONSE_MESSAGE_SIZE 4096

typedef enum
{
    CMD_ADD_USER = 1,
    CMD_DEL_USER = 2,
    CMD_ADD_PET = 3,
    CMD_DEL_PET = 4,
    CMD_GET_ALL = 5,
    CMD_EXIT = -1
} cmd_type_t;

typedef enum
{
    ERR_OK = 0,
    ERR_NULL_PTR = 1,
    ERR_INVALID_ARG = 2,
    ERR_SOCKET_CREATE = 3,
    ERR_SOCKET_BIND = 4,
    ERR_SOCKET_LISTEN = 5,
    ERR_SOCKET_ACCEPT = 6,
    ERR_INVALID_IP = 7,
    ERR_SOCKET_CONNECT = 8,
    ERR_SEND = 9,
    ERR_RECV = 10,
    ERR_DISCONNECTED = -1
} return_code_t;

typedef enum
{
    RESP_OK = 0,
    RESP_INVALID_REQUEST = 1,
    RESP_DUPLICATE = 2,
    RESP_NOT_FOUND = 3,
    RESP_STORAGE_ERROR = 4,
    RESP_INTERNAL_ERROR = 5,
    RESP_CORRUPT = 6,
} response_status_t;

typedef struct
{
    cmd_type_t cmd;
    char username[MAX_NAME_LEN];
    char petname[MAX_NAME_LEN];
} request_t;

typedef struct
{
    response_status_t status;
    char message[RESPONSE_MESSAGE_SIZE];
} response_t;

#endif
