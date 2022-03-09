
#ifndef FILESYS_H
#define FILESYS_H
// contains helper funtions

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <unistd.h>
using namespace std;

string format_response(string code, string message)
{
	const string endline = "\r\n";
	string full_response = code + endline + "Length:" + to_string(message.length()) + endline + endline + message;

	return full_response;
}

// takes a file descriptor for the socket and a string and sends the string
// over the socket
// code from socket_prog.pptx Author: Dr. Zhu
void send_message(int sock_fd, string message)
{
	// TODO: LOOP
	const char *msg = message.c_str();
	// char *p = (char *)&msg;

	// int bytes_sent = 0;
	// while (bytes_sent < sizeof(msg))
	//{
	// cout << "currently sending: " << *msg << endl;
	// int x = send(sock_fd, (void *)p, sizeof(msg) - bytes_sent, 0);
	int x = send(sock_fd, msg, strlen(msg), 0);
	// cout << "sent " << x << " bytes" << endl;
	if (x == -1 || x == 0)
	{
		perror("Error while sending message over socket");
		close(sock_fd);
		exit(1);
	}

	// p += x;
	// bytes_sent += x;
	//}
}

struct recv_msg_t
{
	string message;
	bool quit;
};

recv_msg_t recv_message(int sock_fd)
{
	// TODO: LOOP
	string message;
	int ret;
	// while (true)
	//{
	char temp_buff[65535] = {}; // max packet size
	ret = recv(sock_fd, temp_buff, sizeof(temp_buff), 0);
	if (ret == -1)
	{
		perror("Error while receving message from socket");
		close(sock_fd);
		exit(1);
	}

	message += temp_buff;
	//}

	recv_msg_t msg;
	msg.message = message;
	msg.quit = ret == 0;

	return msg;
}

#endif