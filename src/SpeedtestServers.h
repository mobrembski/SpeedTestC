#ifndef _SPEEDTEST_SERVERS_
#define _SPEEDTEST_SERVERS_


typedef struct speedtestServer
{
	char *url;
	char *lat;
	char *lon;
	char *name;
	char *country;
	char *sponsor;

} SPEEDTESTSERVER_T;
void getServers();
#endif
