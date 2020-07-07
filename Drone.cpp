// Drone.cpp : Defines the entry point for the application.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>

#include "Drone.h"

using namespace std;

int main()
{
	cout << "Starting server..." << endl;
	// Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int videosocket = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1 || videosocket == -1)
	{
		cout << "Filed to create socket. errno: " << errno << endl;
		exit(EXIT_FAILURE);
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0 || setsockopt(videosocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		cout << "Failed to setsockopt. errno: " << errno << endl;
		exit(EXIT_FAILURE);
	}

	// Listen to port
	sockaddr_in sockaddr = get_address(9999);
	sockaddr_in videoaddr = get_address(9990);

	if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0 || bind(videosocket, (struct sockaddr*)&videoaddr, sizeof(videoaddr)) < 0)
	{
		cout << "Failed to bind to port 9999. errno: " << errno << endl;
		exit(EXIT_FAILURE);
	}

	// Start listening
	if (listen(sockfd, 10) < 0 || listen(videosocket, 1) < 0)
	{
		cout << "Failed to listen on socket. errno: " << errno << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Server started. Waiting client connnection..." << endl;

	// Grab a  connection from the queue
	auto addrlen = sizeof(sockaddr);
	int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	if (connection < 0)
	{
		cout << "Failed to grab connection. errno: " << errno << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Client connected. Starting camera..." << endl;

	// Start camera
	pid_t pid = 0;

	if ((pid = fork()) == 0)
	{
		char* const cmd[] = { "raspivid", "-md", "5", "-n", "-t", "0", "-fps", "25", "-w", "1296", "-h", "730", "-o", "tcp://0.0.0.0:9990", (char*)0 };

		execv("/usr/bin/raspivid", cmd);
	}

	// Grab a video connection
	auto videoaddrlen = sizeof(videoaddr);
	int videoconnection = accept(videosocket, (struct sockaddr*)&videoaddr, (socklen_t*)&videoaddrlen);
	if (videoconnection < 0)
	{
		cout << "Failed to grab video connection. errno: " << errno << endl;
		exit(EXIT_FAILURE);
	}
	
	cout << "Camera started." << endl;

	// read bytes from video connection and send message

	char buffer[4096];
	ssize_t bytes_read;
	do
	{
		bytes_read = recv(videoconnection, buffer, sizeof(buffer), 0);
		if (bytes_read > 0)
		{
			send(connection, buffer, sizeof(buffer), 0);
			cout << "Sended " << sizeof(buffer) << " kbytes" << endl;
		}
	} while (bytes_read > 0);

	// Close connection
	close(connection);
	close(sockfd);
	close(videosocket);
	close(videoconnection);
	// Stop capture
	kill(pid, 10);
	sleep(1);
	kill(pid, 10);
}

sockaddr_in get_address(int port)
{
	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);

	return sockaddr;
}
