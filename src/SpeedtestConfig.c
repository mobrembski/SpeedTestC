#include "SpeedtestConfig.h"
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "url.h"

const char *ConfigLineIdentitier = "<client";
const short ConfigParseFieldsNumber = 4;

SPEEDTESTCONFIG_T *parseConfig(const char *configline)
{
	char lat[8];
	char lon[8];
    SPEEDTESTCONFIG_T *result = malloc(sizeof(struct speedtestConfig));
	if(sscanf(configline,"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%255[^\"]\"",
			result->ip, lat, lon, result->isp)!=ConfigParseFieldsNumber)
	{
		fprintf(stderr,"Cannot parse all fields! Config line: %s",configline);
		return NULL;
	}
	result->lat=strtof(lat,NULL);
	result->lon=strtof(lon,NULL);
	return result;
}

SPEEDTESTCONFIG_T *getConfig()
{
	char buffer[0xFFFF] = {0};
	int sockId = httpGetRequestSocket("http://www.speedtest.net/speedtest-config.php");
	if(sockId) {
		long size;
		while((size = recvLine(sockId, buffer, sizeof(buffer))) > 0)
		{
			buffer[size+1]='\0';
			if(strncmp(buffer,ConfigLineIdentitier,strlen(ConfigLineIdentitier))==0)
			{
				printf("Config line: %s\n",buffer);
                SPEEDTESTCONFIG_T *result = parseConfig(buffer);
                httpClose(sockId);
                return result;
			}
		}
	}
	return NULL;
}
