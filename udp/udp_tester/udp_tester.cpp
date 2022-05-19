// udp_tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include"winsock2.h"

#include<iostream>

#include<conio.h>



using namespace std;



#define MYPORT 9009    // the port users will be connecting to

int main()
{
    std::cout << "Hello UDP!\n";

    WSADATA wsaData;

    WSAStartup(MAKEWORD(2, 2), &wsaData);



    SOCKET sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);



    char broadcast = '1';



    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)

    {

        cout << "Error in setting Broadcast option";

        closesocket(sock);

        return 0;

    }



    struct sockaddr_in Recv_addr;

    struct sockaddr_in Sender_addr;



    int len = sizeof(struct sockaddr_in);



    char sendMSG[] = "Broadcast message from SLAVE TAG";



    char recvbuff[50] = "";

    int recvbufflen = 50;



    Recv_addr.sin_family = AF_INET;

    Recv_addr.sin_port = htons(MYPORT);

      Recv_addr.sin_addr.s_addr  = INADDR_BROADCAST; // this isq equiv to 255.255.255.255

    // better use subnet broadcast (for our subnet is 172.30.255.255)

    //Recv_addr.sin_addr.s_addr = inet_addr("172.30.255.255");


      setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, 0,0);


    sendto(sock, sendMSG, strlen(sendMSG) + 1, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));





    recvfrom(sock, recvbuff, recvbufflen, 0, (sockaddr*)&Recv_addr, &len);



    cout << "\n\n\tReceived message from Reader is =>  " << recvbuff;



    cout << "\n\n\tpress any key to CONT...";

    _getch();



    closesocket(sock);

    WSACleanup();

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
