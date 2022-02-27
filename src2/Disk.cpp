// CPSC 3500:  A "virtual" Disk
// This implements a simulated disk consisting of an array of blocks.

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
using namespace std;

#include "Disk.h"
#include "Blocks.h"

// Opens the file "file_name" that represents the disk.  If the file does
// not exist, file is created. Returns true if a file is created and false if
// the file parameter fd exists. Any other error aborts the program.
bool Disk::mount(const char *file_name)
{
  fd = open(file_name, O_RDWR);
  if (fd != -1) return false;

  fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    cerr << "Could not create disk" << endl;
    exit(-1);
  }

  return true;
}

// Closes the file descriptor that represents the disk.
void Disk::unmount()
{
  close(fd);
}
  
// Reads disk block block_num from the disk into block.
void Disk::read_block(int block_num, void *block)
{
  off_t offset;
  off_t new_offset;
  ssize_t size; 

  if (block_num < 0 || block_num >= NUM_BLOCKS) {
    cerr << "Invalid block size" << endl;
    exit(-1);
  }

  offset = block_num * BLOCK_SIZE;
  new_offset = lseek(fd, offset, SEEK_SET);
  if (offset != new_offset) {
    cerr << "Seek failure" << endl;
    exit(-1);
  }

  size = read(fd, block, BLOCK_SIZE);
  if (size != BLOCK_SIZE) {
    cerr << "Failed to read entire block" << endl;
    exit(-1);
  }
}

// Writes the data in block to disk block block_num.
void Disk::write_block(int block_num, void *block)
{
  off_t offset;
  off_t new_offset;
  ssize_t size; 

  if (block_num < 0 || block_num >= NUM_BLOCKS) {
    cerr << "Invalid block size" << endl;
    exit(-1);
  }

  offset = block_num * BLOCK_SIZE;
  new_offset = lseek(fd, offset, SEEK_SET);
  if (offset != new_offset) {
    cerr << "Seek failure" << endl;
    exit(-1);
  }

  size = write(fd, block, BLOCK_SIZE);
  if (size != BLOCK_SIZE) {
    cerr << "Failed to write entire block" << endl;
    exit(-1);
  }
}
