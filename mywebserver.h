#ifndef MYWEBSERVER_H_
#define MYWEBSERVER_H_

#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "myfiles.h"
#include "myconfload.h"

struct config g_configstruct;
int g_logfifofd;
char * g_logfifo;
volatile int g_configStatus;
int main(int argc, char **argv);
int web();
int writeSock(char header[CHAR_MAX],FILE *requestfp,int *clientsockfd);
pid_t g_mainpid;

#endif