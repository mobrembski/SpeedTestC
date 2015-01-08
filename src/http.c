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
#include "url.h"
#include "http.h"

int _httpErrorCode = 0;

int httpLastError()
{
	return _httpErrorCode;
}

int httpPut(char* pAddress, int pPort, char* pRequest, unsigned long contentSize)
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
			"User-Agent: SPEEDTEST_CLIENT\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: %lu\r\n"
			"\r\n", pRequest, pAddress, 100*contentSize);
	send(sockId, buffer, strlen(buffer), 0);

	return sockId;
}

int httpGet(char* pAddress, int pPort, char* pRequest)
{
	char buffer[0xFFFF], *token;
	int sockId;
	struct sockaddr_in addr;
	struct hostent*  hostEntry;
	int length = 0;
	int success = 0;
	int i;
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

	sprintf(buffer, "GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"User-Agent: SPEEDTEST_CLIENT\r\n"
			"Connection: close\r\n"
			"\r\n", pRequest, pAddress);
	send(sockId, buffer, strlen(buffer), 0);

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

	if(!success == 1)
	{
		close(sockId);
		return 0;
	} else return sockId;
}

int httpRecv(int pSockId, char* pOut, int pOutSize)
{
	int size;
	if((size = recv(pSockId, pOut, pOutSize, 0)) > 0)
		return size;
	return 0;
}

int httpSend(int pSockId, char* pOut, int pOutSize)
{
	int size;
	if((size = send(pSockId, pOut, pOutSize, 0)) > 0)
		return size;
	return 0;
}

void httpClose(int pSockId)
{
	close(pSockId);
}

int httpGetRequestSocket(const char *urlToDownload)
{
	char address[1024];
	char request[1024];
	URLPARTS_T url;
	int sockId;

	memset(&url, 0, sizeof(url));
	url.address = address;
	url.request = request;
	url.addressLen = sizeof(address);
	url.requestLen = sizeof(request);

	breakUrl(urlToDownload, &url);

	sockId = httpGet(address, url.port, request);
	if(sockId)
		return sockId;
	fprintf(stderr, "Http error while creating GET request socket: %i\n",
		httpLastError());
	return 0;
}

int httpPutRequestSocket(const char *urlToUpload, unsigned long contentSize)
{
	char address[1024];
	char request[1024];
	URLPARTS_T url;
	int sockId;

	memset(&url, 0, sizeof(url));
	url.address = address;
	url.request = request;
	url.addressLen = sizeof(address);
	url.requestLen = sizeof(request);

	breakUrl(urlToUpload, &url);

	sockId = httpPut(address, url.port, request, contentSize);
	if(sockId)
		return sockId;
	fprintf(stderr, "Http error while creating PUT request socket: %i\n",
		httpLastError());
	return 0;
}

int recvLine(int pSockId, char* pOut, int pOutSize)
{
	int received = 0;
	char letter;
	memset(pOut, 0, pOutSize);
	for(; received < pOutSize - 1; received++)
	{
		if(recv(pSockId, (char*)&letter, 1, 0) > 0)
		{
			pOut[received] = letter;
			if(letter == '\n')
				break;
		} else break;
	}
	return received;
}
