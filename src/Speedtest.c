/*
	Main program.

	Michał Obrembski (byku@byku.com.pl)
*/
#define __USE_POSIX 1
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <pthread.h> /* For pthread_create() */
#include <unistd.h>

#include "http.h"
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"
#include "Speedtest.h"

int sortServers(SPEEDTESTSERVER_T **srv1, SPEEDTESTSERVER_T **srv2)
{
    return((*srv1)->distance - (*srv2)->distance);
}

float getElapsedTime(struct timeval tval_start) {
    struct timeval tval_end, tval_diff;
    gettimeofday(&tval_end, NULL);
    tval_diff.tv_sec = tval_end.tv_sec - tval_start.tv_sec;
    tval_diff.tv_usec = tval_end.tv_usec - tval_start.tv_usec;
    if(tval_diff.tv_usec < 0) {
        --tval_diff.tv_sec;
        tval_diff.tv_usec += 1000000;
    }
    return (float)tval_diff.tv_sec + (float)tval_diff.tv_usec / 1000000;
}

void parseCmdLine(int argc, char **argv) {
    int i;
    for(i=1; i<argc; i++)
    {
        if(strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0)
        {
            printf("Usage (options are case sensitive):\n\
            \t--help - Show this help.\n\
            \t--server URL - use server URL, don'read config.\n\
            \t--upsize SIZE - use upload size of SIZE bytes.\n\
            \t--downtimes TIMES - how many times repeat download test.\n\
            \tSingle download test is downloading 30MB file.\n\
            \t--randomize NUMBER - randomize server usage for NUMBER of best servers\n\
            \nDefault action: Get server from Speedtest.NET infrastructure\n\
            and test download with 30MB download size and 1MB upload size.\n");
            exit(1);
        }
        if(strcmp("--server", argv[i]) == 0)
        {
            downloadUrl = malloc(sizeof(char) * strlen(argv[i+1]) + 1);
            strcpy(downloadUrl, argv[i + 1]);
        }
        if(strcmp("--upsize", argv[i]) == 0)
        {
            totalToBeTransfered = strtoul(argv[i + 1], NULL, 10);
        }
        if(strcmp("--downtimes", argv[i]) == 0)
        {
            totalDownloadTestCount = strtoul(argv[i + 1], NULL, 10);
        }
        if(strcmp("--randomize", argv[i]) == 0)
        {
            randomizeBestServers = strtoul(argv[i + 1], NULL, 10);
        }
    }
}

void freeMem()
{
    free(downloadUrl);
    free(uploadUrl);
    free(serverList);
    free(speedTestConfig);
}

void getBestServer()
{
    size_t selectedServer = 0;
    speedTestConfig = getConfig();
    if (speedTestConfig == NULL)
    {
        printf("Cannot download speedtest.net configuration. Something is wrong...\n");
        freeMem();
        exit(1);
    }
    printf("Your IP: %s And ISP: %s\n",
                speedTestConfig->ip, speedTestConfig->isp);
    printf("Lat: %f Lon: %f\n", speedTestConfig->lat, speedTestConfig->lon);
    serverList = getServers(&serverCount, "http://www.speedtest.net/speedtest-servers-static.php");
    if (serverCount == 0)
    {
        // Primary server is not responding. Let's give a try with secondary one.
        serverList = getServers(&serverCount, "http://c.speedtest.net/speedtest-servers-static.php");
    }
    printf("Grabbed %d servers\n", serverCount);
    if (serverCount == 0)
    {
        printf("Cannot download any speedtest.net server. Something is wrong...\n");
        freeMem();
        exit(1);
    }
    for(i=0; i<serverCount; i++)
        serverList[i]->distance = haversineDistance(speedTestConfig->lat,
            speedTestConfig->lon,
            serverList[i]->lat,
            serverList[i]->lon);

    qsort(serverList, serverCount, sizeof(SPEEDTESTSERVER_T *),
                (int (*)(const void *,const void *)) sortServers);

    if (randomizeBestServers != 0) {
        printf("Randomizing selection of %d best servers...\n", randomizeBestServers);
        srand(time(NULL));
        selectedServer = rand() % randomizeBestServers;
    }

    printf("Best Server URL: %s\n\t Name: %s Country: %s Sponsor: %s Dist: %ld km\n",
        serverList[selectedServer]->url, serverList[selectedServer]->name, serverList[selectedServer]->country,
        serverList[selectedServer]->sponsor, serverList[selectedServer]->distance);
    downloadUrl = getServerDownloadUrl(serverList[selectedServer]->url);
    uploadUrl = malloc(sizeof(char) * strlen(serverList[selectedServer]->url) + 1);
    strcpy(uploadUrl, serverList[selectedServer]->url);

    for(i=0; i<serverCount; i++){
        free(serverList[i]->url);
        free(serverList[i]->name);
        free(serverList[i]->sponsor);
        free(serverList[i]->country);
        free(serverList[i]);
    }
}

static void *__testDownload(void *arg)
{
	/* Testing download... */
	struct thread_param *p = (struct thread_param *)arg;
	int testNum;
	char buffer[BUFFER_SIZE] = {0};
	struct timeval tval_start;
	float speed = 0;

	gettimeofday(&tval_start, NULL);
	for (testNum = 0; testNum < p->testCount; testNum++) {
		int size = -1;
		int sockId = httpGetRequestSocket(p->url);

		if(sockId == 0)
		{
		    printf("Unable to open socket for Download!");
			pthread_exit(NULL);
		}

		while(size != 0)
		{
		    size = httpRecv(sockId, buffer, BUFFER_SIZE);
			if (size != -1)
			    p->transferedBytes += size;
		}
		httpClose(sockId);
	}
	p->elapsedSecs = getElapsedTime(tval_start);
	speed = (p->transferedBytes / p->elapsedSecs) / 1024;
	printf("Bytes %lu downloaded in %.2f seconds %.2f kB/s (%.2f kb/s)\n",
	    p->transferedBytes, p->elapsedSecs, speed, speed * 8);

	return NULL;
}

