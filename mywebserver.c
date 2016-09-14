#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "myfiles.h"
#include "myconfload.h"
#include "myparser.h"
#include "signal.h"
#include "stdio.h"
#include "signal.h"
#include "netdb.h"
#include "netinet/in.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "myresponse.h"
#include "mylog.h"
#include "sys/wait.h"
#include "mywebserver.h"
#include "string.h"
#include <sys/prctl.h>
#include <arpa/inet.h>

// myparser.c for CSP2308 Unix and C
// William Termini 10344438
// 10 April 2015
// Contains main logic, first it loads the webserver config into ram
// Then it passes the config to a loop which runs the command parser.

volatile sig_atomic_t g_hupflag = 0;	//Flag for sighup
volatile sig_atomic_t g_killflag = 0;	//Flag for sigterm

struct config g_configstruct;	//Structure for our config file
volatile int g_configStatus; 			//Determines if the config if current
int g_logfifofd;
char * g_logfifo = "/tmp/httpserver_logfifo";
pid_t g_mainpid, g_deathklock;
int   srvRun;
void     HUPhandler(int);
void     killhandler(int);
void  	 HUPhandler(int sig)
{
    //Reload The Config and restart the server
    g_hupflag=1;
    signal(SIGHUP, HUPhandler);
}

void  killhandler(int sig)
{
    g_killflag = 1;
}

//used to kill children when they dont respond
void  deathklockhandler(int sig)
{
	kill(g_deathklock,SIGKILL);
	printf("Proccess %i took to long to die and was killed\n",g_deathklock);
}


int main(int argc, char** argv)
{
    signal(SIGHUP, HUPhandler);
	 signal(SIGTERM, killhandler);
	 signal(SIGINT, killhandler);
	 signal(SIGQUIT, killhandler);
	 signal(SIGALRM, deathklockhandler);
    //signal(SIGINT, killhandler);
    printf("My process ID : %d\n", getpid());
	pid_t g_mainpid = getpid();
    //Load the config for the first time
    g_configstruct = loadConf();
    //Fork off the log processes
    pid_t logpid;

    //logfork

    logpid = fork();
    if(logpid < 0)
    {
        printf("Error");
        exit(1);
    }
    else if (logpid==0)
    {
        srvlog();
    }
	
    //makelogfifio
    mkfifo(g_logfifo, 0666);
    g_logfifofd = open(g_logfifo, O_WRONLY);
	
	if(!g_killflag) //prevent log from starting 
	{
    sendLog("status_ready");
	}
	
    //Main program loop
    while(1)
    {
        //printf("Waiting for log\n");
        fflush(stdout);
        //write(g_logfifofd, "status_ready", sizeof("status_ready"));
        if(g_killflag==1)
        {

            fflush(stdout);
            break;
		 }
		 web();
        fflush(stdout);
    }
    //The final countdown!
	g_deathklock = logpid;
	 alarm(5);
    sendLog("status_shutdown");
    waitpid(logpid,NULL,0);
	printf("logger shutdown!\n");
	 alarm(0);
    close(g_logfifofd);
    unlink(g_logfifo);
    printf("Application Terminated Successfully\n\n");
    fflush(stdout);
    return 0;
}

