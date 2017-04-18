/*
 *   +3 disc to file converter
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
 *   This program takes a disc image as a flat file (eg dd if=/dev/fd0 
 *   of=diskimage) and dumps all files to disc.
 *
 *   Only guaranteed to work with 720k discs that I dumped
 *
 *   Written due to a lack of joy with libdsk and cpmtool, cpcs, cpcxfs
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define DIR_USER 0x00
#define DIR_FILENAME  0x01
#define DIR_EXTENT 0x0C
#define DIR_BYYECOUNT 0x0B
#define DIR_EXTENT2 0x0E
#define DIR_RECORD_COUNT 0x0F
#define DIR_ALLOCATION 0x10


#define MAX_DIR_ENTRY 256


unsigned char image[1024 * 1024];

struct dir_entry {
    char     name[20];
    uint16_t extents[300];   /* Maximum number of extents, we never get there! */
};


static struct dir_entry   directory[MAX_DIR_ENTRY];



uint32_t get_offset_for_block(int block);
static uint32_t  get_offset(int track, int sector);
static void read_directory();
static void save_files();

int main(int argc, char **argv)
{   
    FILE *fp;
    if ( (fp = fopen(argv[1],"r")) == NULL ) {
        exit(1);
    }
    fread(image, 1, sizeof(image), fp);
    fclose(fp);

    /* We now have the image in image */
    read_directory();
    save_files();

}


static void save_files()
{
    int i, j, k, block;  
    FILE *fp;

    for ( i = 0; i < MAX_DIR_ENTRY; i++ ) {
        if ( directory[i].name[0] == 0x00 ) {
            break;
        }
        printf("Saving %s\n",directory[i].name);
        fp = fopen(directory[i].name, "w");

        for ( j = 0; (block = directory[i].extents[j]) != 0; j++ ) {
                int sector = block * 4;
                int track = (sector / 9) + 1;
                sector = sector % 9;
                printf("Saving %s block %d track %d/%d\n",directory[i].name,block,track,sector);

                k = 0;
                do { 
                    uint32_t offset = get_offset(track, sector);

                    fwrite(image + offset, 1, 512, fp);

                    sector++;
                    if ( sector == 9 ) {
                        sector = 0;
                        track++;
                    }
                    k++;
                } while ( k < 4 );
        }
    }
}


struct dir_entry *get_direntry(unsigned char *filename, unsigned char *ext)
{
    char  buf[20];
    int   offs = 0;
    int i;

    for ( i = 0; i < 8; i++ ) {
        if ( !isspace(filename[i])) {
            buf[offs++] = tolower(filename[i]);
        }
    }
    for ( i = 0; i < 3; i++ ) {
        if ( !isspace(ext[i])) {
            if ( i == 0 ) {
                buf[offs++] = '.';
            }
            buf[offs++] = tolower(ext[i]);
        }
    }
    buf[offs] = 0;

    for ( i = 0;  i < MAX_DIR_ENTRY; i++ ) {
        if ( directory[i].name[0] == 0x00 ) {
            break;
        }
        if ( strcmp(directory[i].name,buf) == 0 ) {
            return &directory[i];
        }
    }
    strcpy(directory[i].name, buf);
    return &directory[i];
}

static void read_directory()
{
    unsigned char *ptr;

    for ( int block = 0; block < 2; block++ ) {
        uint32_t offset = get_offset_for_block(block);
        printf("Block %d %x\n",block,offset);
        for ( int i = 0; i < 64; i++ ) {
            ptr = image + offset + ( i * 32);

            int extent = *(ptr + DIR_EXTENT);
            if ( *ptr < 15 ) {
                struct dir_entry *entry;

                entry = get_direntry(ptr+1,ptr+9);
                
                memcpy(&entry->extents[extent * 8], ptr + DIR_ALLOCATION, 16);
            }
        }
    }
}


uint32_t get_offset_for_block(int block)
{
    int sector = block * 4;

    int track = (sector / 9) + 1;
    sector = sector % 9;

    return get_offset(track,sector);
}

/* Get the offset */
uint32_t  get_offset(int track, int sector)
{
    int otrack = track;
    uint32_t offs = 0;
    if ( track < 80 ) {
        offs = (track * 2) * (9 * 512) + (sector * 512);

    } else {
        track = 159 - track;
        //track = track - 80;
        offs = (track * 2) * (9 * 512) + (sector * 512) + (9 * 512);
    }
    printf("Track %d offset %x\n",otrack,offs);
    return offs;
}