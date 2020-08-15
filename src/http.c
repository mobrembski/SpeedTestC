/*
	Http functions. Dont forget to initialize winsock.

	Luke Graham (39ster@gmail.com)
	Michał Obrembski (byku@byku.com.pl)
 */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef OPENSSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/opensslv.h>
#endif

#include "url.h"
#include "http.h"

#define USER_AGENT "SpeedTestC"

int _httpErrorCode = 0;

int httpLastError()
{
	return _httpErrorCode;
}

#ifdef OPENSSL
static BIO *bioWrap(int s, int ssl, int client)
{
    BIO *bio = BIO_new_socket(s, BIO_CLOSE);
    if (ssl) {
			SSL_CTX *ctx;
			BIO *sslbio;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
			if (client)
			    ctx = SSL_CTX_new(TLSv1_2_client_method());
			else
			    ctx = SSL_CTX_new(TLSv1_2_server_method());
#else
			if (client)
					ctx = SSL_CTX_new(TLS_client_method());
			else
					ctx = SSL_CTX_new(TLS_server_method());
#endif
			if (!ctx) {
			    return NULL;		/* XXX complain */
			}

			sslbio = BIO_new_ssl(ctx, client);
			if (!sslbio) {
			    return NULL;		/* XXX complain */
	}

	// layer SSL over socket
	bio = BIO_push(sslbio, bio);
    }
    return bio;
}
#endif

