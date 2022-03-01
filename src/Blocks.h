// CPSC 3500: Blocks that make up the filesystem

#ifndef BLOCKS_H
#define BLOCKS_H

// CONSTANTS

// Size of block - must be an even power of two 
const int BLOCK_SIZE = 128;

// Number of blocks - set so a bitmap can fit in one block
const int NUM_BLOCKS = (BLOCK_SIZE * 8);

// Maximum filename size
const int MAX_FNAME_SIZE = 9;

// Maximum number of files in a directory
const int MAX_DIR_ENTRIES = ((BLOCK_SIZE - 8) / 12);

// Maximum number of blocks in a data file
const int MAX_DATA_BLOCKS = ((BLOCK_SIZE - 8) / 2);

// Maximum file size for a data file
const int MAX_FILE_SIZE	= (MAX_DATA_BLOCKS * BLOCK_SIZE);

// Magic numbers - used to distinguish between directory blocks and inodes
const unsigned int DIR_MAGIC_NUM = 0xFFFFFFFF;
const unsigned int INODE_MAGIC_NUM = 0xFFFFFFFE;

// BLOCK TYPES

// Superblock - keeps track of which blocks are used in the filesystem.
// Block 0 is the only super block in the system.
struct superblock_t {
  unsigned char bitmap[BLOCK_SIZE]; // bitmap of free blocks
};

// Directory block - represents a directory
struct dirblock_t {
  unsigned int magic;		// magic number, must be DIR_MAGIC_NUM
  unsigned int num_entries;	// number of files in directory
  struct {
    char name[MAX_FNAME_SIZE + 1]; // file name (extra space for null)
    short block_num;		   // block number of file (0 - unused)
  } dir_entries[MAX_DIR_ENTRIES];  // list of directory entries
};

// Inode - index node for a data file
struct inode_t {
  unsigned int magic;		 // magic number, must be INODE_MAGIC_NUM
  unsigned int size;		 // file size in bytes
  short blocks[MAX_DATA_BLOCKS]; // array of direct indices to data blocks
};

// Data block - stores data for a data file
struct datablock_t {
  char data[BLOCK_SIZE];	// data (BLOCK_SIZE bytes)
};

#endif

