#ifndef HTTPH
#define HTTPH

#ifdef OPENSSL
#include <openssl/ssl.h>
#include <openssl/bio.h>

typedef BIO *sock_t;
#define BAD_SOCKID NULL
#else
typedef int sock_t;
#define BAD_SOCKID 0  /* should use -1!! */
#endif

sock_t httpPut(char* pAddress, int pPort, char* pRequest, unsigned long contentSize, int ssl);
sock_t httpGet(char* pAddress, int pPort, char* pRequest, int ssl);
int httpRecv(sock_t pSockId, char* pOut, int pOutSize);
int httpSend(sock_t pSockId, char* pOut, int pOutSize);
int httpLastError();
void httpClose(sock_t pSockId);
sock_t httpGetRequestSocket(const char *urlToDownload);
sock_t httpPutRequestSocket(const char *urlToUpload, unsigned long contentSize);
int recvLine(sock_t pSockId, char* pOut, int pOutSize);

#endif
