#include "socket_handler.h"
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include <stdbool.h>

static return_code_t recv_all(int fd, void *buffer, size_t length);
static return_code_t send_all(int fd, const void *buffer, size_t length);

static return_code_t recv_all(int fd, void *buffer, size_t length)
{
    return_code_t rc = ERR_OK;
    char *cursor = buffer;
    size_t total = 0;
    ssize_t received = 0;
    while (rc == ERR_OK && total < length)
    {
        received = recv(fd, cursor + total, length - total, 0);
        if (received < 0 && errno != EINTR)
            rc = ERR_RECV;
        else if (received == 0)
            rc = ERR_DISCONNECTED;
        else
            total += (size_t)received;
    }
    return rc;
}

static return_code_t send_all(int fd, const void *buffer, size_t length)
{
    return_code_t rc = ERR_OK;
    const char *cursor = buffer;
    size_t total = 0;
    ssize_t sent = 0;
    while (rc == ERR_OK && total < length)
    {
        sent = send(fd, cursor + total, length - total, 0);
        if (sent < 0 && errno != EINTR)
            rc = ERR_SEND;
        else if (sent >= 0)
            total += (size_t)sent;
    }
    return rc;
}

int init_server_socket(int port)
{
    struct sockaddr_in address;
    int server_fd = -1;
    int result = -1;
    int opt = 1;
    bool initialization = true;
    if (!(port > 0 && port <= 65535))
        initialization = false;
    if (initialization)
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        initialization = false;
    if (!initialization || setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
        initialization = false;
    if (initialization)
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons((unsigned short)port);
    }
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == 0 && listen(server_fd, SERVER_BACKLOG) == 0)
        result = server_fd;
    if (result < 0 && server_fd >= 0)
        close_socket(server_fd);
    return result;
}

int accept_client(int server_fd)
{
    int client_fd = -1;
    if (server_fd >= 0)
        client_fd = accept(server_fd, NULL, NULL);
    return client_fd;
}

return_code_t recv_request(int client_fd, request_t *request)
{
    return_code_t rc = ERR_OK;
    if (request == NULL)
        rc = ERR_NULL_PTR;
    if (rc == ERR_OK && client_fd < 0)
        rc = ERR_INVALID_ARG;
    if (rc == ERR_OK)
        rc = recv_all(client_fd, request, sizeof(*request));
    return rc;
}

return_code_t send_response(int client_fd, const response_t *response)
{
    return_code_t rc = ERR_OK;
    if (response == NULL)
        rc = ERR_NULL_PTR;
    if (rc == ERR_OK && client_fd < 0)
        rc = ERR_INVALID_ARG;
    if (rc == ERR_OK)
        rc = send_all(client_fd, response, sizeof(*response));
    return rc;
}

void close_socket(int fd)
{
    if (fd >= 0)
        close(fd);
}
