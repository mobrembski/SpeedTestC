#ifndef _SPEEDTEST_CONFIG_
#define _SPEEDTEST_CONFIG_
#include <pthread.h>
#include <math.h>
#define R 6371
#define PI 3.1415926536
#define TO_RAD (PI / 180)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x) \
	(sizeof(_x) / sizeof(_x[0]))
#endif /* ARRAY_SIZE */

struct thread_param {
	pthread_t tid;
	char *url;
	unsigned int testCount;
	unsigned long transferedBytes;
	float elapsedSecs;
};

struct threadConfig {
	int count; /* number of threads */
	int length; /* testlength? */
};

/* TODO:  use the ignoreids? */
typedef struct speedtestConfig
{
	char ip[15];
	float lat;
	float lon;
	char isp[255];
	struct threadConfig th_upload;
	struct threadConfig th_download;
} SPEEDTESTCONFIG_T;

extern const char *ConfigLineIdentitier[];
extern const short ConfigParseFieldsNumber;
SPEEDTESTCONFIG_T *parseConfig(const char *configline, int type, SPEEDTESTCONFIG_T **result_p);
SPEEDTESTCONFIG_T *getConfig();
int* func (int *func);
long haversineDistance(float lat1, float lon1, float lat2, float lon2);
#endif
