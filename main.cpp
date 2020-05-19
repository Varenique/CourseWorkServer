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
//#pragma comment (lib, "iphlpapi.lib")

#define WORKING_BUFFER_SIZE 15000
#define DEFAULT_PORT 8080


int main() {

	checkOrCreateRoot();					// создает директорию или записывает дескриптор существующей в fileSystem

	WSADATA wsaData;												// работа с сокетами. 
	if (WSAStartup(MAKEWORD(2, 2)/*0000001000000010*/, &wsaData) != 0) {	// Прежде чем воспользоваться функцией socket необходимо проинициализировать процесс библиотеки wsock32.dll вызвав функцию WSAStartup. 0000001000000010 -  версия библиотки
	 	
		printf_s("Error");
		exit(1);
	}

	printf_s("Use this IP: 127.0.0.1\nthis port: 8080\n\n");
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);			// объявление переменной типа SOCKET (domain, type, protocol) 
																				//Аргумент domain задает используемый для взаимодействия набор протоколов (вид коммуникационной области), для стека протоколов TCP/IP он должен иметь символьное значение AF_INET.
																				//Аргумент type задает режим взаимодействия: SOCK_STREAM - с установлением соединения;SOCK_DGRAM - без установления соединения.
																				//IPPROTO_TCP- протокол для AF_INET
	if (ListenSocket == -1) {

		printf("error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	sockaddr_in addr;															//Далее мы задаем параметры для сокета, для этого нам необходимо объявить структуру SOCKADDR_IN
												
	addr.sin_family = AF_INET;													// задаем параметры. Поле sin_family определяет используемый формат адреса (набор протоколов), в нашем случае (для TCP/IP) оно должно иметь значение AF_INET 
	addr.sin_port = htons((USHORT)DEFAULT_PORT);									//Поле sin_port содержит номер порта на узле сети	
	inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));						//преобразуем строку в сетевой адрес
							//Поле sin_addr содержит адрес (номер) узла сети.
																				//Поле sin_zero не используется
																				//Структура SOCKADDR_IN должна быть полностью заполнена перед выдачей системного вызова bind. При этом, если поле sin_addr.s_addr имеет значение INADDR_ANY, то системный вызов будет привязывать к socket'у номер (адрес) локального узла сети.

																				//Для подключения socket'а к коммуникационной среде, образованной вычислительной сетью, 
																				//необходимо выполнить системный вызов bind, определяющий в принятом для сети формате
																				//локальный адрес канала связи со средой. В сетях TCP/IP socket связывается с локальным портом. 
	int resultOfFunction = 0;
	resultOfFunction = bind(ListenSocket, (LPSOCKADDR)&addr, sizeof(addr));							//(descriptor, addr, addrlen). addr - указатель на структуру  SOCKADDR_IN.

	resultOfFunction = listen(ListenSocket, SOMAXCONN);			//Системный вызов listen выражает желание выдавшей его программы-сервера ожидать запросы к ней от программ-клиентов (дескриптор сокета + )
													//Аргумент n определяет максимальную длину очереди входящих запросов на установление связи. Если какой-либо клиент выдаст запрос на установление связи при полной очереди, то этот запрос будет отвергнут.
													//somaxconn ограничивает размер очереди для приема новых TCP соединений. Значение по умолчанию 128
	if (resultOfFunction != 0 /*== INVALID_SOCKET*/) {

		printf(" error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	SOCKET socketForAccept;												//Перед тем как воспользоваться функцией accept сначала объявите ещё одну переменную типа SOCKET
	std::vector<SOCKET> socketVector;
	std::vector<std::thread> threadVector;
	size_t i = 0;
	while (true) {											//Для приема запросов от программ-клиентов на установление связи в программах-серверах используется системный вызов accept
	
		socketForAccept = accept(ListenSocket, NULL, NULL);				//1 аргумент - дескриптор сокета для принятия связей от клиентов. Если сервер не интересуется адресом клиента, в качестве второго и третьего аргументов можно задать NULL-указатели.

		if (socketForAccept == INVALID_SOCKET) {
		
			printf(" error = %d\n", WSAGetLastError());
			continue;
		}
		socketVector.push_back(socketForAccept);
		threadVector.push_back(std::thread(recieveRequest, socketVector[i])); //функция для выполнения + аргумет()
		threadVector[i++].detach();//отсоединяем поток. Он работает обособленно
	}
	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}