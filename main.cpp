#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include <thread>
#include "reciveHTTP.h"
#include "fileSystem.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib, "iphlpapi.lib")

#define WORKING_BUFFER_SIZE 15000
#define DEFAULT_PORT 8080

ULONG GetServerIP()
{
	//ULONG BroadcastAddr = 0;
	ULONG Flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_GATEWAYS;
	ULONG BuffSize = WORKING_BUFFER_SIZE;
	PIP_ADAPTER_ADDRESSES Adr = (PIP_ADAPTER_ADDRESSES)malloc(BuffSize), CurrAdr;

	ULONG RetVal = GetAdaptersAddresses(AF_INET, Flags, NULL, Adr, &BuffSize);			//извлекает адреса типа AF_INET с перечисленными флагами, не используется - должен быть NULL,
																						//Указатель на область память, которая будет содержать список IP_ADAPTER_ADDRESSES в случае успеха, указатель на ее размер
	CurrAdr = Adr;//будем просматривать все адреса, которые вернулись
	ULONG Result = 0;
	// необходимо сделать конвертирование протоколов одной среды в протоколы другой среды
	if (RetVal == NO_ERROR)
	{
		BOOL GatewayIPFounded = FALSE;
		while ((CurrAdr != NULL) && !(GatewayIPFounded)/*|| (GatewayIPFounded)*/)
		{
			GatewayIPFounded = (CurrAdr->FirstGatewayAddress != NULL);							//получаем адрес шлюза
			if (GatewayIPFounded)
			{
				sockaddr_in *IPInAddr = (sockaddr_in*)CurrAdr->FirstUnicastAddress->Address.lpSockaddr;
				Result = IPInAddr->sin_addr.s_addr;
			}
			CurrAdr = CurrAdr->Next;
		}
	}
	else
	{
		//LPVOID lpMsgBuf = NULL;
		printf("Call to GetAdaptersAddresses failed with error: %d\n", RetVal);
		if (RetVal == ERROR_NO_DATA)
			printf("\tNo addresses were found for the requested parameters\n");
		//else
		//{
		//	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		//		NULL, RetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),/* Default language*/(LPTSTR)&lpMsgBuf, 0, NULL))
		//	{
		//		printf("\tError: %s", (char*)lpMsgBuf);
		//	}
		//}
	}

	free(Adr);
	return Result;
}

int main()
{
	//SetConsoleOutputCP(1251);		//устанавливает кодовую таблицу для потока вывода
	//SetConsoleCP(1251);			//для потока ввода (для кириллицы)

	CreateRoot();					// создает директорию или записывает дескриптор существующей в fileSystem

	int Res = 0;
	WSADATA wsaData;												// работа с сокетами. 
	if (WSAStartup(/*MAKEWORD(2, 2)*/0000001000000010, &wsaData) != 0)	// Прежде чем воспользоваться функцией socket необходимо проинициализировать процесс библиотеки wsock32.dll вызвав функцию WSAStartup. 0000001000000010 -  версия библиотки
	{
		printf_s("Error");
		exit(1);
	}

	//ULONG ServerIP = GetServerIP();
	//std::string IPStr;
	//if (inet_ntop(AF_INET, &ServerIP, (PSTR)(IPStr.c_str()), 16) == NULL)			//конвертируем IPv4 адресс из бинарного в текстовый формат. ServerIP из бинарного в текстовый в бифер IPStr 
	//{
	//	printf_s("inet_ntop failed with error: %d\n", WSAGetLastError());
	//	WSACleanup();
	//	getchar();
	//	return 1;
	//}
	//else
	//{
	//	printf_s("Server's IP = %s\nServer's port = %d\n\n\n", IPStr.c_str(), DEFAULT_PORT);
	//	//IPStr.~basic_string();
	//}
	
	SOCKET ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);				// объявление переменной типа SOCKET (domain, type, protocol) 
																				//Аргумент domain задает используемый для взаимодействия набор протоколов (вид коммуникационной области), для стека протоколов TCP/IP он должен иметь символьное значение AF_INET.
																				//Аргумент type задает режим взаимодействия: SOCK_STREAM - с установлением соединения;SOCK_DGRAM - без установления соединения.
																				//IPPROTO_TCP- протокол для AF_INET
	if (ListenSock == /*INVALID_SOCKET*/-1)
	{
		printf("socket function failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	sockaddr_in Adr;															//Далее мы задаем параметры для сокета (сервера) для этого нам необходимо объявить структуру SOCKADDR_IN
	//const int AdrLen = sizeof(Adr);												
	Adr.sin_family = AF_INET;													// задаем параметры. Поле sin_family определяет используемый формат адреса (набор протоколов), в нашем случае (для TCP/IP) оно должно иметь значение AF_INET 
	Adr.sin_port = htons((USHORT)DEFAULT_PORT);									//Поле sin_port содержит номер порта на узле сети
	//Adr.sin_addr.s_addr = ServerIP;	
	inet_pton(AF_INET, "127.0.0.1", &(Adr.sin_addr));
	//Adr.sin_addr.s_addr = inet_addr("127.0.0.1");							//Поле sin_addr содержит адрес (номер) узла сети.
																				//Поле sin_zero не используется
																				//Структура SOCKADDR_IN должна быть полностью заполнена перед выдачей системного вызова bind. При этом, если поле sin_addr.s_addr имеет значение INADDR_ANY, то системный вызов будет привязывать к socket'у номер (адрес) локального узла сети.

																				//Для подключения socket'а к коммуникационной среде, образованной вычислительной сетью, 
																				//необходимо выполнить системный вызов bind, определяющий в принятом для сети формате
																				//локальный адрес канала связи со средой. В сетях TCP/IP socket связывается с локальным портом. 
	Res = bind(ListenSock, (/*SOCKADDR**/LPSOCKADDR)&Adr, /*AdrLen*/ sizeof(Adr));							//(descriptor, addr, addrlen). addr - указатель на структуру  SOCKADDR_IN.

	Res = listen(ListenSock, SOMAXCONN);			//Системный вызов listen выражает желание выдавшей его программы-сервера ожидать запросы к ней от программ-клиентов (дескриптор сокета + )
													//Аргумент n определяет максимальную длину очереди входящих запросов на установление связи. Если какой-либо клиент выдаст запрос на установление связи при полной очереди, то этот запрос будет отвергнут.
													//somaxconn ограничивает размер очереди для приема новых TCP соединений. Значение по умолчанию 128
	if (Res != 0 /*== INVALID_SOCKET*/)
	{
		printf("listen function failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	SOCKET Tmp;												//Перед тем как воспользоваться функцией accept сначала объявите ещё одну переменную типа SOCKET
	std::vector<SOCKET> SockVector;
	std::vector<std::thread> ThreadPool;
	size_t i = 0;
	while (true)											//Для приема запросов от программ-клиентов на установление связи в программах-серверах используется системный вызов accept
	{
		Tmp = accept(ListenSock, NULL, NULL);				//1 аргумент - дескриптор сокета для принятия связей от клиентов. Если сервер не интересуется адресом клиента, в качестве второго и третьего аргументов можно задать NULL-указатели.

		if (Tmp == INVALID_SOCKET)
		{
			printf("accept function failed with error = %d\n", WSAGetLastError());
			continue;
		}
		SockVector.push_back(Tmp);
		ThreadPool.push_back(std::thread(ReciveRequestAndSendResponse, SockVector[i])); //функция для выполнения + аргумет()
		ThreadPool[i++].detach();//отсоединяем поток. Он работает обособленно
	}

	WSACleanup();
	//getchar();
	//return 0;
	return 0;
}