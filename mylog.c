#include "stdio.h"
#include "stdlib.h"
#include "myfiles.h"
#include "limits.h"
#include "myconfload.h"
#include "unistd.h"
#include "time.h"
#include "mywebserver.h"
#include "string.h"
#include "signal.h"


//srvlog is is ment to run as a child
void srvlog()
{
//Set up the timer for the logs
    time_t timer;
    char eventtime[26];
    struct tm* tm_info;

    FILE *log = NULL;
//Load the config to find the log file location
    int logstatus=0;
    printf("logging child born\n");
    fflush(stdout);
    char buf[1024];

    while(1)
    {

        //Update the time
        time(&timer);
        tm_info = localtime(&timer);
        strftime(eventtime, 26, "%Y:%m:%d %H:%M:%S", tm_info);
        memset(buf,0,1024);
        g_logfifofd = open(g_logfifo,O_RDONLY);
        if(read(g_logfifofd, buf, 1024))
        {
            //This test is for the compiler
        }
        else
        {
            fprintf(stderr,"SOMETHING WENT WRONG!");
        }

        if(!strcmp(buf,"status_ready"))
        {
            logstatus = 1;
            printf("\n\nLOG: %s \n", buf);
            printf ("\n\nlogstatus = %i\n\n",logstatus);
            log = loadFile(g_configstruct.log,"a");
            if(log==NULL)
            {
				  logstatus = 2;
                 printf("LOG FILE COULD NOT BE LOADED! SHUTING DOWN!\n"); 
				  kill(g_mainpid,SIGTERM);
            }
			
            else
            {
                 printf("Logging Started!\n");
                fprintf(log,"%s: Server Initialize & Logging Started\n",eventtime);
            }
        }
        else if(!strcmp(buf,"status_reload"))
        {
            fclose(log);
            g_configstruct = loadConf();
            log = loadFile(g_configstruct.log,"a");
        }
        else if(!strcmp(buf,"status_shutdown"))
        {
            fprintf(log,"%s: Logging Stopped application closing\n",eventtime);
            fflush(log);
            fclose(log);
            close(g_logfifo);
            printf("Log file closed\n");
            exit(0);
        }
		else if(logstatus==2)
        {
			printf("LOG: %s\n",buf);
			printf("LOG: DATA SENT TO LOG BUT THERE IS NO LOG!!!\n");
			fflush(stdout);
        }
        else if(logstatus==1)
        {
			 if(checkPrivilege(g_configstruct.log,"w")!=403)
			 {
            printf("LOG: %s\n",buf);
            fprintf(log,"%s: %s\n",eventtime,buf);
            fflush(log);
			 }
			 else
			 {
				  logstatus = 2;
				  printf("LOG NO LONGER WRITABLE! SHUTING DOWN!\n"); 
				  kill(g_mainpid,SIGTERM);
			 }

        }
        else
        {
            printf("\nWARNING CONFIG NOT LOADED\n");
            printf ("\n\nlogstatus = %i\n\n",logstatus);
        }
        close(g_logfifofd);
        fflush(stdout);
    }
}


//Send log is a simple function that allows other functions
//from other proccess to send something to the log file
int sendLog(char *logmsg)
{
    g_logfifofd = open(g_logfifo, O_WRONLY);
    if(write(g_logfifofd, logmsg, CHAR_MAX))
    {
        //To stop warning: ignoring return value of 'write'
    }
    else
    {
        fprintf(stderr,"something went wrong!\n");
    }
    return 0;
}