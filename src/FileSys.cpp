// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// TODO: BST for inode table
#include <array>

// mounts the file system
void FileSys::mount(int sock)
{
  bfs.mount();
  curr_dir = 1;   // by default current directory is home directory, in disk block #1
  fs_sock = sock; // use this socket to receive file system operations from the client and send back response messages
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
}

// switch to a directory
void FileSys::cd(const char *name)
{
}

// switch to home directory
void FileSys::home()
{
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

void FileSys::init_inode_table()
{
  // Create and set the "Home" directory's inode
  inode_table_entry_t home_inode_table_entry_s;
  home_inode_table_entry_s.used = true;
  home_inode_table_entry_s.inode = Inode(1);

  this->inode_table[1] = home_inode_table_entry_s;
}

Inode FileSys::create_inode()
{
  short open_inode_id = -1;
  for (int i = 0; i < MAX_INODES; i++)
  {
    if (!this->inode_is_used(i))
    {
      open_inode_id = i;
      break;
    }
  }

  if (open_inode_id == -1)
  {
    cerr << "ERROR (InodeTable::create_inode): Max inodes reached. Max=" << MAX_INODES << endl;
    return NULL;
  }

  // Attempt to get a free block from the file system
  short free_block_id = bfs.get_free_block();
  if (free_block_id == 0)
  {
    cerr << "ERROR (InodeTable::create_inode): No free blocks available." << endl;
    return NULL;
  }

  // Create the new inode
  Inode inode(open_inode_id, free_block_id);

  // Write the inode to the file system
  bfs.write_block(inode.get_block_id(), (void *)&inode.get_raw_struct());

  // Add the new inode to the inode table (cache)
  inode_table_entry_t inode_table_entry_s;
  inode_table_entry_s.used = true;
  inode_table_entry_s.inode = inode;
  this->inode_table[open_inode_id] = inode_table_entry_s;

  return inode;
}

Inode FileSys::get_inode(short id)
{
  // Get from inode table (cache)
  return this->inode_table[id].inode;
}
void FileSys::delete_inode(short id)
{
  // Delete the blocks that belong to this inode (if it's a file)
  // Delete the directories and files that belong to this inode (if it's a directory)
  // TODO: Implement this

  // Remove from inode table
  this->inode_table[id].used = false;

  // Free the inode's block (delete from file system)
  bfs.reclaim_block(this->inode_table[id].inode.get_block_id());
}
bool FileSys::inode_is_used(short id)
{
  return this->inode_table[id].used;
}

short FileSys::pathname_to_inode_id(const char *name)
{
  Dirblock curr_inode = get_inode(HOME_INODE_ID);

  while (true)
  {
    curr_inode.get_magic()
  }
}