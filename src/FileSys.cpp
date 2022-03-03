// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

// TODO: REMOVE DEBUG STUFF
bool DEBUG = true;
// TODO: REMOVE DEBUG STUFF

#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

#include "WrappedFileSys.h"
#include <math.h>

// Forward declare functions
void validate_before_new_entry(DirInode dir, string name);

// mounts the file system
void FileSys::mount(int sock)
{
  bfs.mount();
  curr_dir = 1;   // by default current directory is home directory, in disk block #1
  fs_sock = sock; // use this socket to receive file system operations from the client and send back response messages

  // Set the WrapperFileSys's BFS instance
  WrappedFileSys::bfs = &bfs;

  // Default working directory to Home Directory
  this->set_working_dir(DirInode(HOME_DIR_ID));
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

  // Will error if dir is full or if name is invalid (too long)
  validate_before_new_entry(working_dir, dir_name);

  // Attempt to create a block for this new directory
  DirInode new_dir = DirInode(); // May throw WrappedFileSys::DiskFullException();

  // Add the new DirNode as an entry to the current working directory
  DirEntry<DirInode> entry = DirEntry<DirInode>(dir_name, new_dir);
  working_dir.add_entry(entry);
  this->response_ok();
}

// switch to a directory
// Should raise 500, 503
void FileSys::cd(const char *name)
{
  string dir_name = name;

  // Retreive working directory
  DirInode working_dir = this->get_working_dir();

  // Search directory until entry name match
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    if (entry.get_name() == name)
    {
      // Found the directory
      this->set_working_dir(entry.get_inode());

      this->response_ok();
      return;
    }
  }

  // Directory was not found, see if the name is of a file
  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
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
  this->response_ok();
}

// remove a directory
// Should raise 500, 503, 507
void FileSys::rmdir(const char *name)
{
  DirInode working_dir = this->get_working_dir();
  string dir_name = name;

  // check if this is a file
  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
  {
    if (entry.get_name() == dir_name)
    {
      throw WrappedFileSys::NotADirException();
    }
  }

  // check that directory exists
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    if (entry.get_name() == dir_name)
    {
      DirInode dir = entry.get_inode();
      // check that directory is empty
      if (dir.get_num_entries() > 0)
      {
        throw WrappedFileSys::DirNotEmptyException();
      }

      // delete entry
      working_dir.remove_entry(entry);

      // destroy directory
      dir.destroy();

      this->response_ok();
      return;
    }
  }

  throw WrappedFileSys::FileNotFoundException();
}

// list the contents of current directory
void FileSys::ls()
{
  DirInode working_dir = this->get_working_dir();

  // Check if the directory is empty (no dir or file entries).
  if ((working_dir.get_dir_inode_entries().size() +
       working_dir.get_file_inode_entires().size()) == 0)
  {
    response_ok("empty folder");
    return;
  }

  // Add all names to a vector for sorting alphabetically
  vector<string> names;
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    names.push_back(entry.get_name());
  }
  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
  {
    names.push_back(entry.get_name());
  }

  // Alphabetically sort the names
  sort(names.begin(), names.end(),
       [](string a, string b)
       { return a < b; });

  // Join vector strings to a single string
  string response;
  for (int i = 0; i < names.size(); i++)
  {
    response.append(names.at(i));

    // Add whitespace delimiter if it's not the last name
    if (i != names.size() - 1)
    {
      response.append(" ");
    }
  }

  this->response_ok(response);
}

// create an empty data file
// Should raise 502, 504, 505, 506
void FileSys::create(const char *name)
{
  string file_name = name;
  DirInode working_dir = this->get_working_dir();

  // Will error if dir is full or if name is invalid (too long)
  validate_before_new_entry(working_dir, file_name);

  // Attempt to create the file
  FileInode new_file = FileInode();

  // Add the new file as an entry to the working directory
  DirEntry<FileInode> new_entry = DirEntry<FileInode>(file_name, new_file);
  working_dir.add_entry(new_entry);

  this->response_ok();
}

