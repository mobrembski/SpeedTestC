#ifndef HTTPH
#define HTTPH

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int httpPut(char* pAddress, int pPort, char* pRequest, unsigned long contentSize);
int httpGet(char* pAddress, int pPort, char* pRequest);
int httpRecv(int pSockId, char* pOut, int pOutSize);
int httpSend(int pSockId, char* pOut, int pOutSize);
int httpLastError();
void httpClose(int pSockId);
int httpGetRequestSocket(const char *urlToDownload);
int httpPutRequestSocket(const char *urlToUpload, unsigned long contentSize);
int recvLine(int pSockId, char* pOut, int pOutSize);

#endif // HTTPH
