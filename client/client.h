#ifndef CLIENT_COMMON_H
#define CLIENT_COMMON_H

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_NAME_LEN 50

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
    ERR_INVALID_IP = 4,
    ERR_SOCKET_CONNECT = 5,
    ERR_SEND = 6,
    ERR_RECV = 7,
    ERR_DISCONNECTED = -1
} return_code_t;

typedef struct
{
    int cmd;
    char username[MAX_NAME_LEN];
    char petname[MAX_NAME_LEN];
} request_t;

typedef struct
{
    int status;
    char message[1024]; 
} response_t;

#endif
