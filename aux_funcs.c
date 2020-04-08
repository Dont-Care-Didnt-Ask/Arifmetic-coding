#include "yet_another_ari.h"
#include <stdio.h>

/* Auxilary functions */

void init_table(unsigned *table)
{
    for (int i = 0; i < TABLE_LEN; i++) {
        table[i] = i << 8;
    }
}

unsigned init_value(FILE * src_file)
{
    unsigned value = 0;
    for (int i = 0; i < INT_SIZE; i++) {
        value = value << CHAR_BIT | fgetc(src_file);
    }
    return value;
}

void normalize_table(unsigned * table)
{
    // FILE *dbg = fopen_or_die("table-log.txt", "a");
    // for (int i = 0; i < TABLE_LEN; i++) {
    //     fprintf(dbg, " %x", table[i]);
    // }
    // fprintf(dbg, "\n\n");

    printf("Normalizing\n");
    unsigned new_freq, accum = (table[0] + 1) >> 1;
    for (int cnt = 0; cnt < TABLE_LEN - 1; cnt++) {
        new_freq = table[cnt + 1] - table[cnt];     // get current frequency
        new_freq = (new_freq + 1) >> 1;             // divide it in half
        table[cnt] = accum;                         // write down new frequency of previous element
        accum += new_freq;                          // calculate new value for current element
    }
    table[TABLE_LEN - 1] = accum; // renew last element

    unsigned cashback = 0;  // let's make sure frequency of every symbol isn't less than 1
    for (int cnt = 1; cnt < TABLE_LEN; cnt++) {
        if (table[cnt] - cashback == table[cnt - 1]) {
            cashback++;
        }
        table[cnt] += cashback;
    }
    // for (int i = 0; i < TABLE_LEN; i++) {
    //     fprintf(dbg, " %x", table[i]);
    // }
    // fprintf(dbg, "\n\n\n");

    // fclose(dbg);
}

void renew_table(unsigned * table, int sym)
{
    for (unsigned cnt = sym; cnt < TABLE_LEN; cnt++) {   // renew frequencies
        table[cnt] += AGRESSIVNESS;
    }
    if (table[TABLE_LEN - 1] > MAX_FREQ) {
        normalize_table(table);
    }
}