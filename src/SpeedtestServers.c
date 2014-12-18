#include "SpeedtestServers.h"
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "url.h"


void parseServer(SPEEDTESTSERVER_T *result, const char *configline)
{
    // TODO: Remove Switch, replace it with something space-friendly
    //result = malloc(sizeof(SPEEDTESTSERVER_T));
    const char *tokens[8] = {"url=\"","lat=\"", "lon=\"", "name=\"", 
                                "country=\"", "cc=\"", "sponsor=\"", "id=\""};
    int i;
    for(i=1;i<8;i++)
    {
        char *first = strstr(configline, tokens[i-1]);
        char *second = strstr(configline, tokens[i]);
        if(first == NULL || second==NULL )
            return;
        int tokensize = strlen(tokens[i-1]);
        int size=second-first-1;
        char *substr = calloc(sizeof(char),size);
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
}

SPEEDTESTSERVER_T **getServers(int *serverCount)
{
	char buffer[1500] = {0};
    int sockId = httpGetRequestSocket("http://localhost/speedtest-servers-static.php");
    SPEEDTESTSERVER_T **list;
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
                    *serverCount=*serverCount+1;
                    list =(SPEEDTESTSERVER_T**)realloc(list,sizeof(SPEEDTESTSERVER_T**)*(*serverCount));
                    if(list==NULL) {
                        fprintf(stderr,"Unable to allocate memory for servers!\n");
                        exit(1);
                    }
                    list[*serverCount-1] =malloc(sizeof(SPEEDTESTSERVER_T));
                    if(list[*serverCount-1])
                    parseServer(list[*serverCount-1],buffer);
                }
			}
		}
		httpClose(sockId);
        return list;
	}
    return NULL;
}
