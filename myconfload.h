#ifndef MYCONFLOAD_H_   /* Include guard */
#define MYCONFLOAD_H_

int checkConf();

struct config
{
    char docroot[CHAR_MAX];
    char index[CHAR_MAX];
    char port[CHAR_MAX];
    char log[CHAR_MAX];
	char interface[CHAR_MAX];

};

struct config loadConf();

int parseConf(char(*rvar)[127],const char var[CHAR_MAX],FILE *FP);


#endif