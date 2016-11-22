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

const char *ConfigLineIdentitier[] = {"<client", "<upload", "<server-config", "<download"};
const short ConfigParseFieldsNumber __attribute__((unused)) = 4;

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

SPEEDTESTCONFIG_T *parseConfig(const char *configline, int type, SPEEDTESTCONFIG_T **result_p)
{

	SPEEDTESTCONFIG_T *result = *result_p;

    if (!result) {
    	result = malloc(sizeof(struct speedtestConfig));
		if (!result) {
			/* Out of memory */
			return NULL;
		}
		*result_p = result;
    }

#define __GET_VALUE(_str, _key, _value) \
		do { \
			char *p, *e; \
			if ((p = strstr(_str, _key)) != NULL) { \
				p += sizeof(_key); /* With an extra " */ \
				e = strchr(p, '"'); \
				if (e) \
					strncpy(_value, p, e - p); \
			} \
		} while (0)

    switch (type) {
    	case 0: /* Client */
		{
			char lat[8];
			char lon[8];

		    if(sscanf(configline,"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%15[^\"]\"%*[^\"]\"%255[^\"]\"",
		            result->ip, lat, lon, result->isp)!=4)
		    {
		        fprintf(stderr,"Cannot parse all fields! Config line: %s", configline);
		        return NULL;
		    }
		    result->lat = strtof(lat, NULL);
	    	result->lon = strtof(lon, NULL);
    	}
	    break;

	    case 1: /* Upload */
		{
			char threads[8] = {"3"}, testlength[8] = {"9"};

			__GET_VALUE(configline, "testlength=", testlength);
			__GET_VALUE(configline, "threads=", threads);

			result->th_upload.count = atoi(threads);
			result->th_upload.length = atoi(testlength);
		}
	    break;

	    case 2: /* server-config */
		{
			char threadcount[8] = {"3"};

			__GET_VALUE(configline, "threadcount=", threadcount);

			result->th_download.count = atoi(threadcount);
	    }
	    break;

		case 3: /* Download */
		{
			char testlength[8] = {"9"};

			__GET_VALUE(configline, "testlength=", testlength);

			result->th_download.length = atoi(testlength);
		}
		break;

		default:
			break;
	}
#undef __GET_VALUE

    return result;
}

SPEEDTESTCONFIG_T *getConfig()
{
    SPEEDTESTCONFIG_T *result = NULL;
    char buffer[0xFFFF] = {0};
    int sockId = httpGetRequestSocket("http://www.speedtest.net/speedtest-config.php");

    if(sockId) {
    	int i, parsed = 0;
        long size;

        while((size = recvLine(sockId, buffer, sizeof(buffer))) > 0)
        {
            buffer[size + 1] = '\0';
            for (i = 0; i < ARRAY_SIZE(ConfigLineIdentitier); i++) {
            	if(strncmp(buffer, ConfigLineIdentitier[i], strlen(ConfigLineIdentitier[i])) == 0)
	            {
    	            parseConfig(buffer, i, &result);
    	            if (i == 0) {
    	            	/* The '<client' one is required */
	    	            parsed += ARRAY_SIZE(ConfigLineIdentitier);
	    	        }
	    	        parsed++;
        	        break;
	            }
	        }

	        if (parsed == ARRAY_SIZE(ConfigLineIdentitier) * 2) {
	        	break;
	        }
        }

        /* Cleanup */
		httpClose(sockId);
		if ((parsed < ARRAY_SIZE(ConfigLineIdentitier))
			&& result) {
			/* The required one is missing, we won't continue */
			free(result);
			result = NULL;
		}

		return result;
    }
    return NULL;
}
