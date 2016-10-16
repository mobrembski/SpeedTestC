#ifndef _SPEEDTEST_
#define _SPEEDTEST_
SPEEDTESTCONFIG_T *speedTestConfig;
SPEEDTESTSERVER_T **serverList;
int serverCount = 0;
int i, size, sockId;
char buffer[1500] = {0};
char *downloadUrl = NULL;
char *tmpUrl = NULL;
char *uploadUrl = NULL;
unsigned long totalTransfered = 1024 * 1024;
unsigned long totalToBeTransfered = 1024 * 1024;
struct timeval tval_start;
float elapsedSecs, speed;
int randomizeBestServers = 0;
int quietMode = 0;
#endif
