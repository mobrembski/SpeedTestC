#ifndef _SPEEDTEST_
#define _SPEEDTEST_

#include <time.h>
#include <pthread.h>

#define SPEED_TEST_FILE_SIZE 31625365
#define BUFFER_SIZE 1500
SPEEDTESTCONFIG_T *speedTestConfig;
SPEEDTESTSERVER_T **serverList;
int serverCount;
int i, size, sockId;
unsigned totalDownloadTestCount;
char buffer[BUFFER_SIZE];
char *downloadUrl;
char *tmpUrl;
char *uploadUrl;
char *latencyUrl;
unsigned long totalTransfered;
unsigned long totalToBeTransfered;
struct timeval tval_start;
float elapsedSecs;
int randomizeBestServers;
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
