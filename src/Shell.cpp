// CPSC 3500: Shell
// Implements a basic shell (command line interface) for the file system

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Shell.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Helper.h"
#include <strings.h>
using namespace std;

static const string PROMPT_STRING = "NFS> "; // shell prompt
const string endline = "\r\n";

// Mount the network file system with server name and port number in the format of server:port
void Shell::mountNFS(string fs_loc)
{
  // create the socket cs_sock and connect it to the server and port specified in fs_loc
  // if all the above operations are completed successfully, set is_mounted to true

  // make vector with filesys location, servername, and port
  // vector<string> filesys_addr;
  string hostname;
  string port;
  string junk;

  size_t curr = 0;
  size_t delimit_idx;
  // parse fs_loc string
  // while ((delimit_idx = fs_loc.find_first_of(':', curr)) != string::npos)
  //{
  // filesys_addr.push_back(fs_loc.substr(curr, delimit_idx - curr));
  // curr = delimit_idx++;
  //}
  // filesys_addr.push_back(fs_loc.substr(curr));
  istringstream ss(fs_loc);
  string token;
  if (getline(ss, token, ':'))
  {
    hostname = token;
    if (getline(ss, token))
    {
      port = token;
    }
  }

  // cout << "Server Name: " << hostname << "  Port: " << port << endl;

  addrinfo *addr, hints;
  bzero(&hints, sizeof(hints));
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = PF_UNSPEC;

  int ret;
  if ((ret = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &addr)) != 0)
  {
    cout << "Error: Could not obtain address information for \"" << hostname << ":" << port << "\"" << endl;
    cout << "getaddrinfo returned with " << ret << endl;
    exit(1);
  }
  // create socket to connect
  this->cs_sock = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);

  if (cs_sock < 0)
  {
    cout << "Error: Failed to create a socket" << endl;
    cout << "socket returned with " << cs_sock << endl;
    exit(1);
  }

  // connect to server
  int connect_ret = connect(cs_sock, addr->ai_addr, addr->ai_addrlen);
  if (connect_ret < 0)
  {
    cout << "Error: Failed to connect with server" << endl;
    cout << "connect returned with " << connect_ret << endl;
    exit(1);
  }

  // Free the linked list created by getaddrinfo
  freeaddrinfo(addr);

  is_mounted = true;
}

// Unmount the network file system if it was mounted
void Shell::unmountNFS()
{
  // close the socket if it was mounted
  if (is_mounted)
  {
    close(cs_sock);
    is_mounted = false;
  }
}

// Remote procedure call on mkdir
void Shell::mkdir_rpc(string dname)
{
  string cmd = "mkdir " + dname + endline;
  network_command(cmd);
}

// Remote procedure call on cd
void Shell::cd_rpc(string dname)
{
  string cmd = "cd " + dname + endline;
  network_command(cmd);
}

// Remote procedure call on home
void Shell::home_rpc()
{
  string cmd = "home" + endline;
  network_command(cmd);
}

// Remote procedure call on rmdir
void Shell::rmdir_rpc(string dname)
{
  string cmd = "rmdir " + dname + endline;
  network_command(cmd);
  // to implement
}

// Remote procedure call on ls
void Shell::ls_rpc()
{
  string cmd = "ls " + endline;
  network_command(cmd);
}

// Remote procedure call on create
void Shell::create_rpc(string fname)
{
  string cmd = "create " + fname + endline;
  network_command(cmd);
}

// Remote procedure call on append
void Shell::append_rpc(string fname, string data)
{
  string cmd = "append " + fname + " " + data + endline;
  network_command(cmd);
}

// Remote procesure call on cat
void Shell::cat_rpc(string fname)
{
  string cmd = "cat " + fname + endline;
  network_command(cmd);
}

// Remote procedure call on head
void Shell::head_rpc(string fname, int n)
{
  string cmd = "head_rpc " + fname + " " + to_string(n) + endline;
  network_command(cmd);
}

// Remote procedure call on rm
void Shell::rm_rpc(string fname)
{
  string cmd = "rm " + fname + endline;
  network_command(cmd);
}

// Remote procedure call on stat
void Shell::stat_rpc(string fname)
{
  string cmd = "stat " + fname + endline;
  network_command(cmd);
}