// append data to a data file
// Should raise 501, 503, 505, 508
void FileSys::append(const char *name, const char *data)
{
  string file_name = name;
  string data_str = data;
  DirInode working_dir = this->get_working_dir();

  // See if the name exists with a directory
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    if (entry.get_name() == name)
    {
      throw WrappedFileSys::NotAFileException();
    }
  }

  // Try to find the file
  bool found = false;
  short file_id;
  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
  {
    if (entry.get_name() == name)
    {
      file_id = entry.get_inode().get_id();
      found = true;
    }
  }

  if (!found)
  {
    throw WrappedFileSys::FileNotFoundException();
  }
  FileInode file = FileInode(file_id);

  // Calculate if we will exceed max file size (if there will be sufficent
  // datablock pointers to hold the new data)
  int new_total_size = file.get_size() + data_str.size();
  int new_total_blocks = ceil(double(new_total_size) / BLOCK_SIZE);
  if (new_total_blocks > MAX_DATA_BLOCKS)
  {
    // The total number of datablocks needed to hold the file data exceeds the
    // max number of datablocks a file is capability of holding.
    throw WrappedFileSys::FileFullException();
  }

  // Determine is we need to update the last existing block (this is necessary
  // if the last block still has unused space).
  unsigned int frag_size = file.internal_frag_size();
  bool has_frag = frag_size > 0;

  DataBlock *fragmented_block;
  array<char, BLOCK_SIZE> fragmented_block_data = {};
  if (has_frag)
  {
    // If there is internal fragmentation, it will always be located in the
    // last data block.
    fragmented_block = &file.get_blocks().at(-1);
    // The fragmented block's data is stored in case we need to rollback the
    // transaction (more info about the transaction below).
    fragmented_block_data = fragmented_block->get_data();
  }

  // Start organizing data (with consideration of fragmentation)
  vector<array<char, BLOCK_SIZE>> new_data_vec;

  string new_data_str;
  if (has_frag)
  {
    // Include the data in the fragmented block in the new data (the existing
    // fragmented block will be overriden)
    for (int i = 0; i < frag_size; i++)
    {
      new_data_str += fragmented_block->get_data().at(i);
    }
  }
  new_data_str += data_str;

  // Convert the data string into block sized char arrays
  for (int i = 0; i < new_data_str.length(); i += BLOCK_SIZE)
  {
    string curr_data = new_data_str.substr(i, BLOCK_SIZE);
    array<char, BLOCK_SIZE> curr_data_block = {};

    for (int j = 0; j < curr_data.size(); j++)
    {
      curr_data_block[j] = curr_data.at(j);
    }

    new_data_vec.push_back(curr_data_block);
  }

  // Everything seems good to go — however, there is one error that is still
  // unchecked. We don't know if the disk has enough storage (free blocks).
  // According to the instructions, the disk must remain unchanged in the case
  // of an error. This means that if there are not enough free blocks on the
  // disk to hold the new data, then the disk must not have been modified
  // (atomic operation — all or none).
  // There is no way of knowing how many free disk blocks are available without
  // directly accessing the superblock. Our solution to keep this operation
  // atomic will be to create a transaction. If the disk is full while trying to
  // create a new block, then rollback all changes.

  // ========== TRANSACTION ==========
  vector<DataBlock> new_datablocks;
  try
  {
    for (int i = 0; i < new_data_vec.size(); i++)
    {
      if (i == 0 && has_frag)
      {
        // We need to override the existing block
        fragmented_block->set_data(new_data_vec.at(i));
        continue;
      }

      // Attempt to create a new datablock. Will error if disk runs out to free
      // blocks.
      DataBlock new_datablock = DataBlock(new_data_vec.at(i));
      // Add the new datablock to the vector in case we need to rollback the
      // transaction (undo allocation of this block).
      new_datablocks.push_back(new_datablock);

      // Add the new datablock to the file
      unsigned int new_data_size = (i != new_data_vec.size() - 1) ? BLOCK_SIZE : new_data_str.size();
      file.add_block(new_datablock, new_data_size);
    }

    // Looks like everything worked! The disk had enough free blocks to append
    // the new data.
    this->response_ok();
  }
  catch (const WrappedFileSys::DiskFullException &e)
  {
    // Oh yikes, looks like the disk ran out of space. We now need to rollback
    // this transaction. Two things need to happen:
    //   1. If there was a fragmented datablock, then undo the data override
    //      that we performed earlier.
    //   2. Deallocate (reclaim) the new datablocks that we allocated.
    if (has_frag)
    {
      fragmented_block->set_data(fragmented_block_data);
    }

    for (DataBlock datablock : new_data_vec)
    {
      datablock.destroy();
    }

    // Let the client know that the disk ran out of space
    throw WrappedFileSys::DiskFullException();
  }
}

// display the contents of a data file
// Should raise 501, 503
void FileSys::cat(const char *name)
{
  const unsigned int MAX_FILE_SIZE = MAX_DATA_BLOCKS * BLOCK_SIZE;
  this->head(name, MAX_FILE_SIZE);
}

// display the first N bytes of the file
// Should raise 501, 503
void FileSys::head(const char *name, unsigned int n)
{
  string file_name = name;
  DirInode working_dir = this->get_working_dir();

  // loop through all dir entries
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    if (entry.get_name() == name)
    {
      throw WrappedFileSys::NotAFileException(); // 501
    }
  }

  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
  {
    if (entry.get_name() == name)
    {
      // read file data
      FileInode file = entry.get_inode();
      unsigned int size_to_get = min(file.get_size(), n); // TODO: what should we do if `n < 0`

      int num_blocks_to_get = floor(size_to_get / BLOCK_SIZE);
      int additional_bytes_to_get = size_to_get % BLOCK_SIZE;

      if (num_blocks_to_get > file.get_blocks().size())
      {
        cerr << "FileSys::head num_blocks_to_get > file.get_blocks().size()" << endl;
      }

      string response;
      for (int i = 0; i < num_blocks_to_get; i++)
      {
        DataBlock datablock = file.get_blocks().at(i);
        for (char datum : datablock.get_data())
        {
          response += datum;
        }
      };

      DataBlock datablock = file.get_blocks().at(num_blocks_to_get);
      for (int i = 0; i < additional_bytes_to_get; i++)
      {
        response += datablock.get_data().at(i);
      }

      response_ok(response);
      return;
    }
  }
  // file does not exist
  throw WrappedFileSys::FileNotFoundException(); // 503
}

