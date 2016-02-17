#include "stdio.h"
#include "stdlib.h"
#include "myfiles.h"
#include "myconfload.h"
#include "limits.h"
#include "unistd.h"
#include "fcntl.h"
#include "string.h"
#include "signal.h"
#include "mywebserver.h"

#define configfile "mywebserver.conf"
#define documentrootvar "DocumentRoot="
#define portnumbervar "Port="
#define indexfilevar "IndexFile="
#define logfilevar "LogFile="
#define interfacevar "Interface="


//myconfload.c for CSP2308 Unix and C
//William Termini 10344438
//22 May 2015
//Contains functions needed to read the config file


//Returns a config struct to the calling function
struct config loadConf()
{

    struct config configstruct;
    FILE *conf = NULL;
    conf = loadFile(configfile,"r");
    if(conf!=NULL)
    {
        parseConf(&configstruct.port,portnumbervar,conf);	//loads port
        parseConf(&configstruct.docroot,documentrootvar,conf); //loads document root
        parseConf(&configstruct.index,indexfilevar,conf); //loads index file location
        parseConf(&configstruct.log,logfilevar,conf); //loads log file location
		 parseConf(&configstruct.interface,interfacevar,conf); //loads an optinal interface

        //failsafe for documentrootvar (adds / to the path if was not provided in config file)
        char slashtest;
        slashtest = configstruct.docroot[strlen(configstruct.docroot) - 1] ;
        if (slashtest!='/')
        {
            strcat(configstruct.docroot, "/");
        }
        //close conf
        fclose(conf);
        return(configstruct);
    }
    else
    {
        printf("\nError: Config file %s not found SHUTING DOWN!\n\n",configfile);
	    fflush(stdout);
		 kill(g_mainpid,SIGTERM);
		 return(configstruct);
    }
}

//Takes a pointer to a char (RVAR) that the value will be returned to, a char with the values we are looking for and a while to parse
int parseConf(char(*rvar)[127] ,const char var[CHAR_MAX],FILE *FP)
{
    rewind(FP); //Be kind rewind

    char line[CHAR_MAX];

    while(fgets(line,sizeof(line),FP)!=NULL)
    {
        //removes end of line character
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        if(line[0] == '#') continue; //I do not wish to comment
        char *cfline;

        //finds the value we are looking for
        if((strstr((const char *)line,var)!=0))
        {
            cfline = strstr((const char *)line,var);
            cfline = cfline + strlen(var);
            //Checks if the value we got is NULL
            if(cfline[0]=='\n'||cfline[0]=='\0')
            {
                fprintf(stderr,"\nError:config file blank value for %s!\n",var);
				  kill(g_mainpid,SIGTERM);
				  fflush(stderr);
				  return 0;
            }
            //printf("\nThe value of %s %s\n",var,cfline);
            //Copys the value of cfline the pointer RVAR
            memcpy(rvar,cfline,sizeof(char[CHAR_MAX]));
            return 0;
        }
        else
        {
        }
    }
     if(!strcmp(interfacevar,var))
	 {
		 memcpy(rvar,"0.0.0.0",sizeof("0.0.0.0"));
		 return 0;
	 }
    fprintf(stderr,"\nError: Error in config file missing %s!\n",var);
	 kill(g_mainpid,SIGTERM);
	 fflush(stderr);
	 return 0;
}