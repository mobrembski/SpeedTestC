#ifndef _SPEEDTEST_
#define _SPEEDTEST_

#include <sys/time.h>
#include <pthread.h>

#define SPEED_TEST_FILE_SIZE 31625365
#define BUFFER_SIZE 1500
extern SPEEDTESTCONFIG_T *speedTestConfig;
extern unsigned totalDownloadTestCount;
extern unsigned long totalTransfered;
extern unsigned long totalToBeTransfered;

typedef struct thread_args {
    pthread_t tid;
    char *url;
    unsigned int testCount;
    unsigned long transferedBytes;
    float elapsedSecs;
} THREADARGS_T;

float getElapsedTime(struct timeval tval_start);
char *strdup(const char *str);
#endif
