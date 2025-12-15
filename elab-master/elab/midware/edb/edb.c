/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "edb.h"
#include "../../common/elab_assert.h"
#include "../../os/cmsis_os.h"

ELAB_TAG("edb");

/* private config ----------------------------------------------------------- */
#define EDB_BUFFER_SIZE                         (256)

/* private typedef ---------------------------------------------------------- */
struct ini_t
{
    char *data;
    char *end;
};

/* private function prototypes ---------------------------------------------- */
static int32_t _get_string_index(char *buff_line, uint8_t index,
                                    char *str, uint32_t size);
static uint32_t _get_string(const char *section,
                            const char *key,
                            char *str,
                            uint32_t size);

typedef struct ini_t ini_t;

ini_t*      ini_load_file(const char *filename);
ini_t*      ini_load_string(const char *str);
void        ini_free(ini_t *ini);
const char* ini_get(ini_t *ini, const char *section, const char *key);
int         ini_sget(ini_t *ini, const char *section, const char *key, const char *scanfmt, void *dst);
const char* int_get_key(ini_t *ini, const char *section, uint32_t index);
int         int_get_section_size(ini_t *ini, const char *section);

/* private variables -------------------------------------------------------- */
static ini_t *edb_ini = NULL;
static osMutexId_t *mutex_edb = NULL;
static char buff_line[EDB_BUFFER_SIZE];

/**
 * The edb global mutex attribute.
 */
static const osMutexAttr_t mutex_attr_edb =
{
    "mutex_edb", osMutexRecursive | osMutexPrioInherit, NULL, 0U 
};

/* public function ---------------------------------------------------------- */
void edb_init(uint8_t mode, const char *path_or_str)
{
    elab_assert(mode == EDB_LOAD_MODE_FILE || mode == EDB_LOAD_MODE_STRING);

    if (mode == EDB_LOAD_MODE_FILE)
    {
        edb_ini = ini_load_file(path_or_str);
    }
    else
    {
        edb_ini = ini_load_string(path_or_str);
    }
    elab_assert(edb_ini != NULL);

    mutex_edb = osMutexNew(&mutex_attr_edb);
    elab_assert(mutex_edb != NULL);
}

uint32_t edb_get_key_num(const char *section)
{
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = int_get_section_size(edb_ini, section);
    elab_assert(ret_ini >= 0);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return (uint32_t)ret_ini;
}

const char *edb_get_key(const char *section, uint32_t index)
{
    osStatus_t ret_os = osOK;
    char *key = NULL;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    key = (char *)int_get_key(edb_ini, section, index);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return (const char *)key;
}

uint32_t edb_get_hex32(const char *section, const char *key)
{
    uint32_t value_u32 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%x", &value_u32);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_u32;
}

uint32_t edb_get_u32(const char *section, const char *key)
{
    uint32_t value_u32 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%u", &value_u32);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_u32;
}

int32_t edb_get_s32(const char *section, const char *key)
{
    int32_t value_s32 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%d", &value_s32);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_s32;
}

uint64_t edb_get_u64(const char *section, const char *key)
{
    uint32_t value_u64 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%lu", &value_u64);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_u64;
}

int64_t edb_get_s64(const char *section, const char *key)
{
    uint32_t value_s64 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%ld", &value_s64);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_s64;
}

bool edb_get_bool(const char *section, const char *key)
{
    bool value = false;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    const char *str = ini_get(edb_ini, section, key);
    elab_assert(str != NULL);

    if (strncmp(str, "true", 4) == 0)
    {
        value = true;
    }
    else if (strncmp(str, "false", 5) == 0)
    {
        value = false;
    }
    else
    {
        printf("%s.\n", str);
        elab_assert(false);
    }

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value;
}

float edb_get_float(const char *section, const char *key)
{
    float value = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%f", &value);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value;
}

double edb_get_double(const char *section, const char *key)
{
    double value = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%lf", &value);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value;
}

uint32_t edb_get_string(const char *section,
                        const char *key,
                        char *str,
                        uint32_t size)
{
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t size_cp = _get_string(section, key, str, size);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return size_cp;
}

bool edb_str_cmp(const char *section, const char *key, const char *str)
{
    bool ret = false;

    osStatus_t ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t size_cp = _get_string(section, key, buff_line, (EDB_BUFFER_SIZE - 1));
    if (strcmp(buff_line, str) == 0)
    {
        ret = true;
    }

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return ret;
}

uint32_t edb_get_sub_u32(const char *section, const char *key, uint8_t index)
{
    uint32_t value = 0;

    #define EDB_BUFF_SIZE_U32                   (12)

    char buff[EDB_BUFF_SIZE_U32];
    memset(buff, 0, EDB_BUFF_SIZE_U32);
    edb_get_sub_string(section, key, index, buff, EDB_BUFF_SIZE_U32);
    value = atoi(buff);

    #undef EDB_BUFF_SIZE_U32

    return value;
}

