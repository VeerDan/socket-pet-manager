#include "server_runtime.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "command_handler.h"
#include "db.h"
#include "errors.h"
#include "protocol.h"
#include "socket_handler.h"
#include <stdbool.h>

static const char *db_status_message(db_status_t status);
static void print_error(return_code_t code, const char *context);
static int initialize_server(int *server_fd, FILE **, FILE **);
static void run_accept_loop(int server_fd, FILE *, FILE *);
static void serve_client_session(int client_fd, FILE *, FILE *);


static const char *db_status_message(db_status_t status)
{
    const char *message = "unknown db error";
    switch (status)
    {
        case DB_OK:
            message = "ok";
            break;
        case DB_NULL_PTR:
            message = "null pointer";
            break;
        case DB_INVALID_NAME:
            message = "invalid name";
            break;
        case DB_DUPLICATE_USER:
            message = "duplicate user";
            break;
        case DB_DUPLICATE_PET:
            message = "duplicate pet";
            break;
        case DB_USER_NOT_FOUND:
            message = "user not found";
            break;
        case DB_PET_NOT_FOUND:
            message = "pet not found";
            break;
        case DB_DATA_CORRUPT:
            message = "storage files are corrupted";
            break;
        case DB_IO_ERROR:
            message = "i/o error";
            break;
        case DB_NO_MEMORY:
            message = "out of memory";
            break;
        default:
            break;
    }
    return message;
}

static void print_error(return_code_t code, const char *context)
{
    const char *prefix = context;
    if (prefix == NULL)
        prefix = "Network";
    if (code == ERR_DISCONNECTED)
        fprintf(stderr, "%s: client disconnected.\n", prefix);
    else if (code == ERR_RECV)
        fprintf(stderr, "%s: recv failed: %s\n", prefix, strerror(errno));
    else if (code == ERR_SEND)
        fprintf(stderr, "%s: send failed: %s\n", prefix, strerror(errno));
    else
        fprintf(stderr, "%s: network error %d.\n", prefix, code);
}

static int initialize_server(int *server_fd, FILE **users, FILE **pets)
{
    int exit_code = EXIT_SUCCESS;
    int local_server_fd = -1;
    db_status_t db_status = DB_OK;

    if (server_fd == NULL)
        exit_code = EXIT_FAILURE;
    if (exit_code == EXIT_SUCCESS)
        db_status = db_init_storage(users, pets);
    if (exit_code == EXIT_SUCCESS && db_status != DB_OK)
    {
        fprintf(stderr, "Storage initialization failed: %s.\n", db_status_message(db_status));
        exit_code = EXIT_FAILURE;
    }
    if (exit_code == EXIT_SUCCESS)
        db_status = db_validate_storage(*users, *pets);
    if (exit_code == EXIT_SUCCESS && db_status != DB_OK)
    {
        fprintf(stderr, "Storage validation failed: %s.\n", db_status_message(db_status));
        exit_code = EXIT_FAILURE;
    }

    if (exit_code == EXIT_SUCCESS)
        local_server_fd = init_server_socket(SERVER_PORT);
    if (exit_code == EXIT_SUCCESS && local_server_fd < 0)
    {
        fprintf(stderr, "Failed to start server on port %d: %s\n", SERVER_PORT, strerror(errno));
        exit_code = EXIT_FAILURE;
    }

    if (exit_code == EXIT_SUCCESS)
        *server_fd = local_server_fd;
    else if (local_server_fd >= 0)
        close_socket(local_server_fd);

    return exit_code;
}

static void serve_client_session(int client_fd, FILE *users, FILE *pets)
{
    bool active = true;
    request_t request;
    response_t response;
    return_code_t rc = ERR_OK;

    while (active)
    {
        rc = recv_request(client_fd, &request);
        if (rc != ERR_OK)
            active = false;
        if (rc != ERR_OK && rc != ERR_DISCONNECTED)
            print_error(rc, "Receiving request");

        if (active && should_close_session(&request))
            active = false;

        if (active)
        {
            process_request(users, pets, &request, &response);
            rc = send_response(client_fd, &response);
        }
        if (rc != ERR_OK)
        {
            print_error(rc, "Sending response");
            active = false;
        }
    }
}

static void run_accept_loop(int server_fd, FILE *users, FILE *pets)
{
    bool waiting = true;
    int client_fd = -1;

    while (waiting)
    {
        client_fd = accept_client(server_fd);
        if (client_fd < 0)
            fprintf(stderr, "Accept failed: %s\n", strerror(errno));
        else
        {
            printf("Client connected.\n");
            serve_client_session(client_fd, users, pets);
            close_socket(client_fd);
            printf("Client session closed.\n");
        }
    }
}

int run_server(void)
{
    int exit_code = EXIT_SUCCESS;
    int server_fd = -1;
    FILE *users = NULL;
    FILE *pets = NULL;
    exit_code = initialize_server(&server_fd, &users, &pets);
    if (exit_code == EXIT_SUCCESS)
    {
        printf("Server listening on port %d\n", SERVER_PORT);
        run_accept_loop(server_fd, users, pets);
    }
    if (server_fd >= 0)
        close_socket(server_fd);
    if (users != NULL)
        fclose(users);
    if (pets != NULL)
        fclose(pets);
    return exit_code;
}