// Executes the shell until the user quits.
void Shell::run()
{
  // make sure that the file system is mounted
  if (!is_mounted)
    return;

  // continue until the user quits
  bool user_quit = false;
  while (!user_quit)
  {

    // print prompt and get command line
    string command_str;
    cout << PROMPT_STRING;
    getline(cin, command_str);

    // execute the command
    user_quit = execute_command(command_str);
  }

  // unmount the file system
  unmountNFS();
}

// Execute a script.
void Shell::run_script(char *file_name)
{
  // make sure that the file system is mounted
  if (!is_mounted)
    return;
  // open script file
  ifstream infile;
  infile.open(file_name);
  if (infile.fail())
  {
    cerr << "Could not open script file" << endl;
    return;
  }

  // execute each line in the script
  bool user_quit = false;
  string command_str;
  getline(infile, command_str, '\n');
  while (!infile.eof() && !user_quit)
  {
    cout << PROMPT_STRING << command_str << endl;
    user_quit = execute_command(command_str);
    getline(infile, command_str);
  }

  // clean up
  unmountNFS();
  infile.close();
}

// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str)
{
  // parse the command line
  struct Command command = parse_command(command_str);

  // look for the matching command
  if (command.name == "")
  {
    return false;
  }
  else if (command.name == "mkdir")
  {
    mkdir_rpc(command.file_name);
  }
  else if (command.name == "cd")
  {
    cd_rpc(command.file_name);
  }
  else if (command.name == "home")
  {
    home_rpc();
  }
  else if (command.name == "rmdir")
  {
    rmdir_rpc(command.file_name);
  }
  else if (command.name == "ls")
  {
    ls_rpc();
  }
  else if (command.name == "create")
  {
    create_rpc(command.file_name);
  }
  else if (command.name == "append")
  {
    append_rpc(command.file_name, command.append_data);
  }
  else if (command.name == "cat")
  {
    cat_rpc(command.file_name);
  }
  else if (command.name == "head")
  {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno)
    {
      head_rpc(command.file_name, n);
    }
    else
    {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  }
  else if (command.name == "rm")
  {
    rm_rpc(command.file_name);
  }
  else if (command.name == "stat")
  {
    stat_rpc(command.file_name);
  }
  else if (command.name == "quit")
  {
    return true;
  }

  return false;
}

void Shell::network_command(string message)
{
  // Format message for network transit
  string formatted_mesage = message + endline;

  // Send command over the network (through the provided socket)
  send_message(this->cs_sock, formatted_mesage, false);

  recv_msg_t msg = recv_message(this->cs_sock);
  if (msg.quit)
  {
    cout << "Server has closed the connection" << endl;
    exit(0);
  }

  string response = msg.message;

  // Parse response (remove the protcol header)
  // The code, length, and body are separated by "\r\n"
  string code, length, body;
  size_t lenPos = response.find("\r\n");
  code = response.substr(0, lenPos);
  size_t bodyPos = response.find("\r\n", lenPos + 2);
  length = response.substr(lenPos + 2, bodyPos);
  body = response.substr(bodyPos + 4); // There should be two sets of "\r\n"

  string output = body;
  if (body.length() == 0)
  {
    output = code;
  }

  cout << output << endl;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Command Shell::parse_command(string command_str)
{
  // empty command struct returned for errors
  struct Command empty = {"", "", ""};

  // grab each of the tokens (if they exist)
  struct Command command;
  istringstream ss(command_str);
  int num_tokens = 0;
  if (ss >> command.name)
  {
    num_tokens++;
    if (ss >> command.file_name)
    {
      num_tokens++;
      if (ss >> command.append_data)
      {
        num_tokens++;
        string junk;
        if (ss >> junk)
        {
          num_tokens++;
        }
      }
    }
  }

  // Check for empty command line
  if (num_tokens == 0)
  {
    return empty;
  }

  // Check for invalid command lines
  if (command.name == "ls" ||
      command.name == "home" ||
      command.name == "quit")
  {
    if (num_tokens != 1)
    {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "mkdir" ||
           command.name == "cd" ||
           command.name == "rmdir" ||
           command.name == "create" ||
           command.name == "cat" ||
           command.name == "rm" ||
           command.name == "stat")
  {
    if (num_tokens != 2)
    {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "append" || command.name == "head")
  {
    if (num_tokens != 3)
    {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else
  {
    cerr << "Invalid command line: " << command.name;
    cerr << " is not a command" << endl;
    return empty;
  }

  return command;
}