float edb_get_sub_float(const char *section, const char *key, uint8_t index)
{
    float value = 0;

    #define EDB_BUFF_SIZE_FLOAT                   (12)

    char buff[EDB_BUFF_SIZE_FLOAT];
    memset(buff, 0, EDB_BUFF_SIZE_FLOAT);
    edb_get_sub_string(section, key, index, buff, EDB_BUFF_SIZE_FLOAT);
    value = atof(buff);

    #undef EDB_BUFF_SIZE_U32

    return value;
}

uint32_t edb_get_sub_string(const char *section,
                            const char *key,
                            uint8_t index,
                            char *str, uint32_t size)
{
    uint32_t ret_size = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t size_cp = _get_string(section, key, buff_line, (EDB_BUFFER_SIZE - 1));
    ret_size = _get_string_index(buff_line, index, str, (size - 1));

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return ret_size;
}

/* private function --------------------------------------------------------- */
static uint32_t _get_string(const char *section,
                            const char *key,
                            char *str,
                            uint32_t size)
{
    const char *_str = ini_get(edb_ini, section, key);
    elab_assert(str != NULL);

    uint32_t size_cp = strlen(_str);
    size_cp = ((size - 1) < size_cp) ? (size - 1) : size_cp;
    memcpy(str, _str, size_cp);
    str[size_cp] = 0;

    return size_cp;
}

static int32_t _get_string_index(char *buff_line, uint8_t index,
                                    char *str, uint32_t size)
{
    int32_t ret = 0;
    char *str_start = buff_line;
    uint32_t count_cp = 0;

    if (index > 0)
    {
        uint8_t interval_count = 0;
        bool existent = false;
        for (uint32_t i = 0; i < strlen(buff_line); i ++)
        {
            if (buff_line[i] == ',')
            {
                interval_count ++;
                if (interval_count == index)
                {
                    str_start = &buff_line[i + 1];
                    existent = true;
                    break;
                }
            }
        }

        if (!existent)
        {
            ret = -1;
            elog_error("Buffer %s has not so much intervals.", buff_line);
            goto exit;
        }
    }

    while (1)
    {
        str[count_cp] = str_start[count_cp];
        count_cp ++;
        if (count_cp > size || str_start[count_cp] == '\n' ||
            str_start[count_cp] == '\r' || str_start[count_cp] == ',')
        {
            break;
        }
    }

exit:
    return ret;
}


/* Case insensitive string compare */
static int strcmpci(const char *a, const char *b)
{
    for (;;)
    {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
        {
            return d;
        }
        a++, b++;
    }
}

/* Returns the next string in the split data */
static char* next(ini_t *ini, char *p)
{
    p += strlen(p);
    while (p < ini->end && *p == '\0')
    {
        p++;
    }
    return p;
}

static void trim_back(ini_t *ini, char *p)
{
    while (p >= ini->data && (*p == ' ' || *p == '\t' || *p == '\r'))
    {
        *p-- = '\0';
    }
}

static char* discard_line(ini_t *ini, char *p)
{
    while (p < ini->end && *p != '\n')
    {
        *p++ = '\0';
    }
    return p;
}


static char *unescape_quoted_value(ini_t *ini, char *p) {
    /* Use `q` as write-head and `p` as read-head, `p` is always ahead of `q`
    * as escape sequences are always larger than their resultant data */
    char *q = p;
    p++;
    while (p < ini->end && *p != '"' && *p != '\r' && *p != '\n')
    {
        if (*p == '\\')
        {
            /* Handle escaped char */
            p++;
            switch (*p)
            {
                default   : *q = *p;    break;
                case 'r'  : *q = '\r';  break;
                case 'n'  : *q = '\n';  break;
                case 't'  : *q = '\t';  break;
                case '\r' :
                case '\n' :
                case '\0' : goto end;
            }
        }
        else
        {
            /* Handle normal char */
            *q = *p;
        }
        q++, p++;
    }
    end:
    return q;
}


/* Splits data in place into strings containing section-headers, keys and
 * values using one or more '\0' as a delimiter. Unescapes quoted values */
