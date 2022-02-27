// Wrapped File System provides wrapper classes on top of the provided structs
// and functions provided by `blocks.h` and `BasicFileSys`.
//
// The goal is to provide an object oriented interface to the underlying
// C-like architecture. Wrapped File System will also handle error checking
// using custom exceptions.

#ifndef WRAPPEDFILESYS_H
#define WRAPPEDFILESYS_H

#include "Blocks.h"
#include "BasicFileSys.h"

#include <unistd.h>
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

const short UNUSED_ID = 0;
const short DISK_FULL_BLOCK_ID = 0;

namespace WrappedFileSys
{
	// This BasicFileSys instance is shared by all the wrapper classes. It must be
	// initialized before any wrapper classes are created/used.
	extern BasicFileSys *bfs;

	// Custom exceptions
	class FileSystemException : public exception
	{
		const char *what() const throw()
		{
			return "File system error";
		}
	};

	class DiskFullException : public FileSystemException
	{
		const char *what() const throw()
		{
			return "Disk full";
		}
	};

	class FileFullException : public FileSystemException
	{
		const char *what() const throw()
		{
			return "File full";
		}
	};

	class DirFullException : public FileSystemException
	{
		const char *what() const throw()
		{
			return "Directory full";
		}
	};
};

// =============================================================================
// No class wrapper is needed for the superblock as none of our code will need
// to interact directly with it.

// =============================================================================
// -------------------------------- BLOCK --------------------------------------
// =============================================================================
// `Block` is an fairly abstract class meant to be used by it's sub classes for
// reading and writing "raw" structs from and to the disk. It is a class
// template to allow for different types of "raw" struct (e.g. `inode_t`).
template <typename T>
class Block
{
public:
	Block(short id); // Retrieve an existing block from disk
	Block();				 // Create a new block without data

	T get_raw();
	short get_id();

protected:
	short id; // This is the index of the block on disk

	T raw;
	void write_and_set_raw_block(T block);
};

// =============================================================================
// ----------------------------- DATA BLOCK ------------------------------------
// =============================================================================
class DataBlock : public Block<datablock_t>
{
public:
	DataBlock(short id);										 // Retrieve an existing data block from disk
	DataBlock(array<char, BLOCK_SIZE> data); // Create a new block from data
	// There is no constructor to create an empty DataBlock because datablocks
	// should NEVER be empty.

	array<char, BLOCK_SIZE> get_data();
	void set_data(array<char, BLOCK_SIZE> data); // vector length can't be longer than BLOCK_SIZE

	using Block<datablock_t>::get_raw;
	using Block<datablock_t>::get_id;

protected:
	// Block uses an array instead of vector because there is no way to tell the
	// data size from the datablock_t struct (some bits of the data block may be
	// unused).
	array<char, BLOCK_SIZE> data;
};

// =============================================================================
// ------------------------------- INODE ---------------------------------------
// =============================================================================
// `Inode` is a class template to allow for different types inode "raw" structs.
// For example, `inode_t` and `dirblock_t` which represent a file inode and a
// directory inode respectively.
// `Inode` shouldn't really ever be used directly besides to check for the type
// for a given inode using `is_dir()`.
template <typename T>
class Inode : protected Block<T>
{
public:
	Inode(short id); // Retrieve an existing inode from disk
	Inode();				 // Should never be used directly

	unsigned int get_magic();

	bool is_dir();
	bool is_file();

	using Block<T>::get_raw;
	using Block<T>::get_id;

protected:
	unsigned int magic;
};

// =============================================================================
// ----------------------------- FILE INODE ------------------------------------
// =============================================================================
class FileInode : public Inode<inode_t>
{
public:
	FileInode(short id); // Retrieve an existing file inode from disk
	FileInode();				 // Create a new file inode

	unsigned int get_size();

	vector<DataBlock> get_blocks();
	void add_block(DataBlock block, unsigned int size);
	void remove_block(DataBlock block, unsigned int size);

	unsigned int internal_frag_size();

protected:
	unsigned int size;
	vector<DataBlock> blocks;
};

// =============================================================================
// ----------------------------- DIRECTORY INODE -------------------------------
// =============================================================================

// Forward declaration for `DirEntry`
template <typename T>
class DirEntry;

class DirInode : public Inode<dirblock_t>
{
public:
	DirInode(short id); // Retrieve an existing directory inode from disk
	DirInode();					// Create a new directory inode

	unsigned int get_num_entries();

	vector<DirEntry<FileInode>> get_file_entries();
	vector<DirEntry<DirInode>> get_dir_entries();

	void add_entry(DirEntry<FileInode> entry);
	void add_entry(DirEntry<DirInode> entry);

	// TODO: remove_entry
	// "rmdir" Removes a subdirectory. The subdirectory must be empty.
	// "rm" Remove a file from the directory, reclaim all of its blocks including its inode. Cannot remove directories.

protected:
	// File and Dir Inodes are stored separately since vectors can only hold a
	// single type (even if they have a common base class). This means there are
	// also different functions for getting and adding entires.
	vector<DirEntry<FileInode>> file_entries;
	vector<DirEntry<DirInode>> dir_entries;

	unsigned int num_entries;

private:
	template <typename T>
	void add_entry_base(DirEntry<T> entry);
};

// =====================
// ----- DIR ENTRY -----
// =====================
// Dir entries are stored within a directory inode and serve as pointers to
// other inodes (either file or directory). In order to use this `DirEntry`
// class, you must specify the type of the inode that the entry points to.
//
// Usage example:
//   DirInode home = DirInode(1);
//   FileInode file = FileInode(2);
//   DirEntry<FileInode> file_entry(home, file);
template <typename T>
class DirEntry
{
	// static_assert(is_base_of<Inode<T>, T>::value, "DirEntry must be used with an Inode");

public:
	DirEntry(string name, T inode);

	string get_name();
	T get_inode();

protected:
	string name;
	T inode;
};

#endif