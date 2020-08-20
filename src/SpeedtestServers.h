#ifndef _SPEEDTEST_SERVERS_
#define _SPEEDTEST_SERVERS_

typedef struct speedtestServer
{
    char *url;
    float lat;
    float lon;
    char *name;
    char *country;
    char *sponsor;
    long distance;
    long latency;

} SPEEDTESTSERVER_T;
SPEEDTESTSERVER_T **getServers(int *serverCount, const char *infraUrl);
char *getServerDownloadUrl(char *serverUrl);
char *getLatencyUrl(char *serverUrl);
#endif
