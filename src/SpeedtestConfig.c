#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SpeedtestConfig.h"
#include "http.h"
#include "url.h"

const char *ConfigLineIdentitier = "<client";
const short ConfigParseFieldsNumber = 4;

long haversineDistance(float lat1, float lon1, float lat2, float lon2)
{
	float dx, dy, dz;
	lon1 -= lon2;
	lon1 *= TO_RAD, lat1 *= TO_RAD, lat2 *= TO_RAD;
 
	dz = sin(lat1) - sin(lat2);
	dx = cos(lon1) * cos(lat1) - cos(lat2);
	dy = sin(lon1) * cos(lat1);
	return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

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
	result->lat = strtof(lat,NULL);
	result->lon = strtof(lon,NULL);
	return result;
}

SPEEDTESTCONFIG_T *getConfig()
{
	SPEEDTESTCONFIG_T *result;
	char buffer[0xFFFF] = {0};
	int sockId = httpGetRequestSocket("http://localhost/speedtest-config.php");
	if(sockId) {
		long size;
		while((size = recvLine(sockId, buffer, sizeof(buffer))) > 0)
		{
			buffer[size+1] = '\0';
			if(strncmp(buffer,ConfigLineIdentitier,strlen(ConfigLineIdentitier))==0)
			{
				printf("Config line: %s\n",buffer);
                result = parseConfig(buffer);
                httpClose(sockId);
                return result;
			}
		}
	}
	return NULL;
}
