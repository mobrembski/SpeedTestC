#ifndef _SPEEDTEST_CONFIG_
#define _SPEEDTEST_CONFIG_
#include <math.h>
#define R 6371
#define TO_RAD (3.1415926536 / 180)

typedef struct speedtestConfig
{
	char ip[15];
	float lat;
	float lon;
	char isp[255];

} SPEEDTESTCONFIG_T;

static int verbose_mode=0;
const char *ConfigLineIdentitier;
const short ConfigParseFieldsNumber;
SPEEDTESTCONFIG_T *parseConfig(const char *configline);
SPEEDTESTCONFIG_T *getConfig();
int* func (int *func);
long haversineDistance(float lat1, float lon1, float lat2, float lon2);
#endif
