/* David Messick	  */
/* May 28, 2019		  */
/* CS3411		 	  */
/* Assignment 1	 ctar */

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*Struct for fileheader*/
typedef struct {
	int file_size;
	char deleted;
	char file_name[256];
} hdr;

/*A wrapper for write()*/
void printWrapper(char buf[2048]) {
	write(1, buf, strlen(buf));
}

/*Writes the mainHeader of the archive*/
int mainHeaderWrite(int fd, char file_name[256]) {
	hdr mainHeader;
	mainHeader.file_size = -1;
	mainHeader.deleted = 0;
	strncpy(mainHeader.file_name, "CS3411 TAR ", sizeof(mainHeader.file_name));
    strcat(mainHeader.file_name, file_name);
	write(fd, &mainHeader, sizeof(mainHeader));
    return 0;
}

/*Writes the specified file header*/
int fileHeaderWrite(int fd, int file_size, char file_name[256]) {
    hdr fileHeader;
    fileHeader.file_size = file_size;
    fileHeader.deleted = 0;
    strncpy(fileHeader.file_name, file_name, sizeof(fileHeader.file_name));
    write(fd, &fileHeader, sizeof(fileHeader));
    return 0;
}

/*Writes the data from a file to the archive*/
int fileDataWrite(int mainFile, int fd) {
    int count;
    char buf[1024];

    count = read(fd, buf, sizeof(buf));
    while (count > 0) {
        write(mainFile, buf, count);
        count = read(fd, buf, sizeof(buf));
    }
    return 0;
}

/*Appends to the archive when flagged*/
void appendFile(int argc, char **argv) {
 	struct stat st;
	int mainFile;
    int fileDesc;
    int size;
    int i = 0;
    char buf[1024]; 

    /*If file exists, append*/
    if (access(argv[2], F_OK) != -1) {
        mainFile = open(argv[2], O_WRONLY | O_APPEND, 0644);
        for(i=3; i<argc; i++) {
            if (access(argv[i], F_OK) != -1) {
                stat(argv[i], &st);
                size = st.st_size;
                fileHeaderWrite(mainFile, size, argv[i]);
                fileDesc = open(argv[i], O_RDONLY, 0644);
                fileDataWrite(mainFile, fileDesc);
                close(fileDesc);            
            }
            else {
                sprintf(buf, "File %s does not exist! File skipped.\n", argv[i]);
                printWrapper(buf);
            }
        }
    }

    /*File does not exist, create*/
    else {
        mainFile = open(argv[2], O_WRONLY | O_CREAT, 0644);
        mainHeaderWrite(mainFile, argv[2]);
        for(i=3; i<argc; i++) {
            if (access(argv[i], F_OK) != -1) {
                stat(argv[i], &st);
                size = st.st_size;
                fileHeaderWrite(mainFile, size, argv[i]);
                fileDesc = open(argv[i], O_RDONLY, 0644);
                fileDataWrite(mainFile, fileDesc);
                close(fileDesc);
            }
            else {
                sprintf(buf, "File %s does not exist! File skipped.\n", argv[i]);
                printWrapper(buf);
            }
        }
    }

    fsync(mainFile);
    close(mainFile);
}

/*Finds the specified fileheader*/
bool findFileHeader(int mainFile, hdr *fileHeader, char *fileName) {
	int count;
	bool done = false;
	bool retVal;
	char buf[1024];
	
	do {
		count = read(mainFile, fileHeader, sizeof(hdr));
		if (count != sizeof(hdr)) {
			retVal = false;
			done = true;
		}
		else {
			if ((strcmp(fileHeader->file_name, fileName) == 0) && (fileHeader->deleted == 0)) {
				retVal = true;
				done = true;
			}
			else {
				if (lseek(mainFile, fileHeader->file_size, SEEK_CUR) == -1) {
					sprintf(buf, "File corrupted.");
					printWrapper(buf);
					retVal = false;
					done = true;
				}
			}
		}
	} while (!done);
	
	return retVal;
}

/*Marks a file as deleted from the archive when flagged*/
void deleteFile(int argc, char **argv) {
	hdr mainFileHeader;
	hdr fileHeader;
	int mainFile;
	int count;
    int i = 0;
    char buf[1024]; 
	const char nameFlag[] = "CS3411 TAR";

    /*If file exists*/
    if (access(argv[2], F_OK) != -1) {
        mainFile = open(argv[2], O_RDWR, 0644);
		count = read(mainFile, &mainFileHeader, sizeof(mainFileHeader));

		if(count != sizeof(mainFileHeader) || (mainFileHeader.file_size != -1) || (mainFileHeader.deleted != 0) || (strstr(mainFileHeader.file_name, nameFlag) == NULL)) {
			sprintf(buf, "Not a valid archive file! No files deleted.\n");
			printWrapper(buf);
			return;
		}
	
		else for(i=3; i<argc; i++) {
			if (findFileHeader(mainFile, &fileHeader, argv[i])) {
				lseek(mainFile, -sizeof(fileHeader), SEEK_CUR);
				fileHeader.deleted = 1;
				write(mainFile, &fileHeader, sizeof(fileHeader));
				lseek(mainFile, sizeof(fileHeader), SEEK_SET);
			}
		}
		fsync(mainFile);
		close(mainFile);
	}

    /*File does not exist error*/
    else {
        sprintf(buf, "Archive file %s does not exist!\n", argv[2]);
        printWrapper(buf);
    }
}

/*Main*/
int main(int argc, char **argv) {
    char flag;
    char buf[1024];

    if (argc < 3) {
        sprintf(buf, "Not enough arguments.\n");
        printWrapper(buf);
        return 1;
    }

    if (!strcmp(argv[1], "-a")) flag = 'a';
    if (!strcmp(argv[1], "-d")) flag = 'd';
    if ((flag != 'a') && (flag != 'd')) {
        sprintf(buf, "Error. Please use a valid operation flag!\n");
        printWrapper(buf);
        return 1;
    }

    /*Create or append*/
    if (flag == 'a') {
        appendFile(argc, argv);
    }
    
    if (flag == 'd') {
       deleteFile(argc, argv);  
    }

	return 0;
}
