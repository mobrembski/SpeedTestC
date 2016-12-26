/*
    Client configuration parsing functions.

    Michał Obrembski (byku@byku.com.pl)
*/
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
    float dx, dy, dz, a, b;
    lon1 -= lon2;
    lon1 *= TO_RAD, lat1 *= TO_RAD, lat2 *= TO_RAD;

    dz = sin(lat1) - sin(lat2);
    dx = cos(lon1) * cos(lat1) - cos(lat2);
    dy = sin(lon1) * cos(lat1);
    a = (dx * dx + dy * dy + dz * dz);
    b = sqrt(a) / 2;
#ifdef USE_ASIN
    return 2 * R * asin(b);
#else
    return 2 * R * atan2(b, sqrt(1 - b * b));
#endif
}

SPEEDTESTCONFIG_T *parseConfig(const char *configline)
{
    char lat[16] = {0};
    char lon[16] = {0};
    SPEEDTESTCONFIG_T *result = malloc(sizeof(struct speedtestConfig));
    if(sscanf(configline,"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%255[^\"]\"",
            result->ip, lat, lon, result->isp)!=ConfigParseFieldsNumber)
    {
        fprintf(stderr,"Cannot parse all fields! Config line: %s", configline);
        return NULL;
    }
    result->lat = strtof(lat, NULL);
    result->lon = strtof(lon, NULL);
    return result;
}

SPEEDTESTCONFIG_T *getConfig()
{
    SPEEDTESTCONFIG_T *result;
    char buffer[0xFFFF] = {0};
    int sockId = httpGetRequestSocket("http://www.speedtest.net/speedtest-config.php");
    if(sockId) {
        long size;
        while((size = recvLine(sockId, buffer, sizeof(buffer))) > 0)
        {
            buffer[size + 1] = '\0';
            if(strncmp(buffer, ConfigLineIdentitier, strlen(ConfigLineIdentitier)) == 0)
            {
                result = parseConfig(buffer);
                httpClose(sockId);
                return result;
            }
        }
    }
    return NULL;
}
