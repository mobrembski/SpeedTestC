#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"
#include "SpeedtestLatencyTest.h"
#include "Speedtest.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* Latency test is just testing time taken to download single latency.txt file
which contains 10 chars: "test=test\n"*/
#define LATENCY_SIZE 10

long getLatency(const char *url)
{
  char buffer[LATENCY_SIZE] = {0};
  sock_t sockId;
  struct timeval tval_start;

  gettimeofday(&tval_start, NULL);
  sockId = httpGetRequestSocket(url);
  if(sockId == 0)
    return LATENCY_CONNECT_ERROR;

  for (;;) {
    int size = httpRecv(sockId, buffer, LATENCY_SIZE);
    if (size == -1) {
      httpClose(sockId);
      return LATENCY_DATA_ERROR;
    }
    if (size == 0)
      break;
  }

  httpClose(sockId);
  return (long)(getElapsedTime(tval_start) * LATENCY_UNITS_PER_SECOND); /* ms */
}
