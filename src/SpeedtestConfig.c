#include "SpeedtestConfig.h"

const char *ConfigLineIdentitier = "<client";

SPEEDTESTCONFIG_T *parseConfig(const char *configline) {
	SPEEDTESTCONFIG_T *result = malloc(sizeof(struct speedtestConfig));
	sscanf(configline,"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%255[^\"]\"",
			&result->ip, &result->lat, &result->lon, &result->isp);
	return result;
}

SPEEDTESTCONFIG_T *getConfig()
{
	char buffer[0xFFFF] = {0};
	int sockId = httpGetRequestSocket("http://www.speedtest.net/speedtest-config.php");
	if(sockId) {
		long size;
		while((size = recvLine(sockId, buffer, sizeof(buffer))) > 0) {
			buffer[size+1]='\0';
			if(strncmp(buffer,ConfigLineIdentitier,strlen(ConfigLineIdentitier))==0) {
				printf("Config line: %s\n",buffer);
				SPEEDTESTCONFIG_T *config = parseConfig(buffer);
				httpClose(sockId);
				return config;
			}
		}
		fprintf("Cannot find config line!\n",stderr);
		httpClose(sockId);
	} else return NULL;
}
