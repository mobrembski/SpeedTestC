#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"

int sortServers (SPEEDTESTSERVER_T **srv1, SPEEDTESTSERVER_T **srv2)
{
	return((*srv1)->distance - (*srv2)->distance);
}

int main(int argc, char **argv)
{
    SPEEDTESTCONFIG_T *speedTestConfig=getConfig();
    printf("Your IP: %s And ISP: %s\n", speedTestConfig->ip, speedTestConfig->isp);
    printf("Lat: %f Lon: %f\n", speedTestConfig->lat, speedTestConfig->lon);
    int serverCount=0;
    int i;
    SPEEDTESTSERVER_T **serverList = getServers(&serverCount);

    printf("Grabbed %d servers\n",serverCount);
    for(i=0;i<serverCount;i++)
    	serverList[i]->distance = haversineDistance(speedTestConfig->lat,
    									speedTestConfig->lon,
    									serverList[i]->lat,
    									serverList[i]->lon);

    qsort (serverList,serverCount,sizeof(SPEEDTESTSERVER_T *),
                (int (*)(const void *,const void *)) sortServers);

    printf("Best Server URL: %s\n\t Name:%s Country: %s Sponsor: %s Dist: %ld\n",
	    serverList[0]->url,serverList[0]->name,serverList[0]->country,
	    serverList[0]->sponsor,serverList[0]->distance);
	return 0;
}