int web()
{
    sendLog("Web Server Started");
    //Load the config for our server
    srvRun = 0; //set the server run vallue to zero
    //inital setup
    int websockfs, newsockfd, portno, sockdead;
    uint clilen;
    char buffer[CHAR_MAX];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;

    //Make a new socket called websockfs
    websockfs = socket(AF_INET, SOCK_STREAM, 0);

    if (websockfs < 0)
    {
        perror("Error creating socket \n ");
        kill(g_mainpid,SIGTERM);
    }

    //Create socket strcuture
    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(g_configstruct.port);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(g_configstruct.interface);
    serv_addr.sin_port = htons(portno);
    //This option will let us reuse the port even if it is already in use
    //This allows us to restart the program with out running into issues
    int optval = 1;
    setsockopt(websockfs, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    //Create a new binding address
    if (bind(websockfs, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
		 perror("Error binding socket shutting down!");
        kill(g_mainpid,SIGTERM);
    }
	
    //STOP THE ZOMBIES, this causes the children to die and stay dead.
    struct sigaction sigchld_action = {
        .sa_handler = SIG_DFL,
        .sa_flags = SA_NOCLDWAIT
    };
    sigaction(SIGCHLD, &sigchld_action, NULL);

    // The listener will now fork off into a child procces
    pid_t listenpid;
    if(srvRun==0)
    {
        srvRun=1;
        //Begin fork
        fflush(stdout);
        listenpid = fork();
        if(listenpid < 0)
        {
            sendLog("Error forking listen child");
            exit(1);
        }

        else if (listenpid==0)
        {
            //Wait and listen for incoming socket connections
            sendLog("Listen");
            listen(websockfs,5);
            clilen = sizeof(cli_addr);

            while(1)
            {
                newsockfd = accept(websockfs, (struct sockaddr *)&cli_addr, &clilen);
				  //After we close the socket and we are sending this child a sigterm shutdown
				  if(g_killflag==1)
				  {
					wait(NULL);
					close(newsockfd);
					close(websockfs);
					sendLog("Web server shutdown");
					exit(0);
				  }
                if (newsockfd < 0)
                {
					  sendLog("Error reading from sock");
                    exit(0);
                }
                //Another fork, after the server forms a socket connection with the
                //client it will fork off into another child proccess
                pid_t clientpid;
                clientpid = fork();
                if(clientpid < 0)
                {
                    printf("Error");
                }

                else if (clientpid==0)
                {
                  prctl(PR_SET_PDEATHSIG, SIGKILL);
					memset(buffer,0,sizeof(buffer));
					n = read( newsockfd,buffer,sizeof(buffer));
					if (n < 0)
					{
						sendLog("Error reading from socket");
					}
					FILE *requestfile = NULL;
					char header[CHAR_MAX];
					//Parse the users request and return a FILE to send
					//and a HTTP response header
					parser(&header,&requestfile,&buffer);
					//We will write to the socket our header, and file
					writeSock(header,requestfile,&newsockfd);
					//shutdown(websockfs,0);
					//Clean up time
					shutdown(newsockfd,2);
					close(newsockfd);
					fclose(requestfile);
					//printf("socket closed\n");
					fflush(stdout);
					sendLog("Close Socket");
					exit(0);
                }
				
                else
                {	  //Close the parents copy of the newsockfd
                    close(newsockfd);
                }
            }
            //shutdown(websockfs,2);
        }
        printf("Going to main loop\n");
        fflush(stdout);
        //Parent Loop continues untill the sighup or sigkill
        while(1)
        {
            if(g_hupflag==1)
            {
                sendLog("Reloading Config");
                sendLog("Server stoping for reload");
				  g_deathklock = listenpid;
				  alarm(5);
				  kill(listenpid,SIGTERM);
				  shutdown(websockfs,2);
				  waitpid(listenpid,NULL,0);
				  alarm(0);
				  sendLog("reload_config");
				  g_configstruct = loadConf();
                 g_hupflag = 0;
                return 0;
            }
            sleep(1);
            if(g_killflag==1)
            {
				//Going through the steps to kill child
				  printf("Got a shutdown signal!\n");
				  sendLog("Shutting down server");
				  g_deathklock = listenpid;
				  alarm(5);
				  kill(listenpid,SIGTERM);
				  shutdown(websockfs,2);
				  waitpid(listenpid,NULL,0);
				  alarm(0);
                return 0;
            }

        }
        
    }
return 0;
}

//This code will take the socket FD and the returned FD
//From the parser so we can send it to the client
int writeSock(char header[CHAR_MAX],FILE *requestfp,int *clientsockfd)
{
    char buff[4096];
    long sent =0;
    int headersend = 0;
	int w = 1;
	int r = 1; 
	int len ;
	

    if(requestfp!=NULL)
    {
    fflush(stdout);
	//Get file size
	fseek(requestfp, 0, SEEK_END);
	len = (unsigned long)ftell(requestfp);
	rewind(requestfp);
    }
	

    while(sent!=1)
    {
        if (headersend == 0)
        {
            if(write(*clientsockfd,header,strlen(header)));
            headersend = 1;
        }
        else
        {
           // while((bytes!=0)&(requestfp!=NULL))
           while(len>0) 
			{
                memset(buff, 0, sizeof(buff));
                r = fread(buff,sizeof(buff),1,requestfp);
				  
				  //sleep(1);
                w = (write(*clientsockfd,buff,sizeof(buff)));
                  if (w < 0)
					{
						perror("Error writing to the socket ");
						return -1;
					}
					//printf("DEBUG %i %i\n",w,r);
					
					len-=w;
					//printf("DEBUG %i %i\n",w,len);
					
				 if(len<0)
				 {
					 w = (write(*clientsockfd,"/n",sizeof("/n")));
					 break;
				 }
					
					
					
			   }
			 sent=1;

        }
    }
	fflush(stdout);
	sleep(5); //quick fix on some computers
	
	fflush(*clientsockfd);
    return 0;
}

