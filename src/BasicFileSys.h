// CPSC 3500: Basic File System
// Implements low-level file system functionality that interfaces with
// the disk.

#ifndef BASIC_FILESYS_H
#define BASIC_FILESYS_H

#include "Disk.h"

// Basic File 
class BasicFileSys {

  public:
    // Mounts the disk.  If the disk is new, it formats the disk by
    // initializing special blocks 0 (superblock) and 1 (root directory). 
    void mount();

    // Unmounts the disk.
    void unmount();

    // Gets a free block from the disk.
    short get_free_block();
  
    // Reclaims block making it available for future use.
    void reclaim_block(short block_num);

    // Reads block from disk. Output parameter block points to new block.
    void read_block(short block_num, void *block);
  
    // Writes block to disk. Input block points to block to write.
    void write_block(short block_num, void *block);

  private:
    Disk disk;
};

#endif
  
