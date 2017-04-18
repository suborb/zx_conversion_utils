/*
 *   Convert RAW PLUS3DOS files to .TAP files
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *   This program takes files with a PLUS3DOS header and creates a tap file out the
 *   listed files
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>


static void save_file(FILE *fpout, uint8_t *header, size_t datalen, uint8_t *data);
static void writeword_p(uint16_t i, FILE *fp,unsigned char *p);
static void writebyte_p(uint8_t c, FILE *fp,unsigned char *p);

static uint8_t image[65536];

int main(int argc, char **argv)
{
    FILE  *writer;
    int    i,j;
    uint8_t *ptr;
    size_t len;
    uint8_t header[17];

    // TODO: Allow this to be configured
    if ( (writer = fopen("output.tap", "wb")) == NULL ) {
        exit(1);
    }

    for ( i = 1; i < argc; i++ ) {
        FILE *reader = fopen(argv[i],"rb");

        if ( reader == NULL ) {
            printf("Couldn't open file <%s> for reading\n",argv[i]);
            continue;
        }
        len = fread(image, 1, sizeof(image),reader);
        fclose(reader);

        if ( strncmp((char *)image,"PLUS3DOS",8) == 0 ) {
            ptr = image + 128;
            len = image[11] + (image[12] << 8) + (image[13] <<16) + (image[14] << 24) - 0x80;
            header[0] = image[15];          // File type
            snprintf(header+1, 11, "%-10s", argv[i]);
            memcpy(header + 11, image + 16, 6); // Copy across rest of header
            header[11] = len % 256;
            header[12] = len / 256;
         } else {
            ptr = image;
            header[0] = 3; // Code
            snprintf(header+1, 11, "%-10s", argv[i]);
            header[11] = len % 256;
            header[12] = len / 256;
            header[13] = 0;  // Start address
            header[14] = 0;
            header[15] = 0;  // xxx
            header[16] = 0;
        }
        for ( j = 1; j < 11; j++ ) {
            header[j] = toupper(header[j]);
        }
        printf("Saving %d\n",len);
        save_file(writer, header, len, ptr);
    }
    fclose(writer);
}

static void save_file(FILE *fpout, uint8_t *header, size_t datalen, uint8_t *data)
{
    uint8_t parity = 0;
    int     i;

   /* First 10 bytes of the file are the header */
    writeword_p(19,fpout,&parity);        /* Header len */
    writebyte_p(0,fpout,&parity);         /* Header is type 0 */
    parity = 0;
    for ( i = 0; i < 17; i++ ) {
        writebyte_p(header[i],fpout,&parity);
    }
    writebyte_p(parity,fpout,&parity);

    writeword_p(datalen + 2, fpout,&parity);
    parity = 0;
    writebyte_p(255,fpout,&parity);        /* Data */

    /* Now write out the data */
    for ( i = 0; i < datalen; i++ ) {
        writebyte_p(data[i],fpout,&parity);
    }

    writebyte_p(parity,fpout,&parity);
}


	
static void writeword_p(uint16_t i, FILE *fp,unsigned char *p)
{
    writebyte_p(i%256,fp,p);
    writebyte_p(i/256,fp,p);
}


static void writebyte_p(uint8_t c, FILE *fp,unsigned char *p)
{
    fputc(c,fp);
    *p^=c;
}