/*
    URL functions.

    Luke Graham (39ster@gmail.com)
    Michał Obrembski (byku@byku.com.pl)
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "url.h"

/*Breaks apart a url into a port, address and request*/
void breakUrl(const char* pUrl, URLPARTS_T* pUrlStruct)
{
    char url[2048];
    char *ptr2, *ptr1 = url;
    strncpy(url, pUrl, sizeof(url));
    pUrlStruct->port = 80;
    pUrlStruct->ssl = 0;
/*find protocol (like http:// or ftp://)*/
    ptr2 = strstr(ptr1, "://");
    if(ptr2 != 0)
    {
        *ptr2 = 0;
        if(pUrlStruct->protocolLen > 0)
            strncpy(pUrlStruct->protocol, ptr1, pUrlStruct->protocolLen);

        if(strcmp(ptr1, "ftp") == 0)
            pUrlStruct->port = 21;
        else if(strcmp(ptr1, "https") == 0) {
            pUrlStruct->port = 443;
        pUrlStruct->ssl = 1;
    }
        ptr1 = ptr2 + 3;
    } else if(pUrlStruct->protocolLen > 0)
    *pUrlStruct->protocol = 0;

/*find request (the part starting at "/" after the address and port)*/
    ptr2 = strchr(ptr1, '/');
    if(ptr2 != 0)
    {
        if(pUrlStruct->requestLen > 0)
            strncpy(pUrlStruct->request, ptr2, pUrlStruct->requestLen);

        *ptr2 = 0;
    } else if(pUrlStruct->requestLen > 1)
    strncpy(pUrlStruct->request, "/", pUrlStruct->requestLen);

/*find port (the part after ":")*/
    ptr2 = strchr(ptr1, ':');
    if(ptr2 != 0)
    {
        pUrlStruct->port = atoi(ptr2+1);
        *ptr2 = 0;
    }

/*whats left should only be the address*/
    if(pUrlStruct->addressLen > 0)
        strncpy(pUrlStruct->address, ptr1, pUrlStruct->addressLen);
}

int hexToAscii(char pHex)
{
    if(pHex >= '0' && pHex <= '9')
        return pHex - '0';
    if(pHex >= 'A' && pHex <= 'F')
        return pHex - 'A' + 10;
    if(pHex >= 'a' && pHex <= 'f')
        return pHex - 'a' + 10;
    return 0;
}
/*Decodes the hex parts of the request (e.g %20 is ' ')*/
void decodeRequest(const char* pRequest, char* pOut, int pOutSize)
{
    char* ptr1 = (char*)pRequest;
    int i;
    memset(pOut, 0, pOutSize);
    for(i = 0; i < pOutSize-1; i++)
    {
        if(*ptr1 == 0)
            break;

        switch(*ptr1)
        {
            case '%':
            pOut[i] = hexToAscii(*++ptr1) << 4;
            pOut[i] += hexToAscii(*++ptr1);
            break;

            default:
            pOut[i] = *ptr1;
            break;
        }
        ptr1++;
    }
}
