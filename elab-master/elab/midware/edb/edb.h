/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EDB_H
#define EDB_H

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* public define ------------------------------------------------------------ */
enum edb_load_mode
{
    EDB_LOAD_MODE_FILE = 0,
    EDB_LOAD_MODE_STRING,
};

/* public define ------------------------------------------------------------ */
void edb_init(uint8_t mode, const char *path_or_str);
uint32_t edb_get_hex32(const char *section, const char *key);
uint32_t edb_get_u32(const char *section, const char *key);
int32_t edb_get_s32(const char *section, const char *key);
uint64_t edb_get_u64(const char *section, const char *key);
int64_t edb_get_s64(const char *section, const char *key);
bool edb_get_bool(const char *section, const char *key);
float edb_get_float(const char *section, const char *key);
double edb_get_double(const char *section, const char *key);
uint32_t edb_get_string(const char *section,
                        const char *key,
                        char *str,
                        uint32_t size);

uint32_t edb_get_key_num(const char *section);
const char *edb_get_key(const char *section, uint32_t index);

bool edb_str_cmp(const char *section, const char *key, const char *str);
uint32_t edb_get_sub_u32(const char *section, const char *key, uint8_t index);
float edb_get_sub_float(const char *section, const char *key, uint8_t index);
uint32_t edb_get_sub_string(const char *section,
                            const char *key,
                            uint8_t index,
                            char *str, uint32_t size);

#endif

/* ----------------------------- end of file -------------------------------- */