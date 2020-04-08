#ifndef YET_ANOTHER_ARI_H
#define YET_ANOTHER_ARI_H

#include <stdio.h>
#include <limits.h>

/* Constants */

#define QUARTER   (UINT_MAX / 4u + 1)
#define HALF      (2 * QUARTER)
#define THIRD_QTR (3 * QUARTER)

enum bit_consts
{
    FULL_BYTE = CHAR_BIT,
    TABLE_LEN = 258,
    EOF_SYM   = 256,
    INT_SIZE  = sizeof(int),
    FULL_INT  = INT_SIZE * CHAR_BIT,
    MASK      = 0xff
};

enum
{
    AGRESSIVNESS   = 0x10000,
    MAX_FREQ       = QUARTER,
    SEEMS_LIKE_EOF = -5,
    TBL_ERR        = -2,
    FILE_ERR       = -1
};

/* Functions */

void compress_ari(char *in_filename, char *out_filename);
void decompress_ari(char *in_filename, char *out_filename);
/*
static unsigned encode(int sym, unsigned *table, FILE *out, FILE *dbg);
static int decode(unsigned *table, FILE *in, FILE *out, FILE *dbg);
*/
void init_table(unsigned *table);
unsigned init_value(FILE * src_file);
void renew_table(unsigned * table, int sym);
void normalize_table(unsigned * table);

unsigned bit_plus_follow(unsigned bit, unsigned follow, FILE * file_ptr, FILE *dbg);
void write_last_bits(unsigned output_state, FILE * file_ptr, FILE *dbg);
int get_byte(unsigned long long value, FILE *in, FILE *out, FILE *dbg);

FILE * fopen_or_die(char * name, char * mode);

#endif