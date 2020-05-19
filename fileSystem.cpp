#include <stdlib.h>
#include "fileSystem.h"
#include "reciveHTTP.h"
#include <WinSock2.h>
#include <fileapi.h>
#include "responseHTTP.h"
#include "workWithURL.h"

//#include "requestHTTP.h"

void checkOrCreateRoot()
{
	WIN32_FIND_DATAA directory;												//���������, ������� ��������� ���������� � �����, ��������� � ������� WIN32_FIND_DATAA
	HANDLE hDerictory = FindFirstFileA(PATH_FOR_SERVER_ROOT, &directory);			// ������� ���� ���� �� ���� � ��������� ���������� � ���������. ���������� ���������� ���������� ����� ��� INVALID_HANDLE_VALUE, ���� ���� �� ������
	if (hDerictory == INVALID_HANDLE_VALUE)
	{
		CreateDirectoryA(PATH_FOR_SERVER_ROOT, NULL);					//�������, ��������� ����� ����������. ���������: LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes
	}
}

BOOL isFile(char Name[])
{
	if ((strstr(Name, ".") != NULL) && (strstr(Name, "./") == NULL))//���������� ��������� �� ������ ���������
	{
		return TRUE;
	}
	return FALSE;
}

void getRequest(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff) {

	int lengthOfPath;
	if (ArrLen == 0) {								//������� ��� ����� �����
		send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
		lengthOfPath = strlen(PATH_FOR_SERVER_ROOT) + strlen(PATH_DELIMITER) + strlen("*");
		char *pathOfDirectory = (char*)calloc(lengthOfPath + 1, sizeof(char));
		strcat_s(pathOfDirectory, lengthOfPath + 1, PATH_FOR_SERVER_ROOT);//��������� 3 �������� � 1
		strcat_s(pathOfDirectory, lengthOfPath + 1, PATH_DELIMITER);
		strcat_s(pathOfDirectory, lengthOfPath + 1, "*");
		readDirectory(Sock, pathOfDirectory);
		free(pathOfDirectory);
		//return;
	}
	else {

		lengthOfPath = strlen(PATH_FOR_SERVER_ROOT);
		char *Path = (char*)calloc(lengthOfPath + 1, sizeof(char));
		memcpy_s(Path, lengthOfPath + 1, PATH_FOR_SERVER_ROOT, lengthOfPath);
		CreateFullPath(PathArr, ArrLen, &Path);

		HANDLE hFile;
		WIN32_FIND_DATAA fileStructure;
		hFile = FindFirstFileA(Path, &fileStructure);//���� ���� �� ������� ���� � ��������� � ���������

		if (isFile(PathArr[ArrLen - 1])) {

			if (hFile == INVALID_HANDLE_VALUE) {

				send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			}
			else {

				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				readFile(Sock, Path, HTTPBuff);
			}
		}
		else {

			if (hFile == INVALID_HANDLE_VALUE) {

				send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			}
			else {

				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				ConcatPath(&Path, "*");
				readDirectory(Sock, Path);
			}
		}
	}
}
void CreateFullPath(char *PathArr[], int ArrLen, char **Path)
{
	for (int i = 0; i < ArrLen; i++)
	{
		ConcatPath(Path, PathArr[i]);
	}
}
void ConcatPath(char **Path, const char *NextName)
{
	int Len = strlen(*Path) + strlen(NextName) + strlen(PATH_DELIMITER);
	(*Path) = (char*)realloc((*Path), (Len + 1) * sizeof(char));
	strcat_s((*Path), Len + 1, PATH_DELIMITER);
	strcat_s((*Path), Len + 1, NextName);
}

void readDirectory(SOCKET Sock, char Path[])
{
	HANDLE search_file;
	WIN32_FIND_DATA fileStruct;
	search_file = FindFirstFileA(Path, &fileStruct);//�������� ���������� �������
	BOOL Empty = TRUE;
	while (FindNextFileA(search_file, &fileStruct) != 0)// ���������� ����� ������, �� ����������� �������
	{
		/*if (!strncmp(Data.cFileName, ".", 1) || !strncmp(Data.cFileName, "..", 2))//������ ���� ..
		{
		continue;
		}*/
		send(Sock, fileStruct.cFileName, strlen(fileStruct.cFileName), 0);
		send(Sock, HTTP_END_OF_LINE, strlen(HTTP_END_OF_LINE), 0);
		Empty = FALSE;
	}
	if (Empty)
	{
		send(Sock, MSG_DIR_IS_EMPTY, strlen(MSG_DIR_IS_EMPTY), 0);
		send(Sock, HTTP_END_OF_LINE, strlen(HTTP_END_OF_LINE), 0);
	}
	FindClose(search_file);
}

