#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "yet_another_ari.h"

/* Global variables */

unsigned long long value = 0;
unsigned long long left = 0, right = UINT_MAX;
static unsigned follow = 0;
static unsigned bits_gone = 0, glo_buf = 0;

static char * debug_filename = "log.txt";

/* Head functions */

static unsigned encode(int sym, unsigned *table, FILE *out, FILE *dbg) {
    unsigned output_state;
    unsigned long long divisor = table[TABLE_LEN-1];
    unsigned long long cur_len = right - left + 1;

    right = left + (cur_len * table[sym]) / divisor - 1;
    left = left + (cur_len * table[sym-1]) / divisor;
/*
    fprintf(dbg, "sym = %d(%c), divisor = %8llx, cur_len = %8llx, right = %8llx, left = %8llx\n",\
        sym, sym, divisor, cur_len, right, left);
*/
    for(;;) {

        if (right < HALF) {                 // interval belongs to [0, 1/2)

            output_state = bit_plus_follow(0, follow, out, dbg);  // so we can write down bit 0
            follow = 0;

        } else if (left >= HALF) {         // interval belongs to [1/2, 1)

            output_state = bit_plus_follow(1, follow, out, dbg);  // so we can write down bit 1
            follow = 0;
            left -= HALF;               // first bit is written, we don't need it any more
            right -= HALF;

        } else if ((left >= QUARTER) && (right < THIRD_QTR)) {  // inteval lies in [1/4, 3/4)

            follow++;           // so we will write in file either 01..1 or 10..0
            left -= QUARTER;
            right -= QUARTER;

        } else {
            break;
        }

        left  += left;
        right += right + 1;     // prepare next bit
    } // encoded current symbol
    
    if (left < QUARTER) {
        output_state |= 1 << (2 * CHAR_BIT); // calculate offset
    }
    return output_state;
}

void compress_ari(char *in_filename, char *out_filename)
{
    FILE *in, *out, *debug;
    in = fopen_or_die(in_filename, "rb");
    out = fopen_or_die(out_filename, "wb");
    debug = fopen_or_die(debug_filename, "w");
    setbuf(debug, NULL);
    //fprintf(debug, "Let's start\n");

    printf("Compressing:\n");

    unsigned table[TABLE_LEN];
    init_table(table);
//encode
    for (int sym = fgetc(in); sym != EOF; sym = fgetc(in)) {
        encode(sym, table, out, debug);
        renew_table(table, sym);
    }
    unsigned output_state = encode(EOF_SYM, table, out, debug); // write down last symbol

    write_last_bits(output_state, out, debug);

    printf("table['a'-1] = %x\ntable['a'] = %x\ntable['b'] = %x\n", table['a'-1], table['a'], table['b']);

    fclose(in);
    fclose(out);
    fclose(debug);
}

static int decode(unsigned *table, FILE *in, FILE *out, FILE *dbg) {
    //setbuf(out, NULL);
    unsigned long long divisor = table[TABLE_LEN - 1];
    unsigned long long cur_len = right - left + 1;

    int sym;
/*
    if (value >= left + table[TABLE_LEN-1] * cur_len/divisor) {
        printf("Seeking symbol error\nvalue = %llx, left = %llx, right = %llx, max_freq = %x, cur_len = %8llx\n",\
            value, left, right, table[TABLE_LEN-1], cur_len);
        _exit(TBL_ERR);
    }
    for (sym = 1; value >= left + table[sym] * cur_len / divisor; sym++);   // find current symbol
*/

    unsigned long long freq = ((value - left + 1) * divisor - 1) / cur_len;
    if (table[TABLE_LEN-1] < freq){
        printf("Seeking symbol error\nvalue = %llx, left = %llx, right = %llx, max_freq = %x, freq = %8llx\n",\
            value, left, right, table[TABLE_LEN-1], freq);
        _exit(TBL_ERR);
    }
    for (sym = 1; table[sym] <= freq; sym++);

    //printf("Got %c(%d)\n", sym, sym);
    if (sym == EOF_SYM) {
        return sym;
    } else {
        fputc(sym, out);
    }

    right = left + table[sym] * cur_len / divisor - 1;
    left  = left + table[sym-1] * cur_len / divisor;
/*
    fprintf(dbg, "sym = %d(%c), divisor = %8llx, cur_len = %8llx, [%8llx > %8llx > %8llx]\n",\
        sym, sym, divisor, cur_len, right, value, left);
*/  
    for (;;) {
        if (right < HALF) {
            ;
        } else if (left >= HALF) {
            left  -= HALF;
            right -= HALF;
            value -= HALF; 
        } else if ((right < THIRD_QTR) && (left >= QUARTER)) {
            left  -= QUARTER;
            right -= QUARTER;
            value -= QUARTER;
        } else {
            break;
        }
        
        left  += left;
        right += right + 1;
        value += value + (glo_buf >> 7);    // adds first bit from buffer
        
        glo_buf = (glo_buf << 1) & MASK;    // shifts new bit forward
        bits_gone++;
        if (bits_gone >= CHAR_BIT) {
            glo_buf = get_byte(value, in, out, dbg);
            bits_gone = 0;
        }
    } // decoded one symbol
    //fprintf(dbg, "sym = %d, left = %8llx, right = %8llx, value = %8llx\n", sym, left, right, value);
    return sym;
}

void decompress_ari(char *in_filename, char *out_filename)
{
    FILE * in, *out, *debug;
    in = fopen_or_die(in_filename, "rb");
    out = fopen_or_die(out_filename, "wb");
    debug = fopen_or_die(debug_filename, "a");
    setbuf(debug, NULL);

    printf("Decompressing:\n");
    //fprintf(debug, "\n\nDecompression:\n");

// Initialize context
    left = 0, right = UINT_MAX; 
    unsigned table[TABLE_LEN];
    init_table(table);
    value = init_value(in);
    glo_buf = fgetc(in);

    //fprintf(debug, "value = %llx\n", value);
// decode
    int sym;
    while ((sym = decode(table, in, out, debug)) != EOF_SYM) {
        renew_table(table, sym);
    }

    fclose(out);
    fclose(in);
    fclose(debug);
}
