#ifndef _SPEEDTEST_CONFIG_
#define _SPEEDTEST_CONFIG_
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "url.h"

typedef struct speedtestConfig
{
	char ip[15];
	char lat[8];
	char lon[8];
	char isp[255];

} SPEEDTESTCONFIG_T;

const char *ConfigLineIdentitier;
const short ConfigParseFieldsNumber;
SPEEDTESTCONFIG_T *parseConfig(const char *configline);
SPEEDTESTCONFIG_T *getConfig();
#endif