void readFile(SOCKET Sock, char Path[], char *HTTPBuff)
{
	FILE *In;
	fopen_s(&In, Path, "rb");// read binary, ��������� ����
	int ReadRes;
	do
	{
		ReadRes = fread_s(HTTPBuff, HTTP_BUFFER_SIZE + 1, sizeof(char), HTTP_BUFFER_SIZE, In);//������ ������ �� ����� � �����
		if (ReadRes != EOF)
		{
			int Tmp = ReadRes;
			char *TmpBuff = HTTPBuff;
			int SendRes;
			do
			{
				SendRes = send(Sock, TmpBuff, Tmp, 0);
				Tmp -= SendRes;
				TmpBuff += SendRes;
			} while (Tmp != 0);
		}
	} while (!feof(In));
	fclose(In);
}

void headRequest(char *PathArr[], int ArrLen, SOCKET Sock) {

	int lenthOfPath = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(lenthOfPath + 1, sizeof(char));
	memcpy_s(Path, lenthOfPath + 1, PATH_FOR_SERVER_ROOT, lenthOfPath);
	CreateFullPath(PathArr, ArrLen, &Path);

	HANDLE search_file;
	WIN32_FIND_DATA Data;
	search_file = FindFirstFileA(Path, &Data);

	if (search_file == INVALID_HANDLE_VALUE) {

		send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
	else {

		if (/*!*/(Data.dwFileAttributes  !=/*&*/ FILE_ATTRIBUTE_DIRECTORY)) { //���������� �������������� ������� (����� ��������� != ???)

			lenthOfPath = HTTP_BUFFER_SIZE;
			char *Buff = (char*)calloc(lenthOfPath + 1, sizeof(char));

			long long Size = (Data.nFileSizeHigh * (MAXDWORD + 1)) + Data.nFileSizeLow;// ����� ������� ���������� ������ �����

			sprintf_s(Buff, lenthOfPath + 1, "HTTP/1.1 200 OK\r\nFile-Size: %lld\r\nType: File\r\nName: %s\r\n\r\n\0", Size, Data.cFileName);// ����� ������ printf ???
			send(Sock, Buff, strlen(Buff), 0);
			free(Buff);
		}
		else {

			send(Sock, NOT_ALLOWED_RESPONSE, strlen(NOT_ALLOWED_RESPONSE), 0);
		}
	}

}
void putRequest(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff, int ReciveRes)//�������� ������ ������� �������	??
{
	if (ArrLen == 0)
	{
		send(Sock, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);
		return;
	}

	int lengthOfPath = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(lengthOfPath + 1, sizeof(char));
	memcpy_s(Path, lengthOfPath + 1, PATH_FOR_SERVER_ROOT, lengthOfPath);

	CreateDir(PathArr, ArrLen, &Path);
	HANDLE Res;
	WIN32_FIND_DATAA fileStruct;
	ConcatPath(&Path, PathArr[ArrLen - 1]);
	Res = FindFirstFileA(Path, &fileStruct);
	if (isFile(PathArr[ArrLen - 1]))
	{
		if (strstr(HTTPBuff, COPY_REQUEST) != NULL)
		{
			FileCopy(PathArr, ArrLen, Sock, HTTPBuff);

		}
		else
		{
			ReciveFile(Path, Sock, HTTPBuff, ReciveRes);
			if (Res == INVALID_HANDLE_VALUE)
			{
				/*response code will be 201 created*/
				send(Sock, CREATED_RESPONSE, strlen(CREATED_RESPONSE), 0);
				free(Path);
			}
			else
			{
				/*response code will be 200 ok*/
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				free(Path);
			}
		}
	}
	else
	{
		timeval Delay = { 1, 0 };
		fd_set ForRead = { 1, Sock };
		int RecvRes = 0;
		int Result;
		do
		{
			Result = select(0, &ForRead, NULL, NULL, &Delay);
			if (Result == SOCKET_ERROR)
			{
				printf("select function failed with error = %d\n", WSAGetLastError());
				free(Path);
				return;
			}

			if (Result > 0)
			{
				RecvRes = recv(Sock, HTTPBuff, HTTP_BUFFER_SIZE, 0);
				if (RecvRes == SOCKET_ERROR)
				{
					printf("recv function failed with error = %d\n", WSAGetLastError());
					return;
				}
				ForRead.fd_count = 1;
				ForRead.fd_array[0] = Sock;
			}
		} while (RecvRes != 0 && Result != 0);

		if (Res == INVALID_HANDLE_VALUE)
		{
			CreateDirectoryA(Path, NULL);
			free(Path);
			/*response code will be 201 created*/
			send(Sock, CREATED_RESPONSE, strlen(CREATED_RESPONSE), 0);
		}
		else
		{
			/*response code will be 200 ok*/
			send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			free(Path);
		}
	}
}

void deleteRequest(char *PathArr[], int ArrLen, SOCKET Sock)
{
	if (ArrLen == 0)
	{
		send(Sock, FORBIDDEN_RESPONSE, strlen(FORBIDDEN_RESPONSE), 0);
		return;
	}

	int lengthOfPath = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(lengthOfPath + 1, sizeof(char));
	memcpy_s(Path, lengthOfPath + 1, PATH_FOR_SERVER_ROOT, lengthOfPath);

	CreateFullPath(PathArr, ArrLen, &Path);

	HANDLE hFile;
	WIN32_FIND_DATA Data;
	hFile = FindFirstFileA(Path, &Data);
	if (isFile(PathArr[ArrLen - 1]))
	{
		if (hFile == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else
		{
			if (DeleteFileA(Path) != 0)
			{
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
			else
			{
				send(Sock, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
			}
		}
	}
	else
	{
		if (hFile == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else
		{
			ConcatPath(&Path, "*");
			if (DelDir(Path) != 0)
			{
				int Len = strlen(Path) - 1;
				Path = (char*)realloc(Path, Len); /*to delete '/*' symbols*/
				Path[Len - 1] = '\0';
				RemoveDirectoryA(Path);

				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
			else
			{
				send(Sock, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
			}
		}
	}
}

void CreateDir(char *PathArr[], int ArrLen, char **Path)
{
	HANDLE Res;
	WIN32_FIND_DATAA Mock;
	for (int i = 0; i < ArrLen - 1; i++)
	{
		ConcatPath(Path, PathArr[i]);
		Res = FindFirstFileA((*Path), &Mock);
		if (Res == INVALID_HANDLE_VALUE)
		{
			CreateDirectoryA((*Path), NULL);
		}
	}
}

void WriteInFile(FILE *Out, char *Buff, int Len)
{
	int Res;
	do
	{
		Res = fwrite(Buff, sizeof(char), Len, Out);
		Len -= Res;
		Buff += Res;
	} while (Len != 0);
}



void FileCopy(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff)
{
	char *Tmp = strstr(HTTPBuff, COPY_REQUEST);
	int DestLen;
	char **DestPathArr = ParseURL(Tmp, &DestLen);
	if (isFile(DestPathArr[DestLen - 1]) && isFile(PathArr[ArrLen - 1]))
	{
		int Len = strlen(PATH_FOR_SERVER_ROOT);
		char *Path = (char*)calloc(Len + 1, sizeof(char));
		memcpy_s(Path, Len + 1, PATH_FOR_SERVER_ROOT, Len);
		CreateDir(PathArr, ArrLen, &Path);
		ConcatPath(&Path, PathArr[ArrLen - 1]);

		int LenDest = strlen(PATH_FOR_SERVER_ROOT);
		char *PathDest = (char*)calloc(LenDest + 1, sizeof(char));
		memcpy_s(PathDest, Len + 1, PATH_FOR_SERVER_ROOT, Len);
		CreateDir(DestPathArr, DestLen, &PathDest);
		ConcatPath(&PathDest, DestPathArr[DestLen - 1]);

		if (strcmp(Path, PathDest) == 0)
		{
			send(Sock, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);
			return;
		}

		HANDLE ResDest, Res;
		WIN32_FIND_DATAA DataDest, Data;

		ResDest = FindFirstFileA(Path, &Data);
		Res = FindFirstFileA(PathDest, &DataDest);

		if (Res == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else
		{
			if (CopyFileA(PathDest, Path, FALSE) != 0)
			{
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				return;
			}
			else
			{
				send(Sock, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
				return;
			}
		}
	}
	else
	{
		send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
}





int DelDir(char *Path)
{
	HANDLE search_file;
	WIN32_FIND_DATA Data;
	search_file = FindFirstFileA(Path, &Data);
	int Result = 1;
	while (FindNextFileA(search_file, &Data) != 0)
	{
		if (!strncmp(Data.cFileName, ".", 1) || !strncmp(Data.cFileName, "..", 2))
		{
			continue;
		}

		if (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			int Len = strlen(Path);
			char *TmpPath = (char*)calloc(Len + 1, sizeof(char));
			memcpy_s(TmpPath, Len + 1, Path, Len - 2);
			ConcatPath(&TmpPath, Data.cFileName);
			ConcatPath(&TmpPath, "*");
			Result = DelDir(TmpPath);
			if (Result == 0)
			{
				return 0;
			}
			Len = strlen(TmpPath) - 1;
			TmpPath = (char*)realloc(TmpPath, Len); /*to delete '/*' symbols*/
			TmpPath[Len - 1] = '\0';
			if (RemoveDirectoryA(TmpPath) == 0)
			{
				free(TmpPath);
				return 0;
			}
			free(TmpPath);
		}
		else
		{
			int Len = strlen(Path);
			char *TmpPath = (char*)calloc(Len + 1, sizeof(char));
			memcpy_s(TmpPath, Len + 1, Path, Len - 2);/*to delete '/*' symbols*/
			ConcatPath(&TmpPath, Data.cFileName);
			if (DeleteFileA(TmpPath) == 0)
			{
				free(TmpPath);
				return 0;
			}
			free(TmpPath);
		}
	}
	FindClose(search_file);
	return Result;
}




