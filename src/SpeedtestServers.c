#include "SpeedtestServers.h"
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "url.h"


SPEEDTESTSERVER_T *parseServer(const char *configline)
{
    // TODO: Remove Switch, replace it with something space-friendly
    SPEEDTESTSERVER_T *result = malloc(sizeof(struct speedtestServer));
    const char *tokens[8] = {"url=\"","lat=\"", "lon=\"", "name=\"", 
                                "country=\"", "cc=\"", "sponsor=\"", "id=\""};
    int i;
    for(i=1;i<8;i++)
    {
        char *first = strstr(configline, tokens[i-1]);
        char *second = strstr(configline, tokens[i]);
        if(first == NULL || second==NULL )
            return NULL;
        int tokensize = strlen(tokens[i-1]);
        int size=second-first-1;
        char *substr = malloc(sizeof(char)*size);
        strncpy(substr, first+tokensize, size-tokensize-1);
        substr[size-tokensize]='\0';
        switch(i)
        {
            case 1:
                result->url=substr;
                break;
            case 2:
                result->lat=strtof(substr,NULL);
                break;
            case 3:
                result->lon=strtof(substr,NULL);
                break;
            case 4:
                result->name=substr;
                break;
            case 5:
                result->country=substr;
                break;
            case 7:
                result->sponsor=substr;
                break;
        }
    }
    return result;
}

void getServers()
{
	char buffer[1500] = {0};
    int sockId = httpGetRequestSocket("http://www.speedtest.net/speedtest-servers-static.php");
	if(sockId) {
		long size;
        while((size = recvLine(sockId, buffer, sizeof(buffer))) > 0)
		{
			buffer[size+1]='\0';
            if(strlen(buffer) > 25)
			{
                //Ommiting XML invocation...
                if (strstr(buffer,"<?xml"))
                    continue;
                //TODO: Fix case when server entry doesn't fit in TCP packet
                if(buffer[0]=='<' && buffer[size-1]=='>')
                {
                    SPEEDTESTSERVER_T *server = parseServer(buffer);
                    if(server)
                        printf("Server URL: %s lat: %f lon: %f name: %s country: %s sponsor: %s\n",
                            server->url,server->lat,server->lon,server->name,
                            server->country,server->sponsor);
                }
                //if(i++ == 10)
                    //httpClose(sockId);
			}
		}
		httpClose(sockId);
	}
}
