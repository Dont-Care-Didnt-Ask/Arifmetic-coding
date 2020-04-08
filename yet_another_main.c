#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "yet_another_ari.h"

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("Need input file, tmp file and destination file in command line arguments\n");
        return 1;
    }
    compress_ari(argv[1], argv[2]);
    decompress_ari(argv[2], argv[3]);
    return 0;
}
