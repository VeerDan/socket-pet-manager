#ifndef ERRORS_H
#define ERRORS_H

typedef enum
{
    DB_OK = 0,
    DB_NULL_PTR,
    DB_INVALID_NAME,
    DB_DUPLICATE_USER,
    DB_DUPLICATE_PET,
    DB_USER_NOT_FOUND,
    DB_PET_NOT_FOUND,
    DB_DATA_CORRUPT,
    DB_IO_ERROR,
    DB_NO_MEMORY
} db_status_t;

#endif