sock_t httpPut(char* pAddress, int pPort, char* pRequest, unsigned long contentSize, int ssl)
{
	char buffer[0xFFFF];
	int sockId;
	struct sockaddr_in addr;
	struct hostent*  hostEntry;

	if ((sockId = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0)
		return 0;

	if ((hostEntry = gethostbyname(pAddress)) == NULL)
		return 0;

	addr.sin_family = AF_INET;
	addr.sin_addr = *((struct in_addr*)*hostEntry->h_addr_list);
	addr.sin_port = htons((unsigned short)pPort);
	if (connect(sockId, (struct sockaddr*)&addr,
		sizeof(struct sockaddr_in)) == -1)
			return 0;

	/* TODO: Content-Length isn't set up, this is some kind of "hack".
	 I cannot understand, but some servers closes up connection too early
	 even if we set up Content-Lenght to 10 times more than we actually send.
	 Leaving it uninitialized gives us random high value.*/
	sprintf(buffer, "POST %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"User-Agent: %s\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: %lu\r\n"
			"\r\n", pRequest, pAddress, USER_AGENT, 100*contentSize);

#ifdef OPENSSL
	sock_t sock = bioWrap(sockId, ssl, 1);
	BIO_write(sock, buffer, strlen(buffer));

	return sock;
#else
	send(sockId, buffer, strlen(buffer), 0);

	return sockId;
#endif
}

sock_t httpGet(char* pAddress, int pPort, char* pRequest, int ssl)
{
	char buffer[0xFFFF], *token;
	sock_t sockId;
	int s;
	struct sockaddr_in addr;
	struct hostent*  hostEntry;
	int length = 0;
	int success = 0;
	int i;
	if ((s = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0)
		return 0;

	if ((hostEntry = gethostbyname(pAddress)) == NULL)
		return 0;

	addr.sin_family = AF_INET;
	addr.sin_addr = *((struct in_addr*)*hostEntry->h_addr_list);
	addr.sin_port = htons((unsigned short)pPort);
	if (connect(s, (struct sockaddr*)&addr,
		sizeof(struct sockaddr_in)) == -1)
		return 0;

	sprintf(buffer, "GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"User-Agent: %s\r\n"
			"Connection: close\r\n"
			"\r\n", pRequest, pAddress, USER_AGENT);

#ifdef OPENSSL
	sockId = bioWrap(s, ssl, 1);
	BIO_write(sockId, buffer, strlen(buffer));
#else
	sockId = s;
	send(sockId, buffer, strlen(buffer), 0);
#endif

	while((length = recvLine(sockId, buffer, sizeof(buffer))) > 0)
	{
		/*trim line*/
		for(i = length-1; i >= 0; i--)
		{
			if(buffer[i] < ' ')
				buffer[i] = 0;
			else break;
		}

		/*if end of header*/
		if(!strlen(buffer))
			break;

		/*get first word*/
		token = strtok(buffer, " ");
		if(token == NULL)
			break;

		/*Check http error code*/
		if(strcmp(token, "HTTP/1.0") == 0 || strcmp(token, "HTTP/1.1") == 0)
		{
			_httpErrorCode = atoi(strtok(NULL, " "));
			if(_httpErrorCode == 200)
				success = 1;
		}

	}

	if(!success)
	{
#ifdef OPENSSL
		BIO_free_all(sockId);
#else
		close(sockId);
#endif
		return 0;
	} else return sockId;
}

int httpRecv(sock_t pSockId, char* pOut, int pOutSize)
{
	int size;
#ifdef OPENSSL
	if((size = BIO_read(pSockId, pOut, pOutSize)) > 0)
		return size;
#else
	if((size = recv(pSockId, pOut, pOutSize, 0)) > 0)
		return size;
#endif
	return 0;
}

int httpSend(sock_t pSockId, char* pOut, int pOutSize)
{
	int size;
#ifdef OPENSSL
	if((size = BIO_write(pSockId, pOut, pOutSize)) > 0)
		return size;
#else
	if((size = send(pSockId, pOut, pOutSize, 0)) > 0)
		return size;
#endif
	return 0;
}

void httpClose(sock_t pSockId)
{
#ifdef OPENSSL
	BIO_free_all(pSockId);
#else
	close(pSockId);
#endif
}

sock_t httpGetRequestSocket(const char *urlToDownload)
{
	char address[1024];
	char request[1024];
	URLPARTS_T url;
	sock_t sockId;

	memset(&url, 0, sizeof(url));
	url.address = address;
	url.request = request;
	url.addressLen = sizeof(address);
	url.requestLen = sizeof(request);

#ifdef TRACE
	printf("GET %s\n", urlToDownload);
#endif
	breakUrl(urlToDownload, &url);

	sockId = httpGet(address, url.port, request, url.ssl);
	if(sockId)
		return sockId;
	fprintf(stderr, "Http error while creating GET request socket: %i\n",
		httpLastError());
	return 0;
}

sock_t httpPutRequestSocket(const char *urlToUpload, unsigned long contentSize)
{
	char address[1024];
	char request[1024];
	URLPARTS_T url;
	sock_t sockId;

	memset(&url, 0, sizeof(url));
	url.address = address;
	url.request = request;
	url.addressLen = sizeof(address);
	url.requestLen = sizeof(request);

#ifdef TRACE
	printf("PUT %s\n", urlToUpload);
#endif
	breakUrl(urlToUpload, &url);

	sockId = httpPut(address, url.port, request, contentSize, url.ssl);
	if(sockId != BAD_SOCKID)
		return sockId;
	fprintf(stderr, "Http error while creating PUT request socket: %i\n",
		httpLastError());
	return BAD_SOCKID;
}

static int
sockRecv(sock_t pSockId, char *ptr, int len)
{
#ifdef OPENSSL
    return BIO_read(pSockId, ptr, len);
#else
    return recv(pSockId, ptr, len, 0);
#endif
}

int recvLine(sock_t pSockId, char* pOut, int pOutSize)
{
	int received = 0;
	char letter;
	memset(pOut, 0, pOutSize);
	for(; received < pOutSize - 1; received++)
	{
		if(sockRecv(pSockId, (char*)&letter, 1) > 0)
		{
			pOut[received] = letter;
			if(letter == '\n')
				break;
		} else break;
	}
	return received;
}
