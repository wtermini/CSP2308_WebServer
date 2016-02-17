#ifndef MYFILES_H_   /* Include guard */
#define MYFILES_H_

#include "stdio.h"
#include "stdlib.h"
#include "limits.h"
#include "unistd.h"
#include "fcntl.h"



FILE *loadFile(const char filename[CHAR_MAX],const char mode[CHAR_MAX]);
//int displayFile(const char *filename);
int displayFile(FILE *file);

int checkFile(const char *filename);
int checkPrivilege(const char *filename,const char *mode);

#endif