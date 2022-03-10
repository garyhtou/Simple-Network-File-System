
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
void send_message(int sock_fd, string message)
{
	const char *msg_ptr = message.c_str();
	int msg_size = message.size();
	int bytes_sent;

	while (msg_size > 0)
	{
		bytes_sent = send(sock_fd, msg_ptr, msg_size, 0);
		if (bytes_sent < 0)
		{
			perror("Error sending message");
			return;
		}

		msg_ptr += bytes_sent;
		msg_size -= bytes_sent;
	}
}

struct recv_msg_t
{
	string message;
	bool quit;
};

recv_msg_t recv_message_client(int sock_fd)
{
	recv_msg_t msg;
	msg.quit = false;
	char temp_buff[150] = {};

	int size;
	while (true)
	{
		size = recv(sock_fd, temp_buff, sizeof(temp_buff), 0);
		if (size > 0)
		{
			// New data. Copy into message string.
			for (int i = 0; i < size; i++)
			{
				msg.message += temp_buff[i];
			}
			// Check if we've recieved the full message

			// Start by identifying the length of the message body
			size_t lenPos = msg.message.find("Length:");
			if (lenPos == string::npos)
			{
				continue;
			}
			size_t lenEndPos = msg.message.find("\r\n", lenPos + 1);
			if (lenEndPos == string::npos)
			{
				continue;
			}

			string lengthStr = msg.message.substr(lenPos + 7, lenEndPos);
			int length = -1;
			try
			{
				length = stoi(lengthStr);
			}
			catch (...)
			{
				// Failed to parse length
				break;
			}

			// Move past the who sets of "\r\n" after the length
			size_t bodyPos = msg.message.find("\r\n", lenEndPos + 1);
			if (bodyPos == string::npos)
			{
				continue;
			}

			string body = msg.message.substr(bodyPos, msg.message.length());
			if (body.length() < length)
			{
				// We're still missing a portion of the message body
				continue;
			}

			// We've got the full message
			break;
		}

		else if (size == 0)
		{
			// The socket as closed on the other end
			msg.quit = true;
			break;
		}
		else
		{
			// An error has occured (size < 0)
			perror("Error while receving message from socket");
			close(sock_fd);
			return msg;
		}
	}

	// cout << "\tdone" << endl;
	return msg;
}

recv_msg_t recv_message_server(int sock_fd)
{
	recv_msg_t msg;
	msg.quit = false;
	char temp_buff[150] = {};

	int size;
	while (true)
	{
		size = recv(sock_fd, temp_buff, sizeof(temp_buff), 0);
		if (size > 0)
		{
			for (int i = 0; i < size; i++)
			{
				msg.message += temp_buff[i];
			}
			// Start by identifying the length of the message body
			size_t endPos = msg.message.find("\r\n");
			if (endPos == string::npos)
			{
				// Still missing the full message
				continue;
			}

			// We've got the full message
			break;
		}
		else if (size == 0)
		{
			// The socket as closed on the other end
			msg.quit = true;
			break;
		}
		else
		{
			// An error has occured (size < 0)
			perror("Error while receving message from socket");
			close(sock_fd);
			return msg;
		}
	}

	return msg;
}

#endif