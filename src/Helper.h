
#ifndef FILESYS_H
#define FILESYS_H
// contains helper funtions

#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <unistd.h>
using namespace std;

const string endline;

string format_response(string message)
{
	string full_response;

	full_response += "200 OK" + endline;
	full_response += "Length:" + message.length() + endline;
	full_response += endline;
	full_response += message;

	return full_response;
}

// takes a file descriptor for the socket and a string and sends the string
// over the socket
// code from socket_prog.pptx Author: Dr. Zhu
void send_message(int sock_fd, string message)
{
	cout << "sock_fd: " << sock_fd << endl;
	string formatted_message = format_response(message);

	const char *msg = formatted_message.c_str();
	char *p = (char *)&msg;

	int bytes_sent = 0;
	while (bytes_sent < sizeof(msg))
	{
		cout << "currently sending: " << (void *)p << endl;
		int x = send(sock_fd, (void *)p, sizeof(msg) - bytes_sent, 0);
		cout << "sent " << x << " bytes" << endl;
		if (x == -1 || x == 0)
		{
			perror("error on write");
			close(sock_fd);
			exit(1);
		}

		p += x;
		bytes_sent += x;
	}
}

string recv_message(int sock_fd)
{
	string message;
	int ret;
	while (true)
	{
		cout << "in loop" << endl;
		char temp_buff[65535]; // max packet size
		ret = recv(sock_fd, temp_buff, sizeof(temp_buff), 0);
		if (ret == 0)
		{
			break;
		}
		else if (ret == -1)
		{
			perror("error on write");
			close(sock_fd);
			exit(1);
		}

		cout << "DEBUG: message received (partial): " << temp_buff << endl;
		message += temp_buff;
	}

	return message;
}

#endif