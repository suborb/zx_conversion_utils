/*
 *   +D disc to .TAP converter
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
 *   Compile with gcc -opdread pdread.c
 *
 *   This program takes a disc image as a flat file (eg dd if=/dev/fd0 
 *   of=diskimage) and allows inspection and dumping of the files contained
 *   within to .TAP files
 *
 *   The flag -a allows disc images that use alternate sides to be read
 *  
 *   Minimal version - best usage is:
 *   ./pdread diskimage
 *   >dir
 *   [Check all files are there]
 *   >saveall
 *
 *   Then append the files to each other to make an appropriate .tap file
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DIR_SECTORS 20
#define DIR_TRACKS  4

static void    saveall();
static void    list_directory();
static int     find_dir_entry(char filename[10], unsigned char buf[256]);
static void    savefile(char *buf);
static void    save_tapfile_from_dir_entry(unsigned char *buf);
static int     get_tracksec_offset(int track, int sector);
static void    writeword_p(unsigned int i, FILE *fp,unsigned char *p);
static void    writebyte_p(unsigned char c, FILE *fp,unsigned char *p);

static int     alternate = 0;
static FILE   *disk;


static void usage(char *progname)
{
    printf("%s [-a] diskimage\n\n",progname);
    printf("-a  Disk image contains alternate tracks\n");
    exit(1);
}


int main(int argc, char *argv[])
{
	char	buffer[80];
    char   *filen = NULL;
    int     i;

    for ( i = 1; i < argc; i++ ) {
        if ( argv[i][0] == '-' ) {
            switch ( argv[i][1] ) {
            case 'a':
                alternate = 1;
                break;
            case 'h':
                usage(argv[0]);
                break;
            }
        } else {
            filen = argv[i];
        }
    }
    if ( filen == NULL ) {
        usage(argv[0]);
    }

	if ( ( disk = fopen(filen,"rb") ) == NULL ) {
        printf("Can't open disc\n");
        usage(argv[0]);
    }

	while ( 1 ) {
		printf("> ");
        buffer[0] = 0;
		fgets(buffer,79,stdin);

		if (strncmp(buffer,"dir",3) == 0 )
			list_directory();
		if (strncmp(buffer,"exit",4) == 0 ) {
			break;
        } else if (strncmp(buffer,"saveall",7) == 0 ) {
            saveall();
        } else if (strncmp(buffer,"save",4) == 0 ) {
			savefile(buffer);
        } else if ( strlen(buffer) == 0 ) {
            break;
        }
	}
	fclose(disk);

    exit(0);
}

/** \brief Save all the files on disc
 */
static void saveall()
{
	unsigned char	buffer[512];
    int             i,j,offset;

    for ( i = 0 ; i < DIR_TRACKS; i++ ) {
        for ( j = 0; j < 10; j++ ) {
            offset = get_tracksec_offset(i,j);
            fseek(disk, offset, SEEK_SET);
            fread(buffer,1,512,disk);
            if (buffer[0] ) {	/* File there... */
                save_tapfile_from_dir_entry(&buffer[0]);
            }
            if (buffer[256]) {	/* File there... */
                save_tapfile_from_dir_entry(&buffer[256]);
            }
        }
    }
}

/*
 *	Directory listing 
 */
static void list_directory()
{
	char	buffer[512];
	int	i,j,offset,files = 0;
	char	name[11];

    for ( i = 0 ; i < DIR_TRACKS; i++ ) {
        for ( j = 0; j < 10; j++ ) {
            offset = get_tracksec_offset(i,j);
            fseek(disk, offset, SEEK_SET);
            fread(buffer,1,512,disk);
            if (buffer[0]) {	/* File there... */
                strncpy(name,&buffer[1],10);
                name[10]=0;
                printf("Type=%d %s\n",buffer[0],name);
                files++;
            }
            if (buffer[256]) {	/* File there... */
                strncpy(name,&buffer[257],10);
                name[10]=0;
                printf("Type=%d %s\n",buffer[256],name);
                files++;
            }
        }
    }
    printf("%d files\n",files);
}

/** \brief Find a directory entry for the file
 *
 *  \param filename Filename to find
 *  \param buf      Destination for directory entry
 *
 *  \retval 0 - Found
 *  \retval - 1 - Not found
 */
static int find_dir_entry(char filename[10], unsigned char buf[256])
{
    char       buffer[512];
    int        i,j;
    int        offset;

    for ( i = 0 ; i < DIR_TRACKS ; i++ ) {
        for ( j = 0; j < 10 ; j++ ) {
            offset = get_tracksec_offset(i,j);
            fseek(disk, offset, SEEK_SET);
            fread(buffer,1,512,disk);
            printf("%s\n%s\n",&buffer[1], &buffer[257]);
            if (strncmp(&buffer[1],filename,10) == 0 ) {
                memcpy(buf,buffer,256);
                return 0;
            }
            if (strncmp(&buffer[257],filename,10) == 0 ) {
                memcpy(buf,buffer + 256, 256);
                return 0;
            }
        }
    }
    return -1;
}


