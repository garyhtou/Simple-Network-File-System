// See `WrappedFileSys.h` for more details regarding the purpose of this file.

#include "WrappedFileSys.h"

using namespace std;

// =============================================================================
// ------------------------------- BLOCK ---------------------------------------
// =============================================================================
template <typename T>
Block<T>::Block(short id) // Retrieve an existing block from disk
{
	this->id = id;

	// Read block from disk
	WrappedFileSys::bfs->read_block(this->id, (void *)&this->raw);
}
template <typename T>
Block<T>::Block() // Create a new block without data
{
	// Find a free block
	short id = WrappedFileSys::bfs->get_free_block();

	// Check return value of get_free_block
	if (id == DISK_FULL_BLOCK_ID)
	{
		throw WrappedFileSys::DiskFullException();
	}

	// Set block id
	this->id = id;
}

template <typename T>
T Block<T>::get_raw()
{
	return this->raw;
}

template <typename T>
void Block<T>::write_and_set_raw_block(T tempRaw)
{
	// Write block to disk
	WrappedFileSys::bfs->write_block(this->id, (void *)&tempRaw);

	// Update the class data member AFTER updating the disk
	this->raw = tempRaw;
}

// =============================================================================
// ----------------------------- DATA BLOCK ------------------------------------
// =============================================================================
DataBlock::DataBlock(short id) : Block<datablock_t>(id)
{
	// Deserialize data
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		this->data[i] = this->raw.data[i];
	}
}

DataBlock::DataBlock(array<char, BLOCK_SIZE> data) : Block<datablock_t>() // Create a new block from data
{
	this->set_data(data);
}

array<char, BLOCK_SIZE> DataBlock::get_data()
{
	return this->data;
}
void DataBlock::set_data(array<char, BLOCK_SIZE> data) // vector length can't be longer than BLOCK_SIZE
{
	// Serialize data to temporary struct
	struct datablock_t tempRaw;
	for (int i = 0; i < data.size(); i++)
	{
		tempRaw.data[i] = this->data[i];
	}

	// Write to disk and set class's raw
	this->write_and_set_raw_block(tempRaw);

	// Update the class data members AFTER updating the disk
	this->data = data;
}

// =============================================================================
// ------------------------------- INODE ---------------------------------------
// =============================================================================
template <typename T>
Inode<T>::Inode(short id) : Block<T>(id) // Retrieve an existing inode from disk
{
	// Deserialize data
	this->magic = this->raw.magic;
}

template <typename T>
Inode<T>::Inode() : Block<T>() // Create a new inode
{
	// Literally doesn't do anything besides call the parent constructor (`Block`)
	// to allocate a new block.
}

template <typename T>
unsigned int Inode<T>::get_magic()
{
	return this->magic;
}

template <typename T>
bool Inode<T>::is_dir()
{
	return this->magic == DIR_MAGIC_NUM;
}

template <typename T>
bool Inode<T>::is_file()
{
	return this->magic == INODE_MAGIC_NUM;
}

// =============================================================================
// ------------------------------- FILE INODE ----------------------------------
// =============================================================================
FileInode::FileInode(short id) : Inode(id) // Retrieve an existing file inode from disk
{
	// Validate that this is in fact a File Inode
	if (!this->is_file())
	{
		cerr << "ERROR (FileInode::FileInode): Inode is not a file inode." << endl;
		exit(1);
	}

	// Deserialize data
	this->magic = this->raw.magic;
	this->size = this->raw.size;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		short block_id = this->raw.blocks[i];
		if (block_id != UNUSED_ID)
		{
			this->blocks.push_back(Block(block_id));
		}
	}
}

FileInode::FileInode() : Inode<inode_t>() // Create a new file inode
{
	// Set up data in a temporary struct
	struct inode_t tempRaw;
	tempRaw.magic = INODE_MAGIC_NUM;
	tempRaw.size = 0;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		tempRaw.blocks[i] = 0;
	}

	// Write to disk and set class's raw
	this->write_and_set_raw_block(tempRaw);

	// Update the class data members AFTER updating the disk
	this->magic = tempRaw.magic;
	this->size = tempRaw.size;
	// this->blocks should already be empty
}

// =============================================================================
// ------------------------------- DIRECTORY INODE -----------------------------
// =============================================================================
DirInode::DirInode(short id) : Inode(id) // Retrieve an existing directory inode from disk
{
	// Validate that this is in fact a Dir Inode
	if (!this->is_dir())
	{
		cerr << "ERROR (DirInode::DirInode): Inode is not a directory inode." << endl;
		exit(1);
	}

	// Deserialize data
	this->magic = this->raw.magic;
	this->num_entries = this->raw.num_entries;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		char *name = this->raw.dir_entries[i].name;
		short block_id = this->raw.dir_entries[i].block_num;

		// Skip block entires that unused
		if (block_id == UNUSED_ID)
		{
			continue;
		}

		// Determine type of Inode (file or directory)
		Inode inode = Inode(block_id);
		if (inode.is_file())
		{
			FileInode file_inode(block_id);
			DirEntry<FileInode> entry(name, file_inode);
			this->file_entries.push_back(entry);
		}
		else if (inode.is_dir())
		{
			DirInode dir_inode(block_id);
			DirEntry<DirInode> entry(name, dir_inode);
			this->dir_entries.push_back(entry);
		}
		else
		{
			cerr << "ERROR (DirInode::DirInode): Inode is not a file or directory inode." << endl;

			// Assume it's a file inode
			FileInode file_inode(block_id);
			DirEntry<FileInode> entry(name, file_inode);
			this->file_entries.push_back(entry);
		}
	}
}

DirInode::DirInode() : Inode() // Create a new directory inode
{
}