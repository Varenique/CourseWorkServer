#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "reciveHTTP.h"
#include "requestHTTP.h"
#include "responseHTTP.h"
#include "workWithURL.h"
#include "fileSystem.h"

BOOL DotsFound(char **PathArr, int ArrLen)
{
	char *Tmp = NULL;
	for (int i = 0; i < ArrLen; i++)
	{
		Tmp = strstr(PathArr[i], "..");
		if (Tmp != NULL)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void ReciveRequestAndSendResponse(SOCKET ClientSock){

	int Res, RecvRes;
	//bool RequestFound = false, EndOfData = false;
	//timeval Delay = { 1, 0 };								//ждем 1 секунду
	//fd_set ForRead = { 1, ClientSock };
	char *HTTPBuff = (char *)calloc(HTTP_BUFFER_SIZE + 1, sizeof(char));

	//Res = select(0, &ForRead, NULL, NULL, &Delay);					//проверяем, можно ли произвести чтение, через 1 сек возвращаем управление. 
	//if (Res == SOCKET_ERROR)
	//{
	//	printf("select function failed with error = %d\n", WSAGetLastError());
	//	free(HTTPBuff);
	//	getchar();
	//	return;
	//}

	RecvRes = recv(ClientSock, HTTPBuff, HTTP_BUFFER_SIZE, 0);// получаем содержимое HTTP запроса (дескриптор, укзатель на буфер, размер буфера и флаги). Возращает размер полученных данных или SOCKET_ERROR
	printf_s("%s\n", HTTPBuff);

	/*define request type*/
	TRequestType ReqType = GetRequestType(HTTPBuff);
	if (ReqType == NotImplemented)
	{
		/*send response with "method not implemented" code*/
		send(ClientSock, NOT_IMPLEMENTED_RESPONSE, strlen(NOT_IMPLEMENTED_RESPONSE), 0);// Отправляем ответ клиенту с помощью функции send
	}
	else
	{
		/*parse url*/
		int ArrLen;
		char **PathArr = ParseURL(HTTPBuff, &ArrLen);
		if (DotsFound(PathArr, ArrLen))
		{
			send(ClientSock, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);// Отправляем ответ клиенту с помощью функции send
		}
		else
		{
			/*do anything depending on the type of request*/
			ProcessRequest(PathArr, ArrLen, ReqType, ClientSock, HTTPBuff, RecvRes);
			free(PathArr);
		}
	}

	free(HTTPBuff);
	shutdown(ClientSock, SD_BOTH);
	closesocket(ClientSock);
	return;
}

void ReciveFile(char Path[], SOCKET Sock, char HTTPBuff[], int ReciveRes)
{
	int Res;

	int Len = strlen(HTTP_END_OF_LINE) * 2;
	char *EndOfHeader = (char*)calloc(Len + 1, sizeof(char));
	strcat_s(EndOfHeader, Len + 1, HTTP_END_OF_LINE);
	strcat_s(EndOfHeader, Len + 1, HTTP_END_OF_LINE);

	char *Tmp = strstr(HTTPBuff, EndOfHeader);
	Tmp += Len;
	Len = abs(Tmp - HTTPBuff);
	ReciveRes -= Len;
	FILE *Out;
	fopen_s(&Out, Path, "wb");

	WriteInFile(Out, Tmp, ReciveRes);

	timeval Delay = { 1, 0 };
	fd_set ForRead = { 1, Sock };
	int RecvRes = 0;

	do
	{
		Res = select(0, &ForRead, NULL, NULL, &Delay);
		if (Res == SOCKET_ERROR)
		{
			printf("select function failed with error = %d\n", WSAGetLastError());
			fclose(Out);
			return;
		}

		if (Res > 0)
		{
			RecvRes = recv(Sock, HTTPBuff, HTTP_BUFFER_SIZE, 0);
			if (RecvRes == SOCKET_ERROR)
			{
				printf("recv function failed with error = %d\n", WSAGetLastError());
				fclose(Out);
				return;
			}

			if (RecvRes > 0)
			{
				WriteInFile(Out, HTTPBuff, RecvRes);
			}
			ForRead.fd_count = 1;
			ForRead.fd_array[0] = Sock;
		}
	} while (RecvRes != 0 && Res != 0);

	fclose(Out);
}