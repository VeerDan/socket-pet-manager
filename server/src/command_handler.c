#include "command_handler.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "db.h"
#include "db_utils.h"

static response_status_t db2rsp_status(db_status_t status);
static void set_response_message(response_t *response, response_status_t status, const char *format, ...);
static void handle_add_user(const request_t *request, response_t *response, FILE *users);
static void handle_remove_user(const request_t *request, response_t *response, FILE *users, FILE *pets);
static void handle_add_pet(const request_t *request, response_t *response, FILE *pets, FILE *users);
static void handle_remove_pet(const request_t *request, response_t *response, FILE *users, FILE *pets);
static void handle_list_all(response_t *response, FILE *users, FILE *pets);

static response_status_t db2rsp_status(db_status_t status)
{
    response_status_t response_status = RESP_OK;
    switch (status)
    {
        case DB_OK:
            response_status = RESP_OK;
            break;
        case DB_INVALID_NAME:
            response_status = RESP_INVALID_REQUEST;
            break;
        case DB_DUPLICATE_USER:
            response_status = RESP_DUPLICATE;
            break;
        case DB_DUPLICATE_PET:
            response_status = RESP_DUPLICATE;
            break;
        case DB_USER_NOT_FOUND:
            response_status = RESP_NOT_FOUND;
            break;
        case DB_PET_NOT_FOUND:
            response_status = RESP_NOT_FOUND;
            break;
        case DB_DATA_CORRUPT:
            response_status = RESP_CORRUPT;
            break;
        case DB_IO_ERROR:
            response_status = RESP_STORAGE_ERROR;
            break;
        default:
            response_status = RESP_INTERNAL_ERROR;
            break;
    }
    return response_status;
}

static void set_response_message(response_t *response, response_status_t status, const char *format, ...)
{
    va_list args;
    if (response != NULL)
    {
        response->status = status;
        if (format != NULL)
        {
            va_start(args, format);
            vsnprintf(response->message, sizeof(response->message), format, args);
            va_end(args);
        }
        else
            response->message[0] = '\0';
    }
}

static void handle_add_user(const request_t *request, response_t *response, FILE *users)
{
    db_status_t status = DB_OK;
    if (request == NULL)
        status = DB_NULL_PTR;
    if (status != DB_OK)
        set_response_message(response, RESP_INVALID_REQUEST, "User name must not be empty.");
    if (status == DB_OK)
        status = db_add_user(users, request->username);
    if (status == DB_OK)
        set_response_message(response, RESP_OK, "User \"%s\" added.", request->username);
    else if (status == DB_DUPLICATE_USER)
        set_response_message(response, RESP_DUPLICATE, "User \"%s\" already exists.", request->username);
    else if (status != DB_INVALID_NAME)
        set_response_message(response, db2rsp_status(status), "Failed to add user.");
}

static void handle_remove_user(const request_t *request, response_t *response, FILE *users, FILE *pets)
{
    db_status_t status = DB_OK;
    if (request == NULL)
        status = DB_NULL_PTR;
    size_t removed_pets = 0;
    if (status != DB_OK)
        set_response_message(response, RESP_INVALID_REQUEST, "User name must not be empty.");
    if (status == DB_OK)
        status = db_remove_user(users, pets, request->username, &removed_pets);
    if (status == DB_OK)
        set_response_message(response, RESP_OK, "User \"%s\" removed with %zu pet(s).", request->username, removed_pets);
    else if (status == DB_USER_NOT_FOUND)
        set_response_message(response, RESP_NOT_FOUND, "User \"%s\" not found.", request->username);
    else if (status != DB_INVALID_NAME)
        set_response_message(response, db2rsp_status(status), "Failed to remove user.");
}

static void handle_add_pet(const request_t *request, response_t *response, FILE *pets, FILE *users)
{
    db_status_t status = DB_OK;
    if (request == NULL)
        status = DB_NULL_PTR;
    if (status != DB_OK)
        set_response_message(response, RESP_INVALID_REQUEST, "Owner name or pet name must not be empty.");
    if (status == DB_OK)
        status = db_add_pet(users, pets, request->username, request->petname);
    if (status == DB_OK)
        set_response_message(response, RESP_OK, "Pet \"%s\" added to user \"%s\".", request->petname, request->username);
    else if (status == DB_USER_NOT_FOUND)
        set_response_message(response, RESP_NOT_FOUND, "User \"%s\" not found.", request->username);
    else if (status == DB_DUPLICATE_PET)
        set_response_message(response, RESP_DUPLICATE, "User \"%s\" already has pet \"%s\".", request->username, request->petname);
    else if (status != DB_INVALID_NAME)
        set_response_message(response, db2rsp_status(status), "Failed to add pet.");
}

static void handle_remove_pet(const request_t *request, response_t *response, FILE *users, FILE *pets)
{
    db_status_t status = DB_OK;
    if (request == NULL)
        status = DB_NULL_PTR;
    if (status != DB_OK)
        set_response_message(response, RESP_INVALID_REQUEST, "Owner name or pet name must not be empty.");
    if (status == DB_OK)
        status = db_remove_pet(users, pets, request->petname, request->username);
    if (status == DB_OK)
        set_response_message(response, RESP_OK, "Pet \"%s\" removed from user \"%s\".", request->petname, request->username);
    else if (status == DB_USER_NOT_FOUND)
        set_response_message(response, RESP_NOT_FOUND, "User \"%s\" not found.", request->username);
    else if (status == DB_PET_NOT_FOUND)
        set_response_message(response, RESP_NOT_FOUND, "Pet \"%s\" not found for user \"%s\".", request->petname, request->username);
    else if (status != DB_INVALID_NAME)
        set_response_message(response, db2rsp_status(status), "Failed to remove pet.");
}

static void handle_list_all(response_t *response, FILE *users, FILE *pets)
{
    db_status_t status = DB_OK;
    status = db_list_all(users, pets, response->message, sizeof(response->message));
    if (status == DB_OK)
        response->status = RESP_OK;
    else
        set_response_message(response, db2rsp_status(status), "Failed to list users and pets.");
}

int should_close_session(const request_t *request)
{
    int should_close = 0;
    if (request != NULL && request->cmd == CMD_EXIT)
        should_close = 1;
    return should_close;
}

void process_request(FILE *users, FILE *pets, const request_t *request, response_t *response)
{
    if (request != NULL && response != NULL && users != NULL && pets != NULL)
    {
        switch (request->cmd)
        {
            case CMD_ADD_USER:
                handle_add_user(request, response, users);
                break;
            case CMD_DEL_USER:
                handle_remove_user(request, response, users, pets);
                break;
            case CMD_ADD_PET:
                handle_add_pet(request, response, pets, users);
                break;
            case CMD_DEL_PET:
                handle_remove_pet(request, response, users, pets);
                break;
            case CMD_GET_ALL:
                handle_list_all(response, users, pets);
                break;
            default:
                set_response_message(response, RESP_INVALID_REQUEST, "Unknown command.");
                break;
        }
    }
}
