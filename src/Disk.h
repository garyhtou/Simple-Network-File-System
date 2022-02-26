// CPSC 3500: A "virtual" Disk
// This implements a simulated disk consisting of an array of blocks.

#ifndef DISK_H
#define DISK_H

class Disk {

  public:
    // Opens the file "file_name" that represents the disk.  If the file does
    // not exist, file is created. Returns true if a file is created and false if
    // the file parameter fd exists. Any other error aborts the program.
    bool mount(const char *filename);

    // Closes the file descriptor that represents the disk.
    void unmount();
  
    // Reads disk block block_num from the disk into block.
    void read_block(int block_num, void *block);

    // Writes the data in block to disk block block_num.
    void write_block(int block_num, void *block);

  private:
    int fd;	// file descriptor that represents the disk
};

#endif
