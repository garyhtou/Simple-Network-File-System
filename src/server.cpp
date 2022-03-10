#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include "FileSys.h"
#include "Helper.h"
using namespace std;

const int BACKLOG = 5; // Silent max value defined by C.

// struct to hold command lines
enum CommandType
{
    mkdir,
    ls,
    cd,
    home,
    rmdir_cmd, // Prevent name colision with an existing `rmdir` function.
    create,
    append,
    stat,
    cat,
    head,
    rm,
    quit,
    noop,
    invalid
};
struct Command
{
    CommandType type; // string to hold the function cmd.name
    string file;      // cmd.name of the file to manipulate
    string data;      // data to add or for the append or number for head
};

// Forward declaration
sockaddr_in get_server_addr(in_port_t port);
Command parse_command(string message);
void exec_command(int sock_fd, FileSys &fs, Command command);
void response_error(string message);
extern string format_response(string code, string message);
extern void send_message(int sock_fd, string message);
struct recv_msg_t
{
    string message;
    bool quit;
};
extern recv_msg_t recv_message_server(int sock_fd);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    // Networking part: create the socket and accept the client connectionSPEC);

    int sockfd = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);
    // cout << "Socket created with fd=" << sockfd << endl;
    if (sockfd < 0)
    {
        cerr << "Error: Failed to create socket" << endl;
        exit(1);
    }

    int opts = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));

    // BIND: bind to the given port number
    struct sockaddr_in server_addr = get_server_addr(port);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // Bind failed
        cerr << "Error: Failed to bind to port " << port << endl;
        exit(1);
    }

    // LISTEN: prepares to accept connections
    if (listen(sockfd, BACKLOG) < 0)
    {
        // Listen failed
        cerr << "Error: Failed to listen for connections" << endl;
        exit(1);
    }

    cout << "Server is ready to handle connections" << endl;

    // Handle multiple connections, serially (not concurrently)
    while (true)
    {
        try
        {
            // ACCEPT: accept a single connection
            struct sockaddr_in client_addr;
            int client_len;
            int new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
            if (new_sockfd < 0)
            {
                cerr << "Error: Failed to accept socket connection" << endl;
                break;
            }
            else
            {
                // cout << "New connection! (socket port: " << new_sockfd << ")" << endl;
            }

            // We now have a client, let's prepare the filesystem to handle requests.

            // Mount the file system
            FileSys fs;
            fs.mount(new_sockfd); // assume that sock is the new socket created
                                  // for a TCP connection between the client and the sin_addrer.

            recv_msg_t msg;
            msg.quit = false;
            // Take in new commands until the client quits (closes the connection)
            while (!msg.quit)
            {
                try
                {
                    // Receive the message
                    msg = recv_message_server(new_sockfd);
                    if (msg.quit)
                    {
                        // cout << "Client has closed the connection" << endl;
                        break;
                    }

                    string message = msg.message;
                    // cout << "Received command: " << message << endl;

                    // Parse the command
                    Command command = parse_command(message);

                    // Check that the command was valid
                    if (command.type == invalid)
                    {
                        // cout << "Command is invalid. " << command.data << endl;

                        // Send back the error message from the parser
                        send_message(new_sockfd, format_response(command.data, command.data));
                    }
                    else if (command.type == noop)
                    {
                        // cout << "Command is a noop (empty command)" << endl;

                        // Send an empty success message
                        send_message(new_sockfd, format_response("200 OK", ""));
                    }
                    else
                    {
                        // Execute the command
                        exec_command(new_sockfd, fs, command);
                    }

                    // cout << "=== FINISHED COMMAND ===\n\n"
                    //      << endl;
                }
                catch (...)
                {
                    // Command failed
                    send_message(new_sockfd, format_response("Command Failed", "Command Failed"));
                }
            }
            // close the listening sockets
            close(new_sockfd);

            // unmount the file system
            fs.unmount();
        }
        catch (...)
        {
            // This connection has failed. Ignore the failure to allow future connection
        }
    }

    // Close down server
    close(sockfd);

    return 0;
}

