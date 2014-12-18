#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"

int main(int argc, char **argv)
{
    SPEEDTESTCONFIG_T *speedTestConfig=getConfig();
    printf("Your IP: %s And ISP: %s\n", speedTestConfig->ip, speedTestConfig->isp);
    printf("Lat: %f Lon: %f\n", speedTestConfig->lat, speedTestConfig->lon);
    int serverCount=0;
    int i;
    SPEEDTESTSERVER_T **serverList = getServers(&serverCount);

    printf("Grabbed %d servers\n",serverCount);
    for(i=0;i<serverCount;i++) {
    	serverList[i]->distance = haversineDistance(speedTestConfig->lat,
    									speedTestConfig->lon,
    									serverList[i]->lat,
    									serverList[i]->lon);
        printf("Server [%d] URL: %s\n\t Name:%s Country: %s Sponsor: %s Dist: %ld\n",
            i+1,serverList[i]->url,serverList[i]->name,serverList[i]->country,
            serverList[i]->sponsor,serverList[i]->distance);

    }
	return 0;
}

