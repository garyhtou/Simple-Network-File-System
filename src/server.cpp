#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "FileSys.h"
using namespace std;

const int BACKLOG = 5;       // Silently max value defined by C.
const int BUFFER_LEN = 2048; // TODO: why?

// struct to hold command lines
struct Command
{
    commands name; // string to hold the function cmd.name
    string file;   // cmd.name of the file to manipulate
    string data;   // data to add or for the append or number for head
}

enum commands { ls = "ls",
                cd = "cd",
                home = "home",
                rmdir = "rmdir",
                create = "create",
                append = "append",
                stat = "stat",
                cat = "cat",
                head = "head",
                rm = "rm" };

// Forward declaration
sockaddr_in
get_server_addr(in_port_t port);
void parse_call(string message);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    // Networking part: create the socket and accept the client connection
    // SOCKET: create a socket
    int sockfd = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);

    // BIND: bind to the given port number
    struct sockaddr_in server_addr = get_server_addr(port);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // Bind failed
        cerr << "Failed to bind to port " << port << endl;
        exit(1);
    }

    // LISTEN: prepares to accept connections
    if (listen(sockfd, BACKLOG) < 0)
    {
        // Listen failed
        cerr << "Failed to listen for connections" << endl;
        exit(1);
    }

    // ACCEPT: accept a single connection
    struct sockaddr_in client_addr;
    int client_len;
    int new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
    if (new_sockfd < 0)
    {
        // Accept failed
        cerr << "Failed to accept socket" << endl;
        exit(1);
    }

    // We now have a client, let's prepare the filesystem to handle requests.

    // Mount the file system
    FileSys fs;
    fs.mount(new_sockfd); // assume that sock is the new socket created
                          // for a TCP connection between the client and the server.

    // Receive the message
    string message;        // string that acts as abuffer
    char temp_buff[65535]; // max packet size
    int retries_left = 3;
    while (true)
    {
        // Receive requests for data
        int recv_ret = recv(new_sockfd, temp_buff, sizeof(temp_buff), 0);
        if (recv_ret < 0)
        {
            // Retry
            retries_left--;
            if (retries_left < 0)
            {
                cerr << "Error receiving data from socket" << endl;
                exit(1);
            }
            continue;
        }
        else if (recv_ret == 0)
        {
            // End of message
            break;
        }

        // Process the packet
        for (int i = 0; i < recv_ret; i++)
        {
            message.append(message[i]);
        }
    }

    // Execute the command
    exec_call(message);

    // close the listening socket
    close(new_sockfd);

    // unmout the file system
    fs.unmount();

    return 0;
}

sockaddr_in get_server_addr(in_port_t port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // sets all bytes of sin_zero (which is not used)
    // to the null terminatior
    for (int i = 0; i < sizeof(8); i++)
    {
        server_addr.sin_zero[i] = '\0';
    }
    return server_addr;
}

void exec_call(string message)
{
    Command command = parse_call(message);

    try
    {
        switch ()
    }
    catch (const WrappedFileSys::FileSystemException &e)
    {
        string err_msg = e.what();
        // Response to socket with err_msg in proper format
    }
}

// Takes the buffer string and calls the
// appropriate function from FileSys
struct Command parse_call(string message)
{

    // parse the buffer string
    struct Command cmd;
    istringstream ss(message);
    int tokens = 0; // number of tokens

    // get tokens and determine number
    if (ss >> cmd.name)
    {
        tokens++;
        if (ss >> cmd.file)
        {
            tokens++;
        }
        if (ss >> cmd.data)
        {
            tokens++;
            string garbage; // data to ignore
        }
        if (ss >> garbage)
        {
            tokens++;
        }
    }

    // empty command line handle
    if (num_tokens == 0)
    {
        // TODO: send back error message saying empty/no command provided
        exit(1);
    }

    // if command lines have too many args
    if (cmd.name == "ls" || cmd.name == "home" || cmd.name == "quit")
    {
        if (tokens != 1)
        {
            // TODO: send error message "Invalide command: not enough args"
            exit(1);
        }
    } // otherwise check for too few arguments
    else if (cmd.name == "mkdir" ||
             cmd.name == "cd" ||
             cmd.name == "rmdir" ||
             cmd.name == "create" ||
             cmd.name == "cat" ||
             cmd.name == "rm" ||
             cmd.name == "stat")
    {
        if (num_tokens != 2)
        {
            // TODO: error handling
            exit(1);
        }
    }
    // too few args for append or head
    else if (cmd.name == "append" || cmd.name == "head")
    {
        if (num_tokens != 3)
        {
            // TODO: error handling
            exit(1)
        }
    }
    else
    {
        // TODO: invalid command
        exit(1);
    }
}