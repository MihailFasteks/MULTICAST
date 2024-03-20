#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib") // Winsock library
#pragma warning(disable:4996) // отключаем предупреждение _WINSOCK_DEPRECATED_NO_WARNINGS

SOCKET server_socket;
SOCKET client_socket[MAX_CLIENTS] = {};

void SendToAllClients(const char* message) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		SOCKET client = client_socket[i];
		if (client != 0) {
			send(client, message, strlen(message), 0);
		}
	}
}
vector<string> history;

int main() {
	system("title Server");

	puts("Start server... DONE.");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	// create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}
	// puts("Create socket... DONE.");

	// prepare the sockaddr_in structure
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// bind socket
	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}

	// puts("Bind socket... DONE.");

	// слушать входящие соединения
	listen(server_socket, MAX_CLIENTS);

	// принять и входящее соединение
	puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

	// размер нашего приемного буфера, это длина строки
	// набор дескрипторов сокетов
	// fd means "file descriptors"
	fd_set readfds; // https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-fd_set
	
	while (true) {
		// очистить сокет fdset
		FD_ZERO(&readfds);

		// добавить главный сокет в fdset
		FD_SET(server_socket, &readfds);

		// добавить дочерние сокеты в fdset
		for (int i = 0; i < MAX_CLIENTS; i++) 
		{
			SOCKET s = client_socket[i];
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		// дождитесь активности на любом из сокетов, тайм-аут равен NULL, поэтому ждите бесконечно
		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		// если что-то произошло на мастер-сокете, то это входящее соединение
		SOCKET new_socket; // новый клиентский сокет
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds)) {
			if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}

			for (int i = 0; i < history.size(); i++)
			{
				cout << history[i] << "\n";
				send(new_socket, history[i].c_str(), history[i].size(), 0);
			}
			
			// информировать серверную сторону о номере сокета - используется в командах отправки и получения
			printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			
			char SocketMessage[500];
			sprintf(SocketMessage, "New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			SendToAllClients(SocketMessage);
			
		/*	for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_socket[i] != 0) {
					send(client_socket[i], SocketMessage, strlen(SocketMessage), 0);
				}
			}*/
			// добавить новый сокет в массив сокетов
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					printf("Adding to list of sockets at index %d\n", i);
					break;
				}
			}
		}

		// если какой-то из клиентских сокетов отправляет что-то
		for (int j = 0; j < MAX_CLIENTS; j++)
		{
			SOCKET s = client_socket[j];
			// если клиент присутствует в сокетах чтения
			if (FD_ISSET(s, &readfds))
			{
				// получить реквизиты клиента
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);

				// проверьте, было ли оно на закрытие, а также прочитайте входящее сообщение
				// recv не помещает нулевой терминатор в конец строки (в то время как printf %s предполагает, что он есть)

				char client_message[DEFAULT_BUFLEN];
				char SocketMessage[256];
				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';

				string check_exit = client_message;
				if (check_exit == "off")
				{
					cout << "Client #" << j << " is off\n";
					closesocket(s); // Закрываем сокет клиента
					FD_CLR(s, &readfds); // Удаляем сокет из набора
					client_socket[j] = 0; // Удаляем сокет из массива
					sprintf(SocketMessage, "Client #%d is off\n", j);
					SendToAllClients(SocketMessage);
					break; // Добавляем break, чтобы выйти из цикла после закрытия сокета
				}

				//string temp = client_message;
				// temp += "\n";
				history.push_back(temp);

				for (int k = 0; k < MAX_CLIENTS; k++) {
					if (client_socket[k] != 0) {
						send(client_socket[k], client_message, client_message_length, 0);
					}
				}

			}
		}
	}

	WSACleanup();
}