static void savefile(char *buf)
{
	char	buffer[256];
	char	name[20];
	char	outname[20];

	printf("name of +d file>");
	fgets(name,19,stdin);
    name[strlen(name)-1] = 0;
	sscanf(buf,"%s %s",outname,outname);
	printf("Saving file %s --> %s\n",name,outname);


    if ( find_dir_entry(name,buffer) == -1 ) {
		printf("File not found\n");
		return;
	}

    save_tapfile_from_dir_entry(buffer);
}

/** \brief Save a file from a dir entry
 *
 *  \param buf The directory entry
 */
static void save_tapfile_from_dir_entry(unsigned char *buf)
{
    char    filename[FILENAME_MAX+1];
    unsigned char    secbuf[512];
	int	    track;
	int	    sec,maxlen;
    int     offset;
    FILE   *fpout;
    int     i,j;
    char   *ptr;
    unsigned char parity = 0;

    switch ( buf[211] ) {
    case 0:
    case 1:
    case 2:
    case 3:
        break;
    default:
        printf("Unhandled filetype %d (%10s)\n",buf[211],buf+1);
        return;
    }

	track=buf[0xD]; sec=buf[0xE];
    maxlen = buf[212] + (buf[213] * 256);
    printf("File length is %d\n",maxlen);
    sprintf(filename,"%10s",buf+1);

    ptr = filename + 9;
    while (*ptr == ' ' && ptr > filename) {
        *ptr = 0;
        ptr--;
    }
    strcat(filename,".tap");



    printf("Saving file <%10s> to <%s>\n",buf+1,filename);

    if ( ( fpout = fopen(filename,"wb") ) == NULL ) {
        printf("Can't open %s\n",filename);
        return;
    }

    offset=get_tracksec_offset(track,sec);
	fseek(disk,offset,SEEK_SET);
    fread(secbuf,1,512,disk);

    /* First 10 bytes of the file are the header */
    writeword_p(19,fpout,&parity);        /* Header len */
    writebyte_p(0,fpout,&parity);        /* Header is 0 */
    parity=0;
    writebyte_p(buf[211],fpout,&parity);
    for ( i = 1; i < 11; i++ ) {
        writebyte_p(buf[i],fpout,&parity);
    }
    /* Write out control information here */
    writeword_p(maxlen, fpout, &parity);
    switch ( buf[211] ) {
    case 0:   /* BASIC */
        i = buf[218] + buf[219] * 256;     /* autostart line */
        printf("Autostart %d\n",i);
        writeword_p(i,fpout,&parity);
        i = buf[216] + buf[217] * 256;
        writeword_p(i,fpout,&parity); /* Length without vars */
        break;
    case 1:  /* Number array */
    case 2:  /* $ array */
        i = buf[216] + buf[219] * 256;  /* Name */
        writeword_p(i,fpout,&parity);
        i = 0; /* Ignored */
        writeword_p(i,fpout,&parity);
        break;
    case 3:  /* Code */
        i = buf[214] + buf[215] * 256;    /* start */
        writeword_p(i,fpout,&parity);
        i = 0; /* Unused */
        writeword_p(i,fpout,&parity);
        break;
    }
    writebyte_p(parity,fpout,&parity);

    writeword_p(maxlen + 2, fpout,&parity);
    parity = 0;
    writebyte_p(255,fpout,&parity);        /* Data */

    j = 0;
    /* Now write out the data */
    for ( i = 9; i < 510 && maxlen > 0 ; i++, maxlen-- ) {
        writebyte_p(secbuf[i],fpout,&parity);
        j++;
    }

    track = secbuf[510];
    sec = secbuf[511];

    while ( track && sec ) {
        offset=get_tracksec_offset(track,sec);
        fseek(disk,offset,SEEK_SET);
        fread(secbuf,1,512,disk);
        for ( i = 0; i < 510 && maxlen > 0 ; i++, maxlen-- ) {
            writebyte_p(secbuf[i],fpout,&parity);
            j++;
        }
        track = secbuf[510];
        sec = secbuf[511];
    }
    writebyte_p(parity,fpout,&parity);
    printf("Left to write is %d %d\n",maxlen,j);
    fclose(fpout);
}

static int get_tracksec_offset(int track, int sector)
{
    if ( alternate ) {
        if ( track >= 128 )
            return ((10*(track-128 - 79)+sector-1)*512);
        else
            return ((20 * track + sector -1 ) * 512);

    } else {
        if	(track>=128) 
            return ((10*(track-128)+sector-1)*512);
        else
            return ((10*track+sector-1)*512);
    }
}


	
static void writeword_p(unsigned int i, FILE *fp,unsigned char *p)
{
    writebyte_p(i%256,fp,p);
    writebyte_p(i/256,fp,p);
}


static void writebyte_p(unsigned char c, FILE *fp,unsigned char *p)
{
    fputc(c,fp);
    *p^=c;
}

