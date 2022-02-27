// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#ifndef FILESYS_H
#define FILESYS_H

#include "BasicFileSys.h"

// OUR OWN IMPORTS
#include "Diamonds.h"
#include <array>

// In a world where all files are empty, this would mean that the only blocks
// on the disk are inode blocks. Therefore, the maximum number of inodes is
// technically NUM_BLOCKS - 1. The -1 is because the first block (index 0) is
// the super block (which doesn't have an inode).
const int MAX_INODES = NUM_BLOCKS - 1;
const short HOME_INODE_ID = 1;
struct inode_table_entry_t
{
  bool used;
  Inode inode;
  Dirblock dirblock;
};

class FileSys
{

public:
  // mounts the file system
  void mount(int sock);

  // unmounts the file system
  void unmount();

  // make a directory
  void mkdir(const char *name);

  // switch to a directory
  void cd(const char *name);

  // switch to home directory
  void home();

  // remove a directory
  void rmdir(const char *name);

  // list the contents of current directory
  void ls();

  // create an empty data file
  void create(const char *name);

  // append data to a data file
  void append(const char *name, const char *data);

  // display the contents of a data file
  void cat(const char *name);

  // display the first N bytes of the file
  void head(const char *name, unsigned int n);

  // delete a data file
  void rm(const char *name);

  // display stats about file or directory
  void stat(const char *name);

private:
  BasicFileSys bfs; // basic file system
  short curr_dir;   // current directory

  int fs_sock; // file server socket

  // ===========================================================================
  // Additional private variables and Helper functions - if desired

  array<inode_table_entry_t, MAX_INODES> inode_table;

  void init_inode_table();
  Inode create_inode();
  Inode get_inode(short id);
  void delete_inode(short id);
  bool inode_is_used(short id);

  short pathname_to_inode_id(const char *name);
};

#endif
