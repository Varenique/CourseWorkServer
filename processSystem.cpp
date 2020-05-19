#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "processSystem.h"
#include "requestHTTP.h"
#include "responseHTTP.h"
#include "workWithURL.h"
#include "fileSystem.h"

void recieveRequest(SOCKET ClientSock) {

	int Res, sizeOfRecievdData;
	char *HTTPBuffer = (char *)calloc(HTTP_BUFFER_SIZE + 1, sizeof(char));

	sizeOfRecievdData = recv(ClientSock, HTTPBuffer, HTTP_BUFFER_SIZE, 0);// �������� ���������� HTTP ������� (����������, �������� �� �����, ������ ������ � �����). ��������� ������ ���������� ������ ��� SOCKET_ERROR
	printf_s("%s\n", HTTPBuffer);

	TRequestType requestType = getRequestType(HTTPBuffer);
	if (requestType == NotImplemented) {

		send(ClientSock, NOT_IMPLEMENTED_RESPONSE, strlen(NOT_IMPLEMENTED_RESPONSE), 0);// ���������� ����� ������� � ������� ������� send
	}
	else {

		int lengthOfPath;
		char **arrayOfPath = parseURL(HTTPBuffer, &lengthOfPath);
		if (isTwoDots(arrayOfPath, lengthOfPath))
		{
			send(ClientSock, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);// ���������� ����� ������� � ������� ������� send
		}
		else {

			getResponse(arrayOfPath, lengthOfPath, requestType, ClientSock, HTTPBuffer);
			free(arrayOfPath);
		}
	}

	free(HTTPBuffer);
	shutdown(ClientSock, SD_BOTH);//��������� �������� � ��������� �� ������
	closesocket(ClientSock);
	return;
}

bool isTwoDots(char **arrayOfPath, int lengthOfPath) {

	char *twoDots = NULL;
	bool result = false;
	for (int i = 0; i < lengthOfPath; i++) {

		twoDots = strstr(arrayOfPath[i], "..");
		if (twoDots != NULL) {

			result = true;
		}
	}
	return result;
}