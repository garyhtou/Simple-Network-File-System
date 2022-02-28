// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

#include "WrappedFileSys.h"

// mounts the file system
void FileSys::mount(int sock)
{
  bfs.mount();
  curr_dir = 1;   // by default current directory is home directory, in disk block #1
  fs_sock = sock; // use this socket to receive file system operations from the client and send back response messages

  // Set the WrapperFileSys's BFS instance
  WrappedFileSys::bfs = &bfs;
}

// unmounts the file system
void FileSys::unmount()
{
  bfs.unmount();
  close(fs_sock);
}

// make a directory
void FileSys::mkdir(const char *name)
{
 
  //create directroy block

  Block<dirblock_t> B = Block(); 
  B.write_and_set_raw_block(B.get_raw());

  // create directory inode
  DirInode dir_node = new DirInode();

  
  //instantiate list that will contain user-readable name lowlevel 
  //name pairs for files in directory

}

// switch to a directory
void FileSys::cd(const char *name)
{ 
  //retreive current directory
  Block curr = Block(curr_dir);

  //search curr until entry name match

    //if match

      //change curr_id to id at loop



}

// switch to home directory
void FileSys::home()
{
  //change curr_dir to 1
}

// remove a directory
void FileSys::rmdir(const char *name)
{ 

}

// list the contents of current directory
void FileSys::ls()
{
}

// create an empty data file
void FileSys::create(const char *name)
{
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
  //navigate get file
  // 
}

// display the contents of a data file
void FileSys::cat(const char *name)
{
}

// display the first N bytes of the file
void FileSys::head(const char *name, unsigned int n)
{
}

// delete a data file
void FileSys::rm(const char *name)
{
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
}

// HELPER FUNCTIONS (optional)
