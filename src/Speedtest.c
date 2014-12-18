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
    struct speedtestConfig *speedTestConfig=getConfig();
    printf("Your IP: %s And ISP: %s\n", speedTestConfig->ip, speedTestConfig->isp);
    printf("Lat: %f Lon: %f\n", speedTestConfig->lat, speedTestConfig->lon);
    getServers();


	return 0;
}