static void split_data(ini_t *ini)
{
    char *value_start, *line_start;
    char *p = ini->data;

    while (p < ini->end)
    {
        switch (*p)
        {
        case '\r':
        case '\n':
        case '\t':
        case ' ':
            *p = '\0';
            /* Fall through */

        case '\0':
            p++;
            break;

        case '[':
            p += strcspn(p, "]\n");
            *p = '\0';
            break;

        case ';':
            p = discard_line(ini, p);
            break;

        default:
            line_start = p;
            p += strcspn(p, "=\n");

            /* Is line missing a '='? */
            if (*p != '=')
            {
                p = discard_line(ini, line_start);
                break;
            }
            trim_back(ini, p - 1);

            /* Replace '=' and whitespace after it with '\0' */
            do
            {
                *p++ = '\0';
            } while (*p == ' ' || *p == '\r' || *p == '\t');

            /* Is a value after '=' missing? */
            if (*p == '\n' || *p == '\0')
            {
                p = discard_line(ini, line_start);
                break;
            }

            if (*p == '"')
            {
                /* Handle quoted string value */
                value_start = p;
                p = unescape_quoted_value(ini, p);

                /* Was the string empty? */
                if (p == value_start) {
                    p = discard_line(ini, line_start);
                    break;
                }
                /* Discard the rest of the line after the string value */
                p = discard_line(ini, p);
            }
            else
            {
                /* Handle normal value */
                p += strcspn(p, "\n");
                trim_back(ini, p - 1);
            }
            break;
        }
    }
}



ini_t* ini_load_file(const char *filename)
{
    ini_t *ini = NULL;
    FILE *fp = NULL;
    int n, sz;

    /* Init ini struct */
    ini = malloc(sizeof(*ini));
    if (!ini)
    {
        goto fail;
    }
    memset(ini, 0, sizeof(*ini));

    /* Open file */
    fp = fopen(filename, "rb");
    if (!fp)
    {
        goto fail;
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    rewind(fp);

    /* Load file content into memory, null terminate, init end var */
    ini->data = malloc(sz + 1);
    ini->data[sz] = '\0';
    ini->end = ini->data  + sz;
    n = fread(ini->data, 1, sz, fp);
    if (n != sz)
    {
        goto fail;
    }

    /* Prepare data */
    split_data(ini);

    /* Clean up and return */
    fclose(fp);
    return ini;

fail:
    if (fp) fclose(fp);
    if (ini) ini_free(ini);

    return NULL;
}

ini_t* ini_load_string(const char *str)
{
    ini_t *ini = NULL;
    int n = 0;
    int sz = strlen(str);

    /* Init ini struct */
    ini = malloc(sizeof(*ini));
    elab_assert (ini != NULL);
    memset(ini, 0, sizeof(*ini));

    /* Load file content into memory, null terminate, init end var */
    ini->data = malloc(sz + 1);
    memcpy(ini->data, str, sz);
    ini->data[sz] = '\0';
    ini->end = ini->data + sz;

    /* Prepare data */
    split_data(ini);

    /* Clean up and return */
    return ini;
}

void ini_free(ini_t *ini)
{
    free(ini->data);
    free(ini);
}


const char* ini_get(ini_t *ini, const char *section, const char *key)
{
    char *current_section = "";
    char *val = NULL;
    char *p = ini->data;

    if (*p == '\0')
    {
        p = next(ini, p);
    }

    while (p < ini->end)
    {
        if (*p == '[')
        {
            /* Handle section */
            current_section = p + 1;
        }
        else
        {
            /* Handle key */
            val = next(ini, p);
            if (!section || !strcmpci(section, current_section))
            {
                if (!strcmpci(p, key))
                {
                    return val;
                }
            }
            p = val;
        }

        p = next(ini, p);
    }

    return NULL;
}

int int_get_section_size(ini_t *ini, const char *section)
{
    char *current_section = "";
    char *val = NULL;
    char *p = ini->data;
    int count = 0;

    if (*p == '\0')
    {
        p = next(ini, p);
    }

    while (p < ini->end)
    {
        if (*p == '[')
        {
            /* Handle section */
            current_section = p + 1;
        }
        else
        {
            /* Handle key */
            val = next(ini, p);
            if (!section || !strcmpci(section, current_section))
            {
                count ++;
            }
            p = val;
        }

        p = next(ini, p);
    }

    return count;
}

const char* int_get_key(ini_t *ini, const char *section, uint32_t index)
{
    int size = int_get_section_size(ini, section);
    assert(index < size);

    char *current_section = "";
    char *val = NULL;
    char *p = ini->data;
    int count = 0;

    if (*p == '\0')
    {
        p = next(ini, p);
    }

    while (p < ini->end)
    {
        if (*p == '[')
        {
            /* Handle section */
            current_section = p + 1;
        }
        else
        {
            /* Handle key */
            if (!section || !strcmpci(section, current_section))
            {
                if (count == index)
                {
                    return p;
                }
                count ++;
            }
            val = next(ini, p);
            p = val;
        }

        p = next(ini, p);
    }

    return NULL;
}

int ini_sget(ini_t *ini, const char *section, const char *key,
                const char *scanfmt, void *dst)
{
    const char *val = ini_get(ini, section, key);
    if (!val)
    {
        return 0;
    }
    if (scanfmt)
    {
        sscanf(val, scanfmt, dst);
    }
    else
    {
        *((const char**) dst) = val;
    }

    return 1;
}

/* ----------------------------- end of file -------------------------------- */
