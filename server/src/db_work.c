#define _POSIX_C_SOURCE 200809L

#include "db.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "db_utils.h"

static void append_with_limit(char *buffer, size_t buffer_size, size_t *length, const char *text);
static db_status_t read_user_at(FILE *users, size_t idx, user_t *user);
static db_status_t write_user_at(FILE *users, size_t idx, const user_t *user);
static db_status_t read_pet_at(FILE *pets, size_t idx, pet_t *pet);
static db_status_t write_pet_at(FILE *pets, size_t idx, const pet_t *pet);
static db_status_t truncate_record(FILE *file, size_t record_size);

static void append_with_limit(char *buffer, size_t buffer_size, size_t *length, const char *text)
{
    int written = 0;
    size_t available = 0;

    if (buffer != NULL && length != NULL && text != NULL && buffer_size > 0 && *length < buffer_size)
    {
        available = buffer_size - *length;
        written = snprintf(buffer + *length, available, "%s", text);
        if (written >= 0 && (size_t)written >= available)
        {
            *length = buffer_size - 1;
            buffer[*length] = '\0';
        }
        else if (written >= 0)
            *length += (size_t)written;
    }
}

static db_status_t read_user_at(FILE *users, size_t idx, user_t *user)
{
    db_status_t status = DB_OK;

    if (users == NULL || user == NULL)
        status = DB_NULL_PTR;
    else if (fseek(users, (long int)(idx * sizeof(user_t)), SEEK_SET) != 0)
        status = DB_IO_ERROR;
    else if (fread(user, sizeof(user_t), 1, users) != 1)
        status = DB_IO_ERROR;
    return status;
}

static db_status_t write_user_at(FILE *users, size_t idx, const user_t *user)
{
    db_status_t status = DB_OK;

    if (users == NULL || user == NULL)
        status = DB_NULL_PTR;
    else if (fseek(users, (long int)(idx * sizeof(user_t)), SEEK_SET) != 0)
        status = DB_IO_ERROR;
    else if (fwrite(user, sizeof(user_t), 1, users) != 1)
        status = DB_IO_ERROR;
    return status;
}

static db_status_t read_pet_at(FILE *pets, size_t idx, pet_t *pet)
{
    db_status_t status = DB_OK;

    if (pets == NULL || pet == NULL)
        status = DB_NULL_PTR;
    else if (fseek(pets, (long int)(idx * sizeof(pet_t)), SEEK_SET) != 0)
        status = DB_IO_ERROR;
    else if (fread(pet, sizeof(pet_t), 1, pets) != 1)
        status = DB_IO_ERROR;
    return status;
}

static db_status_t write_pet_at(FILE *pets, size_t idx, const pet_t *pet)
{
    db_status_t status = DB_OK;

    if (pets == NULL || pet == NULL)
        status = DB_NULL_PTR;
    else if (fseek(pets, (long int)(idx * sizeof(pet_t)), SEEK_SET) != 0)
        status = DB_IO_ERROR;
    else if (fwrite(pet, sizeof(pet_t), 1, pets) != 1)
        status = DB_IO_ERROR;
    return status;
}

static db_status_t truncate_record(FILE *file, size_t record_size)
{
    db_status_t status = DB_OK;
    long int end = 0;
    int fd = -1;

    if (file == NULL || record_size == 0)
        status = DB_NULL_PTR;
    else if (fflush(file) != 0)
        status = DB_IO_ERROR;
    else if (fseek(file, 0, SEEK_END) != 0)
        status = DB_IO_ERROR;
    else if ((end = ftell(file)) < 0 || (size_t)end < record_size)
        status = DB_IO_ERROR;
    else if ((fd = fileno(file)) < 0)
        status = DB_IO_ERROR;
    else if (ftruncate(fd, end - (long int)record_size) != 0)
        status = DB_IO_ERROR;
    else if (fseek(file, 0, SEEK_SET) != 0)
        status = DB_IO_ERROR;
    return status;
}