sockaddr_in get_server_addr(in_port_t port)
{
    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return server_addr;
}

void exec_command(int socket_fd, FileSys &fs, Command command)
{
    CommandType type = command.type;
    const char *file = command.file.c_str();
    const char *data = command.data.c_str();

    try
    {
        // FileSys will respond to the socket on a successful request. If there
        // is an error, then a WrappedFileSystem::FileSystemException will be
        // thrown by FileSys and this function will handle and respond to the
        // socket (below in `catch`).
        switch (type)
        {
        case mkdir:
            fs.mkdir(file);
            break;
        case ls:
            fs.ls();
            break;
        case cd:
            fs.cd(file);
            break;
        case home:
            fs.home();
            break;
        case rmdir_cmd:
            fs.rmdir(file);
            break;
        case create:
            fs.create(file);
            break;
        case append:
            fs.append(file, data);
            break;
        case stat:
            fs.stat(file);
            break;
        case cat:
            fs.cat(file);
            break;
        case head:
            fs.head(file, stoi(data));
            break;
        case rm:
            fs.rm(file);
            break;
        }
    }
    catch (const WrappedFileSys::FileSystemException &e)
    {
        string err_msg = e.what();

        string formatted_message = format_response(err_msg, "");
        // Response to socket with err_msg in proper format
        send_message(socket_fd, formatted_message);
    }
}

// Takes the buffer string and calls the
// appropriate function from FileSys
Command parse_command(string message)
{
    // Parse the buffer string
    struct Command cmd;
    istringstream ss(message);
    string name;
    int tokens = 0; // number of tokens

    // get tokens and determine number
    if (ss >> name)
    {
        tokens++;
        if (ss >> cmd.file)
        {
            tokens++;
        }
        if (ss >> cmd.data)
        {
            tokens++;
        }
        string garbage; // data to ignore
        if (ss >> garbage)
        {
            tokens++;
        }
    }

    // Convert the name to a CommandType
    if (name == "mkdir")
    {
        cmd.type = mkdir;
    }
    else if (name == "ls")
    {
        cmd.type = ls;
    }
    else if (name == "cd")
    {
        cmd.type = cd;
    }
    else if (name == "home")
    {
        cmd.type = home;
    }
    else if (name == "rmdir")
    {
        cmd.type = rmdir_cmd;
    }
    else if (name == "create")
    {
        cmd.type = create;
    }
    else if (name == "append")
    {
        cmd.type = append;
    }
    else if (name == "stat")
    {
        cmd.type = stat;
    }
    else if (name == "cat")
    {
        cmd.type = cat;
    }
    else if (name == "head")
    {
        cmd.type = head;
    }
    else if (name == "rm")
    {
        cmd.type = rm;
    }
    else
    {
        // empty command line handle
        if (tokens == 0)
        {
            cmd.type = noop;
        }
        else
        {
            cmd.type = invalid;
            cmd.data = "Invalid command: " + message;
        }
    }

    CommandType type = cmd.type;

    // if command lines have too many args
    if (type == ls || type == home || type == quit)
    {
        if (tokens != 1)
        {
            // Send error message
            cmd.type = invalid;
            cmd.data = "Invalid command: not enough arguments. Requires 1 token";
        }
    } // otherwise check for too few arguments
    else if (type == mkdir ||
             type == cd ||
             type == rmdir_cmd ||
             type == create ||
             type == cat ||
             type == rm ||
             type == stat)
    {
        if (tokens != 2)
        {
            // Send error message
            cmd.type = invalid;
            cmd.data = "Invalid command: not enough arguments. Requires 2 tokens";
        }
    }
    // too few args for append or head
    else if (type == append || type == head)
    {
        if (tokens != 3)
        {
            // Send error message
            cmd.type = invalid;
            cmd.data = "Invalid command: not enough arguments. Requires 3 token";
        }
    }

    return cmd;
}
