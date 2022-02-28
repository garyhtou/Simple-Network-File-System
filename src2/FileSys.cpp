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

  // Default working directory to Home Directory
  this->home();
}

// unmounts the file system
void FileSys::unmount()
{
  bfs.unmount();
  close(fs_sock);
}

// make a directory
// Should raise 502, 504, 505, 506
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
// Should raise 500, 503
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
  this->set_working_dir(DirInode(HOME_DIR_ID));
}

// remove a directory
// Should raise 500, 503, 507
void FileSys::rmdir(const char *name)
{ 
}

// list the contents of current directory
void FileSys::ls()
{
}

// create an empty data file
// Should raise 502, 504, 505, 506
void FileSys::create(const char *name)
{
}

// append data to a data file
// Should raise 501, 503, 505, 508
void FileSys::append(const char *name, const char *data)
{
  //navigate get file
  // 
}

// display the contents of a data file
// Should raise 501, 503
void FileSys::cat(const char *name)
{
}

// display the first N bytes of the file
// Should raise 501, 503
void FileSys::head(const char *name, unsigned int n)
{
}

// delete a data file
// Should raise 501, 503
void FileSys::rm(const char *name)
{
}

// display stats about file or directory
// Should raise 503
void FileSys::stat(const char *name)
{
}

// HELPER FUNCTIONS (optional)
void FileSys::set_working_dir(DirInode dir)
{
  this->curr_dir = dir.get_id();
}
DirInode FileSys::get_working_dir()
{
  return DirInode(this->curr_dir);
}
