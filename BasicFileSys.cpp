// CPSC 3500: Basic File System
// Implements low-level file system functionality that interfaces with
// the disk.

#include "Disk.h"
#include "Blocks.h"
#include "BasicFileSys.h"

// Mounts the simulated disk file. If a disk file is created, this
// routines also "formats" the disk by initializing special blocks
// 0 (superblock) and 1 (root directory).
void BasicFileSys::mount()
{
  // mount the disk
  bool new_disk = disk.mount("DISK");

  // if the disk exists, return as no further initialization is needed
  if (!new_disk) return;

  // initialize the superblock
  struct superblock_t super_block;
  super_block.bitmap[0] = 0x3;		// mark blocks 0 and 1 as used
  for (int i = 1; i < BLOCK_SIZE; i++) {
    super_block.bitmap[i] = 0;
  }
  disk.write_block(0, (void *) &super_block);

  // initialize the root directory
  struct dirblock_t dir_block;
  dir_block.magic = DIR_MAGIC_NUM;
  dir_block.num_entries = 0;
  for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
    dir_block.dir_entries[i].block_num = 0;
  }
  disk.write_block(1, (void *) &dir_block);

  // write a zeroed-out data block to all other blocks on disk
  struct datablock_t data_block;
  for (int i = 0; i < BLOCK_SIZE; i++) {
    data_block.data[i] = 0;
  }
  for (int i = 2; i < NUM_BLOCKS; i++) {
    disk.write_block(i, (void *) &data_block);
  }
}

// Unmounts the disk
void BasicFileSys::unmount()
{
  disk.unmount();
}

// Gets a free block from the disk.
short BasicFileSys::get_free_block()
{
  // get superblock
  struct superblock_t super_block;
  disk.read_block(0, (void *) &super_block);
  
  // look for first available block
  for (int byte = 0; byte < BLOCK_SIZE; byte++) {

    // check to see if byte has available slot
    if (super_block.bitmap[byte] != 0xFF) {
 
      // loop to check each bit
      for (int bit = 0; bit < 8; bit++) {
        int mask = 1 << bit;
	if (mask & ~super_block.bitmap[byte]) {

          // Available block is found: set bit in bitmap, write result back
	  // to superblock, and return block number.
	  super_block.bitmap[byte] |= mask;
	  disk.write_block(0, (void *) &super_block);
	  return (byte * 8) + bit;
	}
      }
    }
  }

  // disk is full
  return 0;
}
  
// Reclaims block making it available for future use.
void BasicFileSys::reclaim_block(short block_num)
{
  // get superblock
  struct superblock_t super_block;
  disk.read_block(0, (void *) &super_block);

  // clear bit
  int byte = block_num / 8;		// byte number
  int bit = block_num % 8;		// bit number
  unsigned char mask = ~(1 << bit);	// mask to clear bit
  super_block.bitmap[byte] &= mask;

  // write back superblock
  disk.write_block(0, (void *) &super_block);
}
  
// Reads block from disk. Output parameter block points to new block.
void BasicFileSys::read_block(short block_num, void *block) {
  disk.read_block(block_num, block);
}

// Writes block to disk. Input block points to block to write.
void BasicFileSys::write_block(short block_num, void *block) {
  disk.write_block(block_num, block);
}
