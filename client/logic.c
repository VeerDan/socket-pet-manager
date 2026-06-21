#include "logic.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

return_code_t print_error_message(return_code_t rc)
{
    return_code_t ret_code = ERR_OK;

    switch (rc)
    {
      case ERR_OK:
          break;
      case ERR_NULL_PTR:
          fprintf(stderr, "Error: Null pointer passed.\n");
          break;
      case ERR_INVALID_ARG:
          fprintf(stderr, "Error: Invalid argument passed.\n");
          break;
      case ERR_SOCKET_CREATE:
          fprintf(stderr, "Error: Failed to create socket.\n");
          break;
      case ERR_INVALID_IP:
          fprintf(stderr, "Error: Invalid IP address format.\n");
          break;
      case ERR_SOCKET_CONNECT:
          fprintf(stderr, "Error: Failed to connect to server.\n");
          break;
      case ERR_SEND:
          fprintf(stderr, "Error: Network send failed.\n");
          break;
      case ERR_RECV:
          fprintf(stderr, "Error: Network receive failed.\n");
          break;
      case ERR_DISCONNECTED:
          fprintf(stderr, "Error: Connection dropped by remote server.\n");
          break;
      default:
          fprintf(stderr, "Error: Unknown return code (%d).\n", rc);
          break;
    }

    return ret_code;
}

return_code_t connect_to_server(const char *ip, int port, int *out_sock)
{
    return_code_t rc = ERR_OK;
    int sock = -1;
    struct sockaddr_in serv_addr;

    if (ip == NULL || out_sock == NULL)
        rc = ERR_NULL_PTR;

    if (rc == ERR_OK && (port <= 0 || port > 65535))
        rc = ERR_INVALID_ARG;

    if (rc == ERR_OK)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            rc = ERR_SOCKET_CREATE;
    }

    if (rc == ERR_OK)
    {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
            rc = ERR_INVALID_IP;
    }

    if (rc == ERR_OK)
    {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            rc = ERR_SOCKET_CONNECT;
    }

    if (rc == ERR_OK)
        *out_sock = sock;
    else if (sock >= 0)
        close(sock);

    return rc;
}

return_code_t send_request_and_receive(int sock, const request_t *req, response_t *res)
{
    return_code_t rc = ERR_OK;
    ssize_t bytes_received = 0;

    if (req == NULL || res == NULL)
        rc = ERR_NULL_PTR;

    if (rc == ERR_OK && sock < 0)
        rc = ERR_INVALID_ARG;

    if (rc == ERR_OK)
    {
        if (send(sock, req, sizeof(request_t), 0) < 0)
            rc = ERR_SEND;
    }

    if (rc == ERR_OK && req->cmd != CMD_EXIT)
    {
        memset(res, 0, sizeof(response_t));
        bytes_received = recv(sock, res, sizeof(response_t), 0);

        if (bytes_received < 0)
            rc = ERR_RECV;
        else if (bytes_received == 0)
            rc = ERR_DISCONNECTED;
    }

    return rc;
}

return_code_t close_connection(int sock)
{
    return_code_t rc = ERR_OK;

    if (sock < 0)
        rc = ERR_INVALID_ARG;
    else
        close(sock);

    return rc;
}
