
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
	string formatted_message = format_response(message);

	const char *msg = formatted_message.c_str();
	char *p = (char *)&msg;
	int bytes_sent = 0;
	while (bytes_sent < sizeof(msg))
	{
		int x = write(sock_fd, (void *)p, sizeof(msg) - bytes_sent);
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

#endif