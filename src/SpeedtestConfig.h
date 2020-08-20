#ifndef _SPEEDTEST_CONFIG_
#define _SPEEDTEST_CONFIG_
#include <math.h>
#define R 6371
#define PI 3.1415926536
#define TO_RAD (PI / 180)

#ifndef URL_PROTOCOL
#define URL_PROTOCOL "http"
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x) \
    (sizeof(_x) / sizeof(_x[0]))
#endif /* ARRAY_SIZE */

typedef struct ThreadConfig {
    int threadsCount; /* number of threads */
    int length; /* testlength? */
} THREADCONFIG_T;

typedef struct speedtestConfig
{
    char ip[16];
    float lat;
    float lon;
    char isp[255];
    THREADCONFIG_T uploadThreadConfig;
    THREADCONFIG_T downloadThreadConfig;
} SPEEDTESTCONFIG_T;

SPEEDTESTCONFIG_T *getConfig();
long haversineDistance(float lat1, float lon1, float lat2, float lon2);
#endif
