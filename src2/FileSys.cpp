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
  string dir_name = name;
  DirInode working_dir = this->get_working_dir();

  // Check if a directory or file under this name already exists
  for (DirEntry<DirInode> entry : working_dir.get_dir_entries())
  {
    if (entry.get_name() == dir_name)
    {
      throw WrappedFileSys::FileExistsException();
    }
  }
  for (DirEntry<FileInode> entry : working_dir.get_file_entries())
  {
    if (entry.get_name() == dir_name)
    {
      throw WrappedFileSys::FileExistsException();
    }
  }

  // Check if the given name is too long
  if (dir_name.size() > MAX_FNAME_SIZE)
  {
    throw WrappedFileSys::FileNameTooLongException();
  }

  // Check if the directory has an empty space for a new entry.
  if (!working_dir.has_free_entry())
  {
    throw WrappedFileSys::DirFullException();
  }

  // Attempt to create a block for this new directory
  DirInode new_dir = DirInode(); // May throw WrappedFileSys::DiskFullException();

  // Add the new DirNode as an entry to the current working directory
  DirEntry<DirInode> entry = DirEntry<DirInode>(dir_name, new_dir);
  working_dir.add_entry(entry);
  // TODO: 200?
}

// switch to a directory
// Should raise 500, 503
void FileSys::cd(const char *name)
{
  string dir_name = name;

  // Retreive working directory
  DirInode working_dir = this->get_working_dir();

  // Search directory until entry name match
  for (DirEntry<DirInode> entry : working_dir.get_dir_entries())
  {
    if (entry.get_name() == name)
    {
      // Found the directory
      this->set_working_dir(entry.get_inode());
      // TODO: do we need to return something to the socket?
      return;
    }
  }

  // Directory was not found, see if the name is of a file
  for (DirEntry<FileInode> entry : working_dir.get_file_entries())
  {
    if (entry.get_name() == name)
    {
      // Found a file with this name
      throw WrappedFileSys::NotADirException();
    }
  }

  // There was no directory or file with this name
  throw WrappedFileSys::FileNotFoundException();
}

// switch to home directory
void FileSys::home()
{
  this->set_working_dir(DirInode(HOME_DIR_ID));
  // TODO: 200 code message
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
