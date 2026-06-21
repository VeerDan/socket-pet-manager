#include "db_utils.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "server.h"

static FILE *open_storage_file(const char *path);

static FILE *open_storage_file(const char *path)
{
    FILE *file = NULL;

    if (path != NULL)
    {
        file = fopen(path, "r+b");
        if (file == NULL && errno == ENOENT)
            file = fopen(path, "w+b");
    }
    return file;
}

db_status_t count_records(FILE *f, size_t record_size, size_t *count)
{
    db_status_t status = DB_OK;
    long int cur = 0;
    long int end = 0;

    if (count == NULL || f == NULL)
        status = DB_NULL_PTR;
    else if (record_size == 0)
        status = DB_IO_ERROR;
    if (status == DB_OK)
    {
        cur = ftell(f);
        if (cur < 0 || fseek(f, 0, SEEK_END) != 0)
            status = DB_IO_ERROR;
    }
    if (status == DB_OK)
    {
        end = ftell(f);
        if (end < 0)
            status = DB_IO_ERROR;
        else if ((size_t) end % record_size != 0)
            status = DB_DATA_CORRUPT;
        else
            *count = (size_t) (end / (long int) record_size);
    }
    if (cur >= 0 && fseek(f, cur, SEEK_SET) != 0)
        status = DB_IO_ERROR;
    return status;
}

db_status_t ensure_storage_dir(void)
{
    db_status_t status = DB_OK;
    struct stat info;

    errno = 0;
    if (stat(DATA_DIR, &info) == 0 && !S_ISDIR(info.st_mode))
        status = DB_IO_ERROR;
    else if (errno == ENOENT && mkdir(DATA_DIR, 0777) != 0)
        status = DB_IO_ERROR;
    else if (errno != ENOENT && stat(DATA_DIR, &info) != 0)
        status = DB_IO_ERROR;
    return status;
}

db_status_t strip(const char *input, char *output, size_t output_size)
{
    db_status_t status = DB_OK;
    size_t start = 0;
    size_t end = 0;
    if (input == NULL || output == NULL || output_size == 0)
        status = DB_NULL_PTR;
    if (status == DB_OK)
    {
        end = strlen(input);
        while (start < end && isspace((unsigned char) input[start]))
            ++start;

        while (end > start && isspace((unsigned char) input[end - 1]))
            --end;

        if (end == start)
            status = DB_INVALID_NAME;
    }
    if (status == DB_OK && end - start >= output_size)
        status = DB_INVALID_NAME;
    if (status == DB_OK)
    {
        strcpy(output, input + start);
        output[end - start] = '\0';
    }
    return status;
}

