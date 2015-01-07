#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "http.h"
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"

int sortServers (SPEEDTESTSERVER_T **srv1, SPEEDTESTSERVER_T **srv2)
{
	return((*srv1)->distance - (*srv2)->distance);
}

float getElapsedTime(struct timeval tval_start) {
    struct timeval tval_end, tval_diff;
    gettimeofday(&tval_end, NULL);
    tval_diff.tv_sec = tval_end.tv_sec - tval_start.tv_sec;
    tval_diff.tv_usec = tval_end.tv_usec - tval_start.tv_usec;
    if(tval_diff.tv_usec <0) {
        --tval_diff.tv_sec;
        tval_diff.tv_usec += 1000000;
    }
    return (float)tval_diff.tv_sec + (float)tval_diff.tv_usec/1000000;
}

int main(int argc, char **argv)
{
    SPEEDTESTCONFIG_T *speedTestConfig;
    SPEEDTESTSERVER_T **serverList;
    int serverCount=0;
    int i, size, sockId;
    char *downloadUrl, buffer[1500] = {0};
    long long totalTransfered =1024*1024*10;
    long long totalToBeTransfered =1024*1024*10;
    struct timeval tval_start;
    float elapsedSecs, speed;

    serverList = getServers(&serverCount);
    speedTestConfig = getConfig();
    printf("Grabbed %d servers\n",serverCount);
    printf("Your IP: %s And ISP: %s\n",
        speedTestConfig->ip, speedTestConfig->isp);
    printf("Lat: %f Lon: %f\n", speedTestConfig->lat, speedTestConfig->lon);
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

    // Testing download...
    downloadUrl = getServerDownloadUrl(serverList[0]);
    sockId = httpGetRequestSocket(downloadUrl);
    size=-1;
    totalTransfered =0;
    gettimeofday(&tval_start, NULL);
    while(size!=0)
    {
    	size = httpRecv(sockId,buffer,1500);
		totalTransfered+=size;
    }
    elapsedSecs = getElapsedTime(tval_start);
    speed = (totalTransfered/elapsedSecs)/1024;
    httpClose(sockId);
    printf("Bytes %ld downloaded in %.2f seconds %.2f kB/s\n",
        totalTransfered,elapsedSecs,speed);

    // Testing upload...
    sockId = httpPutRequestSocket(serverList[0]->url,size);
    gettimeofday(&tval_start, NULL);
    while(totalTransfered>0)
    {
        for(i=0;i<1500;i++)
            buffer[i]=(char)i;
        size = httpSend(sockId,buffer,1500);
        totalTransfered-=size;
    }
    elapsedSecs = getElapsedTime(tval_start);
    speed = (totalToBeTransfered/elapsedSecs)/1024;
    httpClose(sockId);
    printf("Bytes %ld uploaded in %.2f seconds %.2f kB/s\n",
        totalToBeTransfered,elapsedSecs,speed);

    for(i=0;i<serverCount;i++){
    	free(serverList[i]->url);
    	free(serverList[i]->name);
    	free(serverList[i]->sponsor);
    	free(serverList[i]->country);
    	free(serverList[i]);
    }
    free(downloadUrl);
    free(serverList);
    free(speedTestConfig);
	return 0;
}

