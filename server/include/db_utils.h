#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <stddef.h>

#include "db.h"

db_status_t ensure_storage_dir(void);
db_status_t strip(const char *input, char *output, size_t output_size);
db_status_t find_user_index_by_name(FILE *f, const char *name, size_t *index);
db_status_t find_pet_index_by_owner_and_name(FILE *f, size_t owner_id, const char *petname, size_t *index);
db_status_t next_user_id(FILE *f, size_t *count);
db_status_t next_pet_id(FILE *f, size_t *count);
db_status_t user_exists_by_id(FILE *f, size_t id);
db_status_t db_init_storage(FILE **users, FILE **pets);
db_status_t db_validate_storage(FILE *users, FILE *pets);
db_status_t count_records(FILE *f, size_t record_size, size_t *count);

#endif
