#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "http.h"
#include "url.h"

int sock;

char *ip;
char *get;

char *host;
char *page;

int main(int argc, char **argv)
{
	char buffer[0xFFFF] = {0};
    char address[1024];
    char request[1024];
    URLPARTS_T url;

    memset(&url, 0, sizeof(url));
    url.address = address;
    url.request = request;
    url.addressLen = sizeof(address);
    url.requestLen = sizeof(request);

    breakUrl("http://www.speedtest.net/speedtest-config.php", &url);

    int sockId = httpGet(address, url.port, request);
    if(sockId)
    {
        FILE* file = fopen("config.xml", "wb");
        int size;
        while((size = httpRecv(sockId, buffer, sizeof(buffer))) > 0) {
            printf("Got %d bytes %s \n",size,buffer);
            fwrite(buffer, size, 1, file);
        }
        printf("Buffer: %s",buffer);
            
        fclose(file);
        httpClose(sockId);
    } else printf("Http error: %i\n", httpLastError());

    return 0;
}

