#include "yet_another_ari.h"
#include <stdio.h>
#include <unistd.h>

extern unsigned long long left, right;

FILE * fopen_or_die(char * name, char * mode) {
    FILE * tmp = fopen(name, mode);
    if (!tmp) {
        printf("Cannot open %s\n", name);
        _exit(FILE_ERR);
    }
    return tmp;
}

unsigned bit_plus_follow(unsigned bit, unsigned follow, FILE * file_ptr, FILE *dbg)
{
    static unsigned char out_byte = 0, ready_bits = 0;
    
    out_byte = (out_byte << 1) | bit;   // add bit to the buffer
    ready_bits++;
    if (ready_bits == FULL_BYTE) {
        // fprintf(dbg, "  follow = %d, ", follow);
        // fprintf(dbg, "  wrote byte %hhx\n", out_byte);
        fputc(out_byte, file_ptr);
        ready_bits = out_byte = 0;
    }

    while (follow--) {
        out_byte = (out_byte << 1) | !bit;
        ready_bits++;
        if (ready_bits == FULL_BYTE) {
            // fprintf(dbg, "  follow = %d, ", follow);
            // fprintf(dbg, "  wrote byte %hhx\n", out_byte);
            fputc(out_byte, file_ptr);
            ready_bits = out_byte = 0;
        }
    }
    /*
    I return both current state of the buffer and amount of ready bits 
    in one variable of unsigned int type, using it as a structure
    */
    return out_byte | (ready_bits << CHAR_BIT); 
}

int get_byte(unsigned long long value, FILE *in, FILE *out, FILE *dbg) {
    static int eof_cnt = 0;
    int buf = fgetc(in); // get next byte
    //fprintf(dbg, "  Current value = %8llx (right = %8llx, left = %8llx), read byte %2hhx, ", value, right, left, buf);
    if (buf == EOF) {         // check if it's the end
        buf = 0x80;     // Why not. (We can put random data here. 0x80 will get us to the center of current interval)
        eof_cnt++;
        if (eof_cnt > 3) {          // when we meet eof for the first time,
            fclose(in);             // we still have valid bits in "value" 
            fclose(out);            // so I wait for three EOFs 
            printf("Seems like EOF\n");
            _exit(SEEMS_LIKE_EOF);
        }                           
    }
    
    //fprintf(dbg, "  New value = %8llx\n\n", value | buf);
    return buf;
}

void write_last_bits(unsigned output_state, FILE * file_ptr, FILE * dbg)
{
    /* output_state contains {offset | readybits | out_byte}, bytewise*/
    unsigned out_byte = output_state & MASK;
    unsigned ready_bits = (output_state >> CHAR_BIT) & MASK;
    int offset = output_state >> (2*CHAR_BIT);
    /* 
    I want to add a couple of bits to the sequence I have. It will be either 01 or 10. 
    Which pair - depends on offset parametr.
    */
    unsigned bit;
    if (offset) {
        bit = 0;
    } else {
        bit = 1;
    }
    out_byte = (out_byte << 1) | bit;
    ready_bits++;

    if (ready_bits == 8) {
        fputc(out_byte, file_ptr);
        //fprintf(dbg, "  wrote byte %hhx\n", out_byte);
        ready_bits = out_byte = 0;
    }
    out_byte = (out_byte << 1) | !bit;
    ready_bits++;

    out_byte <<= (FULL_BYTE - ready_bits);
    //fprintf(dbg, "  wrote byte %hhx\n\n", out_byte);
    fputc(out_byte, file_ptr);
    ready_bits = out_byte = 0;

    return;
}
