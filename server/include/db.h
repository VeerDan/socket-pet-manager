#ifndef DB_H
#define DB_H

#include <stddef.h>

#include "errors.h"
#include "protocol.h"

typedef struct
{
    size_t user_id;
    char name[MAX_NAME_LEN];
} user_t;

typedef struct
{
    size_t pet_id;
    size_t owner_id;
    char pet_name[MAX_NAME_LEN];
} pet_t;

#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_utils.h"
#include "server.h"
#include <unistd.h>

db_status_t db_remove_user_by_idx(FILE *users, size_t idx);
db_status_t db_remove_pet_by_id(FILE *pets, size_t idx);
db_status_t db_add_user(FILE *users, const char *username);
db_status_t db_remove_user(FILE *users, FILE *pets, const char *username, size_t *removed);
db_status_t db_add_pet(FILE *users, FILE *pets, const char *username, const char *petname);
db_status_t db_remove_pet(FILE *users, FILE *pets, const char *petname, const char *username);
db_status_t db_list_all(FILE *users, FILE *pets, char *buffer, size_t buffer_size);


#endif
