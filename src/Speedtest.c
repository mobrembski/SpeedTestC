#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "http.h"
#include <time.h>
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


    char *downloadUrl = getServerDownloadUrl(serverList[0]);
    int sockId = httpGetRequestSocket(downloadUrl);
    char buffer[1500];
    int size=-1;
    long totalTransfered =0;
  	clock_t start = clock();
    while(size!=0)
    {
    	size = httpRecv(sockId,buffer,1500);
		totalTransfered+=size;
		if(verbose_mode)
		{
			float elapsedSecs = (float)((clock()-start)/(CLOCKS_PER_SEC / 1000))/100;
			float speed = (totalTransfered/elapsedSecs)/1024;
			printf("%ld %.2f %.2f\n",totalTransfered,elapsedSecs,speed);
		}
    }
    float elapsedSecs = (float)((clock()-start)/(CLOCKS_PER_SEC / 1000))/100;
    float speed = (totalTransfered/elapsedSecs)/1024;
    printf("Bytes %ld downloaded in %.2f seconds %.2f kB/s\n",totalTransfered,elapsedSecs,speed);
	return 0;
}

