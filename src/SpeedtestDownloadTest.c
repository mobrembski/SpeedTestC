#include "SpeedtestDownloadTest.h"
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"
#include "Speedtest.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

static void *__downloadThread(void *arg)
{
  THREADARGS_T *threadConfig = (THREADARGS_T*)arg;
	int testNum;
	char buffer[BUFFER_SIZE] = {0};
	struct timeval tval_start;

	gettimeofday(&tval_start, NULL);
	for (testNum = 0; testNum < threadConfig->testCount; testNum++)
  {
		int size = -1;
		int sockId = httpGetRequestSocket(threadConfig->url);

		if(sockId == 0)
		{
		  fprintf(stderr, "Unable to open socket for Download!");
			pthread_exit(NULL);
		}

		while(size != 0)
		{
		  size = httpRecv(sockId, buffer, BUFFER_SIZE);
			if (size != -1)
      {
        threadConfig->transferedBytes += size;
      }
		}
		httpClose(sockId);
	}
	threadConfig->elapsedSecs = getElapsedTime(tval_start);

	return NULL;
}

void testDownload(const char *url)
{
  size_t numOfThreads = speedTestConfig->downloadThreadConfig.threadsCount;
  THREADARGS_T *param = (THREADARGS_T *)calloc(numOfThreads, sizeof(THREADARGS_T));
  int i;
  float speed = 0;

  /* Initialize and start threads */
  for (i = 0; i < numOfThreads; i++) {
    param[i].testCount = totalDownloadTestCount / numOfThreads;
    if (param[i].testCount == 0) {
      /* At least one test should be run */
      param[i].testCount = 1;
    }
    param[i].url = strdup(url);
    if (param[i].url) {
      pthread_create(&param[i].tid, NULL, &__downloadThread, &param[i]);
    }
  }
  /* Wait for all threads */
  for (i = 0; i < numOfThreads; i++) {
    pthread_join(param[i].tid, NULL);
    if (param[i].transferedBytes) {
      /* There's no reason that we transfered nothing except error occured */
      totalTransfered += param[i].transferedBytes;
      speed += (param[i].transferedBytes / param[i].elapsedSecs) / 1024;
    }
    /* Cleanup */
    free(param[i].url);
  }
  free(param);

  /* Report */
  printf("Bytes %lu downloaded with a speed %.2f kB/s (%.2f Mbit/s)\n",
     totalTransfered, speed, speed * 8 / 1024);
}
