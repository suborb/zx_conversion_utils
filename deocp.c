/*
 *   Convert OCP Editor Assembler Files to Text
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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Converts an OCP from the OCP Editor Assembler (possibly only the version
 *   that I was using) to normal text files which can be cross assembled.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char  buf[65536];

int main(int argc, char *argv[])
{
    unsigned char *ptr;
    FILE *fp;
    size_t len;

    if ( ( fp = fopen(argv[1],"r")) == NULL ) {
        exit(1);
    }

    len = fread(buf, 1, sizeof(buf),fp);

    if ( strncmp(buf,"PLUS3DOS",8) == 0 ) {
       ptr = buf + 128 + 5;
       len = buf[11] + buf[12] * 256 + buf[13] * 65536 + buf[14] * (65536 * 256);
    } else {
       ptr = buf + 5;
    }

    int fields = 4;
    while ( ptr < buf + len ) {
        int segment_length = *ptr++;

        fields--;
        if ( fields == 0 ) {
            printf("\n");
            fields = 4;
            ptr += 2;
            continue;
        }

        for ( int i = 0; i < segment_length; i++ ) {
            printf("%c",*ptr++);
        }
        if ( fields == 3  && segment_length > 0) {
            printf(":");
        }
        printf("\t");
    }
}
