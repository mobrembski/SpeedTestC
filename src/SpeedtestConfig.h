#ifndef _SPEEDTEST_CONFIG_
#define _SPEEDTEST_CONFIG_


typedef struct speedtestConfig
{
	char ip[15];
	float lat;
	float lon;
	char isp[255];

} SPEEDTESTCONFIG_T;

const char *ConfigLineIdentitier;
const short ConfigParseFieldsNumber;
SPEEDTESTCONFIG_T *parseConfig(const char *configline);
SPEEDTESTCONFIG_T *getConfig();
int* func (int *func);
#endif
