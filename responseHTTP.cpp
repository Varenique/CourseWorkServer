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
		putRequest(PathArr, ArrLen, Sock, HTTPBuff, ReciveRes);
		break;
	case Get:
		getRequest(PathArr, ArrLen, Sock, HTTPBuff);
		break;
	case Delete:
		deleteRequest(PathArr, ArrLen, Sock);
		break;
	case Head:
		headRequest(PathArr, ArrLen, Sock);
		break;
	}
}