// delete a data file
// Should raise 501, 503
void FileSys::rm(const char *name)
{
  // Aquire working directory
  DirInode working_dir = this->get_working_dir();

  // Loop thru all dir entries
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    if (entry.get_name() == name)
    {
      if (DEBUG)
      {
        cout << "DEBUG: rm Throwing Not a file exception" << endl;
      }
      throw WrappedFileSys::NotAFileException();
    }
  }

  if (DEBUG)
  {
    cout << "DEBUG: rm (entering file name match loop )" << endl;
  }

  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
  {
    if (entry.get_name() == name)
    {
      FileInode file = entry.get_inode();
      vector<DataBlock> datablocks = file.get_blocks();

      // Delete Entry
      if (DEBUG)
      {
        cout << "DEBUG: rm (deleting entry))" << endl;
      }
      working_dir.remove_entry(entry);

      // Delete Inode
      if (DEBUG)
      {
        cout << "DEBUG: rm (deleting Inode))" << endl;
      }
      file.destroy();

      // Delete datablocks
      if (DEBUG)
      {
        cout << "DEBUG: rm (deleting datablocks))" << endl;
      }
      for (DataBlock db : datablocks)
      {
        db.destroy();
      }

      response_ok("success");
      return;
    }
  }

  // file not found
  throw WrappedFileSys::FileNotFoundException();
}

// display stats about file or directory
// Should raise 503
void FileSys::stat(const char *name)
{
  // get working directory
  string dir_name = name;
  DirInode working_dir = this->get_working_dir();

  // search directory until name found
  if (DEBUG)
  {
    cout << "DEBUG: stat (searching directory entries ))" << endl;
  }
  for (DirEntry<DirInode> entry : working_dir.get_dir_inode_entries())
  {
    // if found determine type
    if (entry.get_name() == name)
    {
      // create directory node to pass
      DirInode dir = entry.get_inode();

      string message; // c++ string to hand to response
      message.append("Directory name: ");
      message.append(dir_name + '\n');
      message.append("Directory block: ");
      message.append(to_string(dir.get_id()) + '\n');
      response_ok(message);
      return;
    }
  }
  // search file enteries
  if (DEBUG)
  {
    cout << "DEBUG: stat (searching file entries ))" << endl;
  }
  for (DirEntry<FileInode> entry : working_dir.get_file_inode_entires())
  {
    // create file inode
    FileInode file = entry.get_inode();

    string message;                               // make a c++ string to give response
    vector<DataBlock> blocks = file.get_blocks(); // vector of blocks we will need info about;
    message.append("Inode block: ");              // inode block
    message.append(to_string(file.get_id()) + '\n');
    message.append("Bytes in file: "); /// Bytes in file
    message.append(to_string(file.get_size()) + '\n');
    message.append("Number of blocks: "); // Number of blocks
    message.append(to_string(blocks.size()) + '\n');
    message.append("First block: "); // First block
    message.append(to_string(blocks[0].get_id()) + '\n');
    response_ok(message);
    return;
  }

  // file/directory not found throw error
  throw WrappedFileSys::FileNotFoundException();
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

void FileSys::response_ok(string message)
{
  if (DEBUG)
  {
    if (message.size() > 0)
    {
      cout << "[DEBUG] (FileSys::response_ok) message = " << message << endl;
    }
    else
    {
      cout << "[DEBUG] (FileSys::response_ok) with no message." << endl;
    }
  }

  // Used for testing. TODO: remove
  DEBUG_LAST_RESPONSE_MESSAGE = message;

  // TODO: format data and send to client via socket
}

void validate_before_new_entry(DirInode dir, string name)
{
  // Check if the given name is too long
  if (name.size() > MAX_FNAME_SIZE)
  {
    throw WrappedFileSys::FileNameTooLongException();
  }

  // Check if the directory has space for another entry
  if (!dir.has_free_entry())
  {
    throw WrappedFileSys::DirFullException();
  }

  // Check if a directory or file under this name already exists
  for (DirEntry<DirInode> entry : dir.get_dir_inode_entries())
  {
    if (entry.get_name() == name)
    {
      throw WrappedFileSys::FileExistsException();
    }
  }
  for (DirEntry<FileInode> entry : dir.get_file_inode_entires())
  {
    if (entry.get_name() == name)
    {
      throw WrappedFileSys::FileExistsException();
    }
  }
}
