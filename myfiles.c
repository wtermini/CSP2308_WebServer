//checks if file exists

#include "stdio.h"
#include "stdlib.h"
#include "limits.h"
#include "unistd.h"
#include "fcntl.h"
#include "myfiles.h"
#include "string.h"
#include "unistd.h"
#include "mywebserver.h"
#include "mylog.h"


//myconfload.c for CSP2308 Unix and C
//William Termini 10344438
//10 April 2015
//Contains functions to load and print files


//Takes files and puts them into memmory
FILE *loadFile(const char filename[4096],const char mode[CHAR_MAX])
{
    FILE *file = NULL;
    //Check to see if file exists this is checked twice when loading pages unless we are going to write
    if( checkFile(filename)!=404)
    {
        //check permissions
        if(checkPrivilege(filename,mode)==200)
        {
            //GOOD NEWS WE CAN OPEN THE FILE ((>'-')> <('-'<) ^(' - ')^ <('-'<) (>'-')>)
            printf("\nOpening File %s!\n\n",filename);
            file = fopen(filename,mode);
            return file;
        }
        else
        {
            //Return Empty File
            return file;
        }
    }
    else if(!strcmp(mode,"a"))
    {
        printf("\nmakeing new file\n");
        creat(filename,0666);
        file = fopen(filename,"w");
        return file;
    }
    else
    {
        //Also Returns and Empty File
        return file;
    }
    //Yet again, another empty file (programer had to much monster and its 4:51 AM)
    return file;
}


//opens and prints a file to stdout


//check exist
int checkFile(const char *filename)
{
    //ONLY CHECK IF FILE EXISTS
    if( (access(filename, F_OK)) != -1)
    {
        printf("\nFile %s exists!\n",filename);
        return 1;
    }
    else
    {
        printf("File %s can not be found\n",filename);
        return 404;
    }

};

//check your privilege
int checkPrivilege(const char *filename,const char *mode)
{
    char modelong[8];
    int result;
    if (strcmp(mode,"r")==0)
    {
        strcpy(modelong,"READ");
        result = (access(filename,R_OK));
    }
    else if ((strcmp(mode,"w")==0)||(strcmp(mode,"a")==0))
    {
        result = (access(filename,W_OK));
        strcpy(modelong,"WRITE");
    }
    else
    {
        //user should never, ever ever ever ever see this, if this happens the code is wrong
        printf("ERROR: This should never happen!\n");
        return -1;
    }

    //Check the results
    if (result!=-1)
    {
        //File can be read
        printf("Sufficent privileges to %s file %s\n",modelong,filename);
        return 200;
    }
    else
    {
        printf("Insufficent privilege to %s file %s\n",modelong,filename);
        return 403;
    }

}

//Takes the contents of one file and
int fileSock();
