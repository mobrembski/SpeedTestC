
#ifndef HTTPH
#define HTTPH

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int httpGet(char* pAddress, int pPort, char* pRequest);
int httpRecv(int pSockId, char* pOut, int pOutSize);
int httpLastError();
void httpClose(int pSockId);
int httpGetRequestSocket(const char *urlToDownload);

#endif // HTTPH
