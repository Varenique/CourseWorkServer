#include <stdlib.h>
#include <string.h>
#include "responseHTTP.h"
#include "reciveHTTP.h"
#include "requestHTTP.h"
#include "fileSystem.h"

void ProcessRequest(char *PathArr[], int ArrLen, TRequestType ReqType, SOCKET Sock, char *HTTPBuff, int ReciveRes)
{
	switch (ReqType)
	{
	case Put:
		PutFile(PathArr, ArrLen, Sock, HTTPBuff, ReciveRes);
		break;
	case Get:
		GetFile(PathArr, ArrLen, Sock, HTTPBuff);
		break;
	case Delete:
		DelFile(PathArr, ArrLen, Sock);
		break;
	case Head:
		HeadFile(PathArr, ArrLen, Sock);
		break;
	}
}