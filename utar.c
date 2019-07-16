/* David Messick     */
/* May 29, 2019      */
/* CS3411            */
/* Assignment 1 utar */

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define FILENAMESIZE 256
#define MIN(a,b) ((((a)-(b))&0x80000000) >> 31) ? (a) : (b)

/*Struct for fileheader*/
typedef struct {
    int file_size;
    char deleted;
    char file_name[FILENAMESIZE];
} hdr;

/*Wrapper for write()*/
void printWrapper(char buf[1024]) {
    write(STDOUT_FILENO, buf, strlen(buf));
}

/*Checks to see if user entered y or n for a file overwrite*/
char getYorN(){
    char buf[1024];
    char retVal;
    bool done;

    done = false;

    do {
        read(STDIN_FILENO, buf, 2);
        if ((buf[0] == 'y') || (buf[0] == 'Y')) {
            done = true;
            retVal = 'Y';
        }
        else if ((buf[0] == 'n') || (buf[0] == 'N')) {
            done = true;
            retVal = 'N';
        }
        else {
            sprintf(buf, "Enter Y or N. ");
            printWrapper(buf);
        }
    } while(!done);
    return retVal;
}

/*Extracts the file*/
int extractFile(int arcFile) {
    int newFile;
    int count;
    int cpyCount;
    bool writeFile;
    hdr newFileHeader;
    char fileBuf[1024];
    char buf[1024];
    char name[FILENAMESIZE];

    count = read(arcFile, &newFileHeader, sizeof(newFileHeader));
    if(count == 0) return 1;
    if(count != sizeof(newFileHeader)) {
        sprintf(buf, "Invalid secondary file header.\n");
        printWrapper(buf);
        return 1;
    }
    cpyCount = newFileHeader.file_size;
    if (newFileHeader.deleted == 1) writeFile = false;
    else {
        strncpy(name, newFileHeader.file_name, sizeof(name));
        if (access(name, F_OK) != -1) {
            sprintf(buf, "File %s already exists. Do you wish to overwrite it? (y/n) ", name);
            printWrapper(buf);
            writeFile = (getYorN() == 'Y');
        }
        else writeFile = true;
    }
    if (writeFile) newFile = open(name, O_WRONLY | O_CREAT, 0644);
    while (cpyCount > 0) {           
        count = read(arcFile, fileBuf, MIN(sizeof(fileBuf), cpyCount));
        if(count != 0) {
            if(writeFile) write(newFile, fileBuf, count);
            cpyCount -= count;
        }
    }
    if (writeFile) close(newFile);
    return 0;
}

/*Main*/
int main(int argc, char **argv) {
    int arcFile;
    int count;
    hdr arcFileHeader;
    const char nameFlag[] = "CS3411 TAR";
    char buf[1024];
    
    arcFile = open(argv[1], O_RDONLY, 0644);    
    count = read(arcFile, &arcFileHeader, sizeof(arcFileHeader));

    if(count != sizeof(arcFileHeader) || (arcFileHeader.file_size != -1) || (arcFileHeader.deleted != 0) || (strstr(arcFileHeader.file_name, nameFlag) == NULL)) {
        sprintf(buf, "Not a valid archive file!\n");
        printWrapper(buf);
    }
    else while (extractFile(arcFile) == 0);

    return 0;
}
