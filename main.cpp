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

	checkOrCreateRoot();					// ������� ���������� ��� ���������� ���������� ������������ � fileSystem

	WSADATA wsaData;												// ������ � ��������. 
	if (WSAStartup(MAKEWORD(2, 2)/*0000001000000010*/, &wsaData) != 0) {	// ������ ��� ��������������� �������� socket ���������� ������������������� ������� ���������� wsock32.dll ������ ������� WSAStartup. 0000001000000010 -  ������ ���������
	 	
		printf_s("Error");
		exit(1);
	}

	printf_s("Use this IP: 127.0.0.1\nthis port: 8080\n\n");
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);			// ���������� ���������� ���� SOCKET (domain, type, protocol) 
																				//�������� domain ������ ������������ ��� �������������� ����� ���������� (��� ���������������� �������), ��� ����� ���������� TCP/IP �� ������ ����� ���������� �������� AF_INET.
																				//�������� type ������ ����� ��������������: SOCK_STREAM - � ������������� ����������;SOCK_DGRAM - ��� ������������ ����������.
																				//IPPROTO_TCP- �������� ��� AF_INET
	if (ListenSocket == -1) {

		printf("error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	sockaddr_in addr;															//����� �� ������ ��������� ��� ������, ��� ����� ��� ���������� �������� ��������� SOCKADDR_IN
												
	addr.sin_family = AF_INET;													// ������ ���������. ���� sin_family ���������� ������������ ������ ������ (����� ����������), � ����� ������ (��� TCP/IP) ��� ������ ����� �������� AF_INET 
	addr.sin_port = htons((USHORT)DEFAULT_PORT);									//���� sin_port �������� ����� ����� �� ���� ����	
	inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));						//����������� ������ � ������� �����
							//���� sin_addr �������� ����� (�����) ���� ����.
																				//���� sin_zero �� ������������
																				//��������� SOCKADDR_IN ������ ���� ��������� ��������� ����� ������� ���������� ������ bind. ��� ����, ���� ���� sin_addr.s_addr ����� �������� INADDR_ANY, �� ��������� ����� ����� ����������� � socket'� ����� (�����) ���������� ���� ����.

																				//��� ����������� socket'� � ���������������� �����, ������������ �������������� �����, 
																				//���������� ��������� ��������� ����� bind, ������������ � �������� ��� ���� �������
																				//��������� ����� ������ ����� �� ������. � ����� TCP/IP socket ����������� � ��������� ������. 
	int resultOfFunction = 0;
	resultOfFunction = bind(ListenSocket, (LPSOCKADDR)&addr, sizeof(addr));							//(descriptor, addr, addrlen). addr - ��������� �� ���������  SOCKADDR_IN.

	resultOfFunction = listen(ListenSocket, SOMAXCONN);			//��������� ����� listen �������� ������� �������� ��� ���������-������� ������� ������� � ��� �� ��������-�������� (���������� ������ + )
													//�������� n ���������� ������������ ����� ������� �������� �������� �� ������������ �����. ���� �����-���� ������ ������ ������ �� ������������ ����� ��� ������ �������, �� ���� ������ ����� ���������.
													//somaxconn ������������ ������ ������� ��� ������ ����� TCP ����������. �������� �� ��������� 128
	if (resultOfFunction != 0 /*== INVALID_SOCKET*/) {

		printf(" error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	SOCKET socketForAccept;												//����� ��� ��� ��������������� �������� accept ������� �������� ��� ���� ���������� ���� SOCKET
	std::vector<SOCKET> socketVector;
	std::vector<std::thread> threadVector;
	size_t i = 0;
	while (true) {											//��� ������ �������� �� ��������-�������� �� ������������ ����� � ����������-�������� ������������ ��������� ����� accept
	
		socketForAccept = accept(ListenSocket, NULL, NULL);				//1 �������� - ���������� ������ ��� �������� ������ �� ��������. ���� ������ �� ������������ ������� �������, � �������� ������� � �������� ���������� ����� ������ NULL-���������.

		if (socketForAccept == INVALID_SOCKET) {
		
			printf(" error = %d\n", WSAGetLastError());
			continue;
		}
		socketVector.push_back(socketForAccept);
		threadVector.push_back(std::thread(recieveRequest, socketVector[i])); //������� ��� ���������� + �������()
		threadVector[i++].detach();//����������� �����. �� �������� �����������
	}
	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}