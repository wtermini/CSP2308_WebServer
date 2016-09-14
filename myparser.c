//parse user commands
#include "stdio.h"
#include "stdlib.h"
#include "myfiles.h"
#include "limits.h"
#include "myconfload.h"
#include "unistd.h"
#include "fcntl.h"
#include "string.h"
#include "mywebserver.h"
#include "myresponse.h"
#include "mylog.h"


#define RESET_COLOR "\e[m"
#define MAKE_RED "\e[31m"


//myparser.c for CSP2308 Unix and C
//William Termini 10344438
//10 April 2015
//Takes user input and parses the commands issued, also contains the
//Code used to parse HTTP requests and print them on screen


// This is the parser, this takes user inputs and then does one of the following
void parser(char *header[CHAR_MAX],FILE **returnfile,char *buf)
{
    //struct config g_configstruct;
    //g_configstruct = g_configstruct;
    //Creates a buffer that will be used to hold the user input
    //This hold the name of the command that is to be executed
    char command[CHAR_MAX];
    memset(command, 0, sizeof(*buf));
    //Argument 1 hold in most cases the location of a file
    char arg1[CHAR_MAX];
    memset(arg1, 0, sizeof(arg1));
    //Get the user input
    //Parse out the command issued
    sscanf(buf,"%s",command);
    char filepath[4096];
    //HTTP REQUEST PARSE
    if (strcmp(command, "GET")==0) {
        sscanf(buf,"%s %s",command,arg1);
        //Trys to detect if the user trys to perform a directory traversal attack
        if(strstr((char *)arg1,"../")!=0)
        {
            fprintf(stderr, MAKE_RED "\nAttempted Directory Traversal! Reported to the Cyber Police!\n" RESET_COLOR);
        }
        //If argument 1 is not empty, and is not only a "/" concatinate the strings of docroot and arg 1 to form file path
        if((((strcmp(arg1,"/")!=0))&&arg1[0]!='\0')||(((strcmp(arg1,"/")==0)&&arg1[1]!='\0')))
        {
            strcpy(filepath, g_configstruct.docroot);
            strcat(filepath, arg1);
            *returnfile = loadFile(filepath,"r");
        }
        //If argument 1 is not provided use the default index file set in the config file
        else
        {
            strcpy(filepath, g_configstruct.docroot);
            strcat(filepath, g_configstruct.index);
            *returnfile = loadFile(filepath,"r");
        }
    }
    //If the user enters nothing end the function
    else if (strcmp(command, "")==0)
    {
        return;
    }
    else
    {
        return;
    }
    char logresponse[CHAR_MAX];
    //Now that we have the files we need to build a header
    //If the returned file is NULL, check why its NULL 404 or 403
	if(*returnfile==NULL)
    {
		 if((checkFile(filepath)==404))
		 {
			 strcpy(header,response_404); 
             printf(response_404);
			 strcat(logresponse,"404 ERROR can not find FILE ");

		 }
		 else
		 {
			strcpy(header,response_403); 
			strcat(logresponse,"403 ERROR insufficient permissions for FILE ");
		 }
        strcat(logresponse,filepath);
        sendLog(logresponse);
         *returnfile=NULL;
		 return;
		
    }
    else
    {   
		//Build response
		char tmpheader[CHAR_MAX];
		 strcat(tmpheader,response_200);
		 char fsize[CHAR_MAX];
		 
		 
	    char *ext = strrchr(filepath, '.');
		//printf("%s",ext);
		if(!strcmp(ext,".html")||!(strcmp(ext,".htm")))
		{
		 strcat(tmpheader,"Content-type: text/html\n");
		}
		else if(!strcmp(ext,".png"))
		{
		 strcat(tmpheader,"Content-type: image/png\n");
		}
		else if(!strcmp(ext,".jpg")||!(strcmp(ext,".jpeg")))
		{
		 strcat(tmpheader,"Content-type: image/jpg\n");
		}
		else if(!strcmp(ext,".gif"))
		{
		 strcat(tmpheader,"Content-type: image/gif\n");
		}
		
		//Get file size
		 fseek(*returnfile, 0, SEEK_END);
		 int len = (unsigned long)ftell(*returnfile);
		 rewind(*returnfile);
		 //printf("%i",len);
		 sprintf(fsize, "%i", len);
		 strcat(tmpheader,"Content-Length:");
		 strcat(tmpheader,fsize);
		 strcat(tmpheader,"\n");
		 //strcat(tmpheader,"Connection: Keep-Alive\n");
		 strcat(tmpheader,"\n");
	    fflush(stdout);
        strcpy(header,tmpheader);
		
		//log
        strcat(logresponse,"200 OK opened FILE ");
        strcat(logresponse,filepath);
        sendLog(logresponse);
    }
    return;

}
