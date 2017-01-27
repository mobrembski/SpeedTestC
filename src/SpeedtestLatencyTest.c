#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"
#include "Speedtest.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* Latency test is just testing time taken to download single latency.txt file
which contains 10 chars: "test=test\n"*/
#define LATENCY_SIZE 10

void testLatency(const char *url)
{
  char buffer[LATENCY_SIZE] = {0};
  int sockId, size = -1;
	struct timeval tval_start;

  gettimeofday(&tval_start, NULL);
	sockId = httpGetRequestSocket(url);

	if(sockId == 0)
	{
	  fprintf(stderr, "ERROR: Unable to open socket for testing latency!");
		exit(1);
	}

	while(size != 0)
	{
	  size = httpRecv(sockId, buffer, LATENCY_SIZE);
		if (size == -1)
    {
      fprintf(stderr, "ERROR: Cannot download latency!");
      exit(1);
    }
	}
	httpClose(sockId);
  printf("Latency: %d ms\n", (int)(getElapsedTime(tval_start) * 1000.0f) );
}
