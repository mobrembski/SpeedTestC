/*
	Main program.

	Michał Obrembski (byku@byku.com.pl)
*/
#include "http.h"
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"
#include "Speedtest.h"
#include "SpeedtestLatencyTest.h"
#include "SpeedtestDownloadTest.h"
#include "SpeedtestUploadTest.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

// Global variables
SPEEDTESTSERVER_T **serverList;
int serverCount;
unsigned totalDownloadTestCount;
char *downloadUrl;
char *tmpUrl;
char *uploadUrl;
char *latencyUrl;
unsigned long totalTransfered;
unsigned long totalToBeTransfered;
int randomizeBestServers;
int lowestLatencyServers;

// strdup isnt a C99 function, so we need it to define itself
char *strdup(const char *str)
{
    int n = strlen(str) + 1;
    char *dup = malloc(n);
    if(dup)
    {
        strcpy(dup, str);
    }
    return dup;
}

int sortServersDistance(SPEEDTESTSERVER_T **srv1, SPEEDTESTSERVER_T **srv2)
{
    return((*srv1)->distance - (*srv2)->distance);
}

int sortServersLatency(SPEEDTESTSERVER_T **srv1, SPEEDTESTSERVER_T **srv2)
{
    return (*srv1)->latency - (*srv2)->latency;
}

float getElapsedTime(struct timeval tval_start) {
    struct timeval tval_end, tval_diff;
    gettimeofday(&tval_end, NULL);
    tval_diff.tv_sec = tval_end.tv_sec - tval_start.tv_sec;
    tval_diff.tv_usec = tval_end.tv_usec - tval_start.tv_usec;
    if(tval_diff.tv_usec < 0)
    {
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
            \t--server URL - use server URL, don't read config.\n\
            \t--upsize SIZE - use upload size of SIZE bytes.\n\
            \t--downtimes TIMES - how many times repeat download test.\n\
            \t\tSingle download test is downloading 30MB file.\n\
            \t--lowestlatency NUMBER - pick server with lowest latency\n\
            \t\tamong NUMBER closest\n\
            \t--randomize NUMBER - select random server among NUMBER best\n\
            \t\tCan be combined with --lowestlatency\n\
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
        if(strcmp("--lowestlatency", argv[i]) == 0)
        {
            lowestLatencyServers = strtoul(argv[i + 1], NULL, 10);
        }
    }
}

void freeMem()
{
    free(latencyUrl);
    free(downloadUrl);
    free(uploadUrl);
    free(serverList);
    free(speedTestConfig);
}

void getBestServer()
{
    int i;
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
    serverList = getServers(&serverCount, URL_PROTOCOL "://www.speedtest.net/speedtest-servers-static.php");
    if (serverCount == 0)
    {
        // Primary server is not responding. Let's give a try with secondary one.
        serverList = getServers(&serverCount, URL_PROTOCOL "://c.speedtest.net/speedtest-servers-static.php");
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
          (int (*)(const void *,const void *)) sortServersDistance);

    if (lowestLatencyServers != 0)
    {
        int debug = 0;
        if (lowestLatencyServers < 0)
        {
            lowestLatencyServers = -lowestLatencyServers;
            debug = 1;
        }
        printf("Testing closest %d servers for latency", lowestLatencyServers);
        fflush(stdout);
        // for LARGE numbers of servers could do this in parallel
        // (but best not to pester them, maybe limit max number??)
        for(i=0; i<lowestLatencyServers; i++)
        {
            latencyUrl = getLatencyUrl(serverList[i]->url);
            serverList[i]->latency = getLatency(latencyUrl);
            putchar('.');
            fflush(stdout);
        }
        putchar('\n');

        /* perform secondary sort on latency */
        qsort(serverList, lowestLatencyServers, sizeof(SPEEDTESTSERVER_T *),
              (int (*)(const void *,const void *)) sortServersLatency);

        if (debug)
        {
            for(i=0; i<lowestLatencyServers; i++)
            {
                printf("%-30.30s %-20.20s Dist: %3ld km Latency: %ld %s\n",
                       serverList[i]->sponsor, serverList[i]->name,
                       serverList[i]->distance, serverList[i]->latency, LATENCY_UNITS);
            }
        }

        if (randomizeBestServers >= lowestLatencyServers)
            randomizeBestServers = lowestLatencyServers / 2;
    }
    if (randomizeBestServers > 1)
    {
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

    if (lowestLatencyServers)        /* avoid getting latency twice! */
        printf("Latency: %ld %s\n",
               serverList[selectedServer]->latency, LATENCY_UNITS);

    for(i=0; i<serverCount; i++)
    {
        free(serverList[i]->url);
        free(serverList[i]->name);
        free(serverList[i]->sponsor);
        free(serverList[i]->country);
        free(serverList[i]);
    }
}

static void getUserDefinedServer()
{
    /* When user specify server URL, then we're not downloading config,
    so we need to specify thread count */
    speedTestConfig = malloc(sizeof(struct speedtestConfig));
    speedTestConfig->downloadThreadConfig.threadsCount = 4;
    speedTestConfig->uploadThreadConfig.threadsCount = 2;
    speedTestConfig->uploadThreadConfig.length = 3;

    uploadUrl = downloadUrl;
    tmpUrl = malloc(sizeof(char) * strlen(downloadUrl) + 1);
    strcpy(tmpUrl, downloadUrl);
    downloadUrl = getServerDownloadUrl(tmpUrl);
    free(tmpUrl);
}

int main(int argc, char **argv)
{
    totalTransfered = 1024 * 1024;
    totalToBeTransfered = 1024 * 1024;
    totalDownloadTestCount = 1;
    randomizeBestServers = 0;
    lowestLatencyServers = 0;
    speedTestConfig = NULL;

    parseCmdLine(argc, argv);

#ifdef OPENSSL
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
#endif

    if(downloadUrl == NULL)
    {
        getBestServer();
    }
    else
    {
        getUserDefinedServer();
    }

    if (lowestLatencyServers == 0)
    {
        latencyUrl = getLatencyUrl(uploadUrl);
        printf("Latency: %ld %s\n", getLatency(latencyUrl), LATENCY_UNITS);
    }

    testDownload(downloadUrl);
    testUpload(uploadUrl);

    freeMem();
    return 0;
}
