// This Diamonds file contains class wrappers for the extremely terrible and
// basic structs defined in "Blocks.h". (Diamonds are the superior blocks)

#include "Diamonds.h"

#include "Blocks.h"
#include <unistd.h>
#include <string>
#include <array>
using namespace std;

Superblock::Superblock() // Constructor
{
}

array<bool, BLOCK_SIZE> Superblock::get_bitmap() // Get entire bitmap
{
}
bool Superblock::get_bit(int index) // Get bit at index
{
}
void Superblock::set_bit(int index, bool value) // Set bit at index
{
}

Dirblock::Dirblock(unsigned int magic) // Constructor
{
}

unsigned int Dirblock::get_magic()
{ // gets magic number from dirblock_struct
}

unsigned int Dirblock::get_num_entries()
{
}

array<DirblockEntry, MAX_DIR_ENTRIES> Dirblock::get_dirblock_entries()
{
}

DirblockEntry Dirblock::get_dirblock_entry(int index)
{
}

void Dirblock::set_num_entries(unsigned int value)
{
}

DirblockEntry::DirblockEntry(string name, short block_num) // Constructor
{
}

string DirblockEntry::get_name()
{
}

short DirblockEntry::get_block_num()
{
}

DataBlock::DataBlock(array<char, BLOCK_SIZE>) // Constructor
{
}

array<char, BLOCK_SIZE> DataBlock::get_data() // gets data from block
{
}

void DataBlock::set_data(array<char, BLOCK_SIZE>)
{
}

Inode::Inode(unsigned int magic) // Constructor
{
}

unsigned int Inode::get_magic() // gets magic number from inode
{
}

unsigned int Inode::get_size() // gets size from inode
{
}

array<short, MAX_DATA_BLOCKS> Inode::get_blocks() // Get all blocks
{
}

short Inode::get_block(int index) // Get block at index
{
}

short Inode::add_block(DataBlock block)
{
}

void Inode::remove_block(int index)
{
}

void Inode::set_size(unsigned int size) // sets size of inode array
{
}