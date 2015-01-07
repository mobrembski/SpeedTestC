/*
    Http functions. Dont forget to initialize winsock.

    Luke Graham (39ster@gmail.com)
 */

#include <stdio.h>
#include "http.h"
#include "url.h"

int _httpErrorCode = 0;

int httpLastError()
{
	return _httpErrorCode;
}

int httpPut(char* pAddress, int pPort, char* pRequest, unsigned long contentSize)
{
	char buffer[0xFFFF];
	//Create the socket and connect to server.
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
	if (connect(sockId, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
		return 0;

	//Make and send get request
	sprintf(buffer, "POST %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"User-Agent: SPEEDTEST_CLIENT\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: %lu\r\n"
			"\r\n", pRequest, pAddress);
	send(sockId, buffer, strlen(buffer), 0);

	int length = 0;
	int success = 0;
	int i;
	return sockId;
}

int httpGet(char* pAddress, int pPort, char* pRequest)
{
	char buffer[0xFFFF];
	//Create the socket and connect to server.
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
	if (connect(sockId, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
		return 0;

	//Make and send get request
	sprintf(buffer, "GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"User-Agent: SPEEDTEST_CLIENT\r\n"
			"Connection: close\r\n"
			"\r\n", pRequest, pAddress);
	send(sockId, buffer, strlen(buffer), 0);

	int length = 0;
	int success = 0;
	int i;
	//Use a loop to receive all of the header.
	while((length = recvLine(sockId, buffer, sizeof(buffer))) > 0)
	{
		//trim line
		for(i = length-1; i >= 0; i--)
		{
			if(buffer[i] < ' ')
				buffer[i] = 0;
			else break;
		}

		//if end of header
		if(!strlen(buffer))
			break;

		//get first word
		char* token = strtok(buffer, " ");
		if(token == NULL)
			break;

		//Check http error code
		if(strcmp(token, "HTTP/1.0") == 0 || strcmp(token, "HTTP/1.1") == 0)
		{
			_httpErrorCode = atoi(strtok(NULL, " "));
			if(_httpErrorCode == 200)
				success = 1;
		}

	}

	if(!success==1)
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

	memset(&url, 0, sizeof(url));
	url.address = address;
	url.request = request;
	url.addressLen = sizeof(address);
	url.requestLen = sizeof(request);

	breakUrl(urlToDownload, &url);

	int sockId = httpGet(address, url.port, request);
	if(sockId)
		return sockId;
	fprintf(stderr, "Http error: %i\n", httpLastError());
	return 0;
}

int httpPutRequestSocket(const char *urlToUpload, unsigned long contentSize)
{
	char address[1024];
	char request[1024];
	URLPARTS_T url;

	memset(&url, 0, sizeof(url));
	url.address = address;
	url.request = request;
	url.addressLen = sizeof(address);
	url.requestLen = sizeof(request);

	breakUrl(urlToUpload, &url);

	int sockId = httpPut(address, url.port, request, contentSize);
	if(sockId)
		return sockId;
	fprintf(stderr, "Http error: %i\n", httpLastError());
	return 0;
}

int recvLine(int pSockId, char* pOut, int pOutSize)
{
	int received = 0;
	char letter;
	memset(pOut, 0, pOutSize);
	for(; received < pOutSize-1; received++)
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