db_status_t find_user_index_by_name(FILE *f, const char *name, size_t *index)
{
    user_t user;
    size_t count = 0;
    db_status_t status = DB_OK;
    size_t i = 0;
    long int cur = -1;

    if (f == NULL || name == NULL || index == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(f, sizeof(user_t), &count);
    if (status == DB_OK)
        status = DB_USER_NOT_FOUND;
    if (status == DB_USER_NOT_FOUND)
    {
        cur = ftell(f);
        if (cur < 0 || fseek(f, 0, SEEK_SET) != 0)
            status = DB_IO_ERROR;
    }
    for (i = 0; i < count && status == DB_USER_NOT_FOUND; i++)
    {
        if (fread(&user, sizeof(user_t), 1, f) != 1)
        {
            status = DB_IO_ERROR;
            break;
        }
        if (strcmp(user.name, name) == 0)
        {
            *index = i;
            status = DB_OK;
        }
    }
    if (cur >= 0 && fseek(f, cur, SEEK_SET) != 0)
        status = DB_IO_ERROR;
    return status;
}

db_status_t find_pet_index_by_owner_and_name(FILE *f, size_t owner_id, const char *petname, size_t *index)
{
    pet_t pet;
    size_t count = 0;
    db_status_t status = DB_OK;
    size_t i = 0;
    long int cur = -1;

    if (f == NULL || petname == NULL || index == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(f, sizeof(pet_t), &count);
    if (status == DB_OK)
        status = DB_PET_NOT_FOUND;
    if (status == DB_PET_NOT_FOUND)
    {
        cur = ftell(f);
        if (cur < 0 || fseek(f, 0, SEEK_SET) != 0)
            status = DB_IO_ERROR;
    }
    for (i = 0; i < count && status == DB_PET_NOT_FOUND; i++)
    {
        if (fread(&pet, sizeof(pet_t), 1, f) != 1)
        {
            status = DB_IO_ERROR;
            break;
        }
        if (strcmp(pet.pet_name, petname) == 0 && pet.owner_id == owner_id)
        {
            *index = i;
            status = DB_OK;
        }
    }
    if (cur >= 0 && fseek(f, cur, SEEK_SET) != 0)
        status = DB_IO_ERROR;
    return status;
}

db_status_t next_user_id(FILE *f, size_t *count)
{
    db_status_t status = DB_OK;

    if (f == NULL || count == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(f, sizeof(user_t), count);
    if (status == DB_OK)
        *count += 1;
    return status;
}

db_status_t next_pet_id(FILE *f, size_t *count)
{
    db_status_t status = DB_OK;

    if (f == NULL || count == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(f, sizeof(pet_t), count);
    if (status == DB_OK)
        *count += 1;
    return status;
}

db_status_t user_exists_by_id(FILE *f, size_t id)
{
    user_t user;
    size_t count = 0;
    db_status_t status = DB_OK;
    size_t i = 0;
    long int cur = -1;

    if (f == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(f, sizeof(user_t), &count);
    if (status == DB_OK)
        status = DB_USER_NOT_FOUND;
    if (status == DB_USER_NOT_FOUND)
    {
        cur = ftell(f);
        if (cur < 0 || fseek(f, 0, SEEK_SET) != 0)
            status = DB_IO_ERROR;
    }
    for (i = 0; i < count && status == DB_USER_NOT_FOUND; i++)
    {
        if (fread(&user, sizeof(user_t), 1, f) != 1)
        {
            status = DB_IO_ERROR;
            break;
        }
        if (user.user_id == id)
            status = DB_OK;
    }
    if (cur >= 0 && fseek(f, cur, SEEK_SET) != 0)
        status = DB_IO_ERROR;
    return status;
}

db_status_t db_init_storage(FILE **users, FILE **pets)
{
    db_status_t status = DB_OK;
    FILE *local_users = NULL;
    FILE *local_pets = NULL;

    if (users == NULL || pets == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = ensure_storage_dir();
    if (status == DB_OK)
    {
        local_users = open_storage_file(USERS_DB_PATH);
        local_pets = open_storage_file(PETS_DB_PATH);
        if (local_users == NULL || local_pets == NULL)
            status = DB_IO_ERROR;
    }
    if (status == DB_OK)
    {
        *users = local_users;
        *pets = local_pets;
    }
    else
    {
        if (local_users != NULL)
            fclose(local_users);
        if (local_pets != NULL)
            fclose(local_pets);
    }
    return status;
}

db_status_t db_validate_storage(FILE *users, FILE *pets)
{
    size_t cnt = 0;
    db_status_t status = DB_OK;
    long int cur = 0;

    if (users == NULL || pets == NULL)
        status = DB_NULL_PTR;
    if (status == DB_OK)
        status = count_records(pets, sizeof(pet_t), &cnt);
    if (status == DB_OK)
    {
        cur = ftell(pets);
        if (cur < 0 || fseek(pets, 0, SEEK_SET) != 0)
            status = DB_IO_ERROR;
    }
    for (size_t i = 0; i < cnt && status == DB_OK; i++)
    {
        pet_t pet;
        db_status_t user_status = DB_OK;

        if (fread(&pet, sizeof(pet_t), 1, pets) != 1)
            status = DB_IO_ERROR;
        if (status == DB_OK)
        {
            user_status = user_exists_by_id(users, pet.owner_id);
            if (user_status == DB_USER_NOT_FOUND)
                status = DB_DATA_CORRUPT;
            else if (user_status != DB_OK)
                status = user_status;
        }
    }
    if (cur >= 0 && fseek(pets, cur, SEEK_SET) != 0)
        status = DB_IO_ERROR;
    return status;
}
