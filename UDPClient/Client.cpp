
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
COORD point;// Координата точки
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); // получаем адрес собственного окна

DWORD WINAPI Sender(void* param)
{

        cout << "Please, input your nickname and color number" << endl;
        char nick[DEFAULT_BUFLEN];
        cin.getline(nick, DEFAULT_BUFLEN);
        /* send(client_socket, nick, strlen(nick), 0);*/

        while (true) {
            cout << "Please insert your query for server: ";
            char query[DEFAULT_BUFLEN];
            cin.getline(query, DEFAULT_BUFLEN);
            char message[DEFAULT_BUFLEN];
            strcpy(message, nick);
            strcat(message, " ");
            strcat(message, query);
            send(client_socket, message, strlen(message), 0);

            // альтернативный вариант ввода данных стрингом
            // string query;
            // getline(cin, query);
            // send(client_socket, query.c_str(), query.size(), 0);
        
    }
}

DWORD WINAPI Receiver(void* param)
{
    while (true) {
        char response[DEFAULT_BUFLEN];
        char ServerResponse[DEFAULT_BUFLEN];
        char nickname[DEFAULT_BUFLEN] = {0};
        char color[DEFAULT_BUFLEN] = {0};
        char text[DEFAULT_BUFLEN] = {0};
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';
       /* int counter=0;
        for (int i = 0; i < result; i++) {
            if ((response[i] == ' ')&&(response[i-1]!=' ')) {
                counter++;
            }
            do {
                strcat(nickname, &response[i]);
            } while (counter==0);
            if (counter == 1) {
                strcat(color, &response[i]);
            }
            else if (counter< 2) {
                strcat(text, &response[i]);
            }
            
        }*/
        if (strcmp(response, "off") == 0) {
            printf("Server: A client has disconnected.\n");
            continue;
        }
        if (strstr(response, "New connection") != nullptr) {
            printf("%s\n", response);
            continue;
        }
        sscanf(response, "%s %s %[^\n]", nickname, color, text);
        int colorNum= atoi(color);
        SetConsoleTextAttribute(h, colorNum);  // изменение цвета шрифта
        // cout << "...\nYou have new response from server: " << response << "\n";
     /*   strcat(ServerResponse, nickname);
        
        strcat(ServerResponse, " : ");
        strcat(ServerResponse, text);*/
        sprintf(ServerResponse, "%s : %s", nickname, text);

        cout << ServerResponse << "\n";
        SetConsoleTextAttribute(h, 007);
        // cout << "Please insert your query for server: ";
    }
}

BOOL ExitHandler(DWORD whatHappening)
{
    switch (whatHappening)
    {
    case CTRL_C_EVENT: // closing console by ctrl + c
    case CTRL_BREAK_EVENT: // ctrl + break
    case CTRL_CLOSE_EVENT: // closing the console window by X button
      return(TRUE);
        break;
    default:
        return FALSE;
    }
}

int main()
{
    // обработчик закрытия окна консоли
    //SetConsoleCtrlHandler((PHANDLER_ROUTINE)ExitHandler, true);

    system("title Client");

    // initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // разрешить адрес сервера и порт
    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    // пытаться подключиться к адресу, пока не удастся
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // создать сокет на стороне клиента для подключения к серверу
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (client_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }

        // connect to server
        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }
    CreateThread(0, 0, Receiver, 0, 0, 0);
    CreateThread(0, 0, Sender, 0, 0, 0);
    

    Sleep(INFINITE);
}