db_status_t db_remove_user_by_idx(FILE *users, size_t idx)
{
    db_status_t status = DB_OK;
    size_t cnt = 0;

    if (users == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(users, sizeof(user_t), &cnt);
    if (status == DB_OK && !(idx < cnt))
        status = DB_IO_ERROR;
    for (size_t i = idx; i + 1 < cnt && status == DB_OK; i++)
    {
        user_t temp;

        status = read_user_at(users, i + 1, &temp);
        if (status == DB_OK)
            status = write_user_at(users, i, &temp);
    }
    if (status == DB_OK)
        status = truncate_record(users, sizeof(user_t));
    return status;
}

db_status_t db_remove_pet_by_id(FILE *pets, size_t idx)
{
    db_status_t status = DB_OK;
    size_t cnt = 0;

    if (pets == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(pets, sizeof(pet_t), &cnt);
    if (status == DB_OK && !(idx < cnt))
        status = DB_IO_ERROR;
    for (size_t i = idx; i + 1 < cnt && status == DB_OK; i++)
    {
        pet_t temp;

        status = read_pet_at(pets, i + 1, &temp);
        if (status == DB_OK)
            status = write_pet_at(pets, i, &temp);
    }
    if (status == DB_OK)
        status = truncate_record(pets, sizeof(pet_t));
    return status;
}

db_status_t db_add_user(FILE *users, const char *username)
{
    db_status_t status = DB_OK;
    size_t index = 0;
    char strip_name[MAX_NAME_LEN];

    if (users == NULL || username == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = strip(username, strip_name, MAX_NAME_LEN);
    if (status == DB_OK)
        status = find_user_index_by_name(users, strip_name, &index);
    if (status == DB_OK)
        status = DB_DUPLICATE_USER;
    else if (status == DB_USER_NOT_FOUND)
        status = DB_OK;
    if (status == DB_OK)
    {
        long int cur = ftell(users);
        user_t user;

        if (cur < 0 || fseek(users, 0, SEEK_END) != 0)
            status = DB_IO_ERROR;
        strncpy(user.name, strip_name, MAX_NAME_LEN);
        user.name[MAX_NAME_LEN - 1] = '\0';
        if (status == DB_OK)
            status = next_user_id(users, &user.user_id);
        if (status == DB_OK && fwrite(&user, sizeof(user_t), 1, users) != 1)
            status = DB_IO_ERROR;
        if (fseek(users, cur, SEEK_SET) != 0 && status == DB_OK)
            status = DB_IO_ERROR;
    }
    return status;
}

db_status_t db_remove_user(FILE *users, FILE *pets, const char *username, size_t *removed_pets)
{
    db_status_t status = DB_OK;
    size_t index = 0;
    size_t pets_cnt = 0;
    char strip_name[MAX_NAME_LEN];
    user_t user;

    if (users == NULL || pets == NULL || username == NULL || removed_pets == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        *removed_pets = 0;
    if (status == DB_OK)
        status = strip(username, strip_name, MAX_NAME_LEN);
    if (status == DB_OK)
        status = find_user_index_by_name(users, strip_name, &index);
    if (status == DB_OK)
        status = read_user_at(users, index, &user);
    if (status == DB_OK)
        status = db_remove_user_by_idx(users, index);
    if (status == DB_OK)
        status = count_records(pets, sizeof(pet_t), &pets_cnt);

    for (size_t i = 0; i < pets_cnt && status == DB_OK; )
    {
        pet_t pet;

        status = read_pet_at(pets, i, &pet);
        if (status != DB_OK)
            break;
        if (pet.owner_id == user.user_id)
        {
            size_t pet_idx = 0;

            status = find_pet_index_by_owner_and_name(pets, pet.owner_id, pet.pet_name, &pet_idx);
            if (status == DB_OK)
                status = db_remove_pet_by_id(pets, pet_idx);
            if (status == DB_OK)
            {
                pets_cnt--;
                *removed_pets += 1;
                continue;
            }
        }
        i++;
    }
    return status;
}

db_status_t db_add_pet(FILE *users, FILE *pets, const char *username, const char *petname)
{
    db_status_t status = DB_OK;
    size_t user_idx = 0;
    size_t pet_idx = 0;
    user_t user;
    char strip_username[MAX_NAME_LEN];
    char strip_petname[MAX_NAME_LEN];

    if (users == NULL || pets == NULL || username == NULL || petname == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = strip(username, strip_username, MAX_NAME_LEN);
    if (status == DB_OK)
        status = strip(petname, strip_petname, MAX_NAME_LEN);
    if (status == DB_OK)
        status = find_user_index_by_name(users, strip_username, &user_idx);
    if (status == DB_OK)
        status = read_user_at(users, user_idx, &user);
    if (status == DB_OK)
        status = find_pet_index_by_owner_and_name(pets, user.user_id, strip_petname, &pet_idx);
    if (status == DB_OK)
        status = DB_DUPLICATE_PET;
    else if (status == DB_PET_NOT_FOUND)
        status = DB_OK;
    if (status == DB_OK)
    {
        long int cur = ftell(pets);
        pet_t pet = { .owner_id = user.user_id };

        if (cur < 0 || fseek(pets, 0, SEEK_END) != 0)
            status = DB_IO_ERROR;
        strncpy(pet.pet_name, strip_petname, MAX_NAME_LEN);
        pet.pet_name[MAX_NAME_LEN - 1] = '\0';
        if (status == DB_OK)
            status = next_pet_id(pets, &pet.pet_id);
        if (status == DB_OK && fwrite(&pet, sizeof(pet_t), 1, pets) != 1)
            status = DB_IO_ERROR;
        if (fseek(pets, cur, SEEK_SET) != 0 && status == DB_OK)
            status = DB_IO_ERROR;
    }
    return status;
}

db_status_t db_remove_pet(FILE *users, FILE *pets, const char *petname, const char *username)
{
    db_status_t status = DB_OK;
    size_t user_idx = 0;
    size_t pet_idx = 0;
    user_t user;
    char strip_petname[MAX_NAME_LEN];
    char strip_username[MAX_NAME_LEN];

    if (users == NULL || pets == NULL || petname == NULL || username == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = strip(petname, strip_petname, MAX_NAME_LEN);
    if (status == DB_OK)
        status = strip(username, strip_username, MAX_NAME_LEN);
    if (status == DB_OK)
        status = find_user_index_by_name(users, strip_username, &user_idx);
    if (status == DB_OK)
        status = read_user_at(users, user_idx, &user);
    if (status == DB_OK)
        status = find_pet_index_by_owner_and_name(pets, user.user_id, strip_petname, &pet_idx);
    if (status == DB_OK)
        status = db_remove_pet_by_id(pets, pet_idx);
    return status;
}

db_status_t db_list_all(FILE *users, FILE *pets, char *buffer, size_t buffer_size)
{
    db_status_t status = DB_OK;
    size_t users_cnt = 0;
    size_t pets_cnt = 0;
    size_t length = 0;

    if (users == NULL || pets == NULL || buffer == NULL || buffer_size == 0)
        status = DB_NULL_PTR;

    if (status == DB_OK)
        buffer[0] = '\0';
    if (status == DB_OK)
        status = count_records(users, sizeof(user_t), &users_cnt);
    if (status == DB_OK)
        status = count_records(pets, sizeof(pet_t), &pets_cnt);
    if (status == DB_OK && users_cnt == 0)
        snprintf(buffer, buffer_size, "No users found.");

    for (size_t i = 0; i < users_cnt && status == DB_OK; i++)
    {
        bool has_pets = false;
        char line[256];
        user_t user;

        status = read_user_at(users, i, &user);

        snprintf(line, sizeof(line), "%s:", user.name);
        append_with_limit(buffer, buffer_size, &length, line);

        for (size_t j = 0; j < pets_cnt && status == DB_OK; j++)
        {
            pet_t pet;

            status = read_pet_at(pets, j, &pet);
            if (pet.owner_id == user.user_id)
            {
                if (has_pets)
                    append_with_limit(buffer, buffer_size, &length, ", ");
                else
                    append_with_limit(buffer, buffer_size, &length, " ");

                append_with_limit(buffer, buffer_size, &length, pet.pet_name);
                has_pets = true;
            }
        }

        if (status == DB_OK && !has_pets)
            append_with_limit(buffer, buffer_size, &length, " (no pets)");
        if (status == DB_OK && i + 1 < users_cnt)
            append_with_limit(buffer, buffer_size, &length, "\n");
    }
    return status;
}
