// CPSC 3500: main
// Executes the file system program by starting the shell.

#include <iostream>
#include <cstring>
using namespace std;

#include "Shell.h"

int main(int argc, char **argv)
{
  Shell shell;

  if (argc == 2) {
    shell.mountNFS(string(argv[1]));
    shell.run();
  }
  else if (argc == 4 && strcmp(argv[1], "-s") == 0) {
    shell.mountNFS(string(argv[3]));
    shell.run_script(argv[2]);
  }
  else {
    cerr << "Invalid command line" << endl;
    cerr << "Usage (one of the following): " << endl;
    cerr << "./nfsclient server:port" << endl;
    cerr << "./nfsclient -s <script-name> server:port" << endl;
  }

  return 0;
}
