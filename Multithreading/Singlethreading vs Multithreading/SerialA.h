#ifndef PROJ4_SERIALA_H
#define PROJ4_SERIALA_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <sys/resource.h>
void Input(void);

struct Statistic {
    int Bad;
    int Dir;
    int Reg;
    int Spe;
    long RegFB;
    int TextF;
    long TextFB;

};

void getStats(struct Statistic* stats, char* FileName);
void printer(struct Statistic* stat);  // Print stats
int getModeStats(struct Statistic* stat, mode_t m);
int isText(char* buffer);
void *reader(void* args);
void timesUsed(struct rusage* usage, int threads);

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; //Global variable

#endif //PROJ4_SERIALA_H