void testDownload(const char *url)
{
	struct thread_param *param = (struct thread_param *) calloc(speedTestConfig->th_download.count,
		sizeof(struct thread_param));
	int i;

	for (i = 0; i < speedTestConfig->th_download.count; i++) {
		/* Initializing some parameters */
		param[i].testCount = totalDownloadTestCount / speedTestConfig->th_download.count;
		if (param[i].testCount == 0) {
			/* At least one test should be run */
			param[i].testCount = 1;
		}
		param[i].url = strdup(url);
		if (param[i].url) {
			pthread_create(&param[i].tid, NULL, &__testDownload, &param[i]);
		}
	}
	/* Wait for all threads */
	for (i = 0; i < speedTestConfig->th_download.count; i++) {
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
	printf("[ SUM ] Bytes %lu downloaded with a speed %.2f kB/s (%.2f Mbit/s)\n",
	   totalTransfered, speed, speed * 8 / 1024);

}

static void __appendTimestamp(const char *url, char *buff, int buff_len)
{
	char delim = '?';
	char *p = strchr(url, '?');

	if (p)
		delim = '&';
	snprintf(buff, buff_len, "%s%cx=%llu", url, delim, (unsigned long long)time(NULL));
}

static int __readUploadResp(int sockId)
{
	char ch;
	int ret;

	while (1) {
		ret = read(sockId, &ch, 1);
		if (ret == -1) {
			perror("read()");
			break;
		} else if (!ret) {
			printf("Ending connection...");
			break;
		}

		if (ch == '\n' || ch == '\r')
			break;
		/* printf("%c", ch); */
	}

	return 0;
}
static void *__testUpload(void *arg)
{
    /* Testing upload... */
	struct thread_param *p = (struct thread_param *)arg;
	char t[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned int seed = (unsigned int)time(NULL);
	char buffer[BUFFER_SIZE] = {0};
	int i, size, sockId;
	float speed;
	struct timeval tval_start;
	unsigned long totalTransfered = 0;
	char uploadUrl[1024];

	/* Build the random buffer */
	for(i=0; i < BUFFER_SIZE; i++)
		    buffer[i] = t[rand_r(&seed) % ARRAY_SIZE(t)];

	gettimeofday(&tval_start, NULL);
	for (i = 0; i < p->testCount; i++) {
		__appendTimestamp(p->url, uploadUrl, sizeof(uploadUrl));
		/* FIXME: totalToBeTransfered should be readonly while the upload thread is running */
	    totalTransfered = totalToBeTransfered;
	    sockId = httpPutRequestSocket(uploadUrl, totalToBeTransfered);
	    if(sockId == 0)
	    {
	        printf("Unable to open socket for Upload!");
	        pthread_exit(NULL);
	    }

	    while(totalTransfered != 0)
	    {
			if (totalTransfered > BUFFER_SIZE) {
		        size = httpSend(sockId, buffer, BUFFER_SIZE);
			} else {
				buffer[totalTransfered - 1] = '\n'; /* Indicate terminated */
				size = httpSend(sockId, buffer, totalTransfered);
			}
	        totalTransfered -= size;
	    }
		/* XXX: need to check the real response? */
		__readUploadResp(sockId);
		p->transferedBytes += totalToBeTransfered;
		/* Cleanup */
		httpClose(sockId);
	}
    p->elapsedSecs = getElapsedTime(tval_start);
    speed = (p->transferedBytes / p->elapsedSecs) / 1024;

    printf("Bytes %lu uploaded in %.2f seconds %.2f kB/s (%.2f Mbit/s)\n",
        p->transferedBytes, p->elapsedSecs, speed, speed * 8 / 1024);

	return NULL;
}

void testUpload(const char *url)
{
	struct thread_param *param = (struct thread_param *) calloc(speedTestConfig->th_upload.count,
		sizeof(struct thread_param));
	int i;



	for (i = 0; i < speedTestConfig->th_upload.count; i++) {
		/* Initializing some parameters */
		param[i].testCount =  speedTestConfig->th_upload.length;
		if (param[i].testCount == 0) {
			/* At least three test should be run */
			param[i].testCount = 3;
		}
		param[i].url = strdup(url);
		if (param[i].url) {
			pthread_create(&param[i].tid, NULL, &__testUpload, &param[i]);
		}
	}

	/* Refresh */
	totalTransfered = 0;
	speed = 0;

	/* Wait for all threads */
	for (i = 0; i < speedTestConfig->th_upload.count; i++) {
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
	printf("[SUM] Bytes %lu uploaded with a speed %.2f kB/s (%.2f Mbit/s)\n",
        totalTransfered, speed, speed * 8 / 1024);
}

int main(int argc, char **argv)
{
    speedTestConfig = NULL;
    parseCmdLine(argc, argv);

    if(downloadUrl == NULL)
    {
        getBestServer();
    }
    else
    {
        uploadUrl = downloadUrl;
        tmpUrl = malloc(sizeof(char) * strlen(downloadUrl) + 1);
        strcpy(tmpUrl, downloadUrl);
        downloadUrl = getServerDownloadUrl(tmpUrl);
        free(tmpUrl);
    }

    testDownload(downloadUrl);
    testUpload(uploadUrl);

    freeMem();
    return 0;
}
