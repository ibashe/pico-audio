#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define BUF_SIZE	1024*10
#define BUF_MAX     70000

#if 0
#define SAMPLE_BIT_DEPTH     8
#else
#define SAMPLE_BIT_DEPTH     13
#endif

// Input:  16 bits signed
// output:  SAMPLE_BIT_DEPTH

int main(int argc, char *argv[])
{
    FILE *fin=NULL, *fout=NULL;
    int ReadBytes, i;
    int16_t buf[BUF_SIZE];
    char buffer[50];
    int counter = 0;

    snprintf(buffer, strlen(argv[1])-3, "%s\n", argv[1]);
    strcat(buffer, ".h");

    fout = fopen(buffer, "w");
    fin  = fopen(argv[1], "rb");

    if( (fin==NULL) || (fout==NULL) ) {
        printf("Error: check %s, %s\n", argv[1], argv[2]);
        return -1;
    }

    fprintf(fout, "#define SAMPLE_BIT_DEPTH       %d\n", SAMPLE_BIT_DEPTH);
    fprintf(fout, "#define AUDIO_SAMPLES       %d\n\n", BUF_MAX);
    fprintf(fout, "uint16_t %s[] = {\n", "audio_buffer");


    while( (ReadBytes=fread(buf, sizeof(buf[1]), BUF_SIZE, fin)) == BUF_SIZE) {
        for(i=0; i<BUF_SIZE; i++) {
            fprintf(fout, "    0x%x,\n", ((buf[i] >> (16 - SAMPLE_BIT_DEPTH)) + (1 << (SAMPLE_BIT_DEPTH-1))) & ((1 << SAMPLE_BIT_DEPTH) - 1) );
        }

        // exit early due PICO flash memory capacity
        counter += ReadBytes;
        if(counter >= BUF_MAX) {
            ReadBytes = 0;
            break;
        }
    }

    if(ReadBytes > 0) {
        for(i = 0; i < ReadBytes; i++) 
            fprintf(fout, "    0x%x,\n", ((buf[i] >> (16 - SAMPLE_BIT_DEPTH)) + (1 << (SAMPLE_BIT_DEPTH-1))) & ((1 << SAMPLE_BIT_DEPTH) - 1) );
    }

    fprintf(fout, "};\n");

    fclose(fin);
    fclose(fout);

    return 0;
}
