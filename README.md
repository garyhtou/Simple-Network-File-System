# 📂 Simple Network File System

## About

This **client-server network file system** is built on top of a provided virtual
disk featuring an **indexed block allocation** approach. The disk includes a
**Superblock**, **Free block bitmap**, **Inodes** (for files and directories),
and **Datablocks**. The client and server communicate over a **persistent TCP
connection**.

### Support commands
- `ls`:	List the contents of the current directory
- `cd <directory>`: Change to a specified directory
- `home`: Switch to the home (root) directory (similar to `cd /` in Unix)
- `rmdir <directory>`: Remove a directory. The directory must be empty
- `create <filename>`: Create an empty file
- `append <filename> <data>`: Append data to an existing file
- `stat <name>`: Display information for a given file or directory
- `cat <filename>`: Display the contents of a file
- `head <filename> <n>`: Display the first `n` bytes of the file
- `rm <filename>`: Remove a file

<sub>More information can be found [here](/assignment/Project4_NFS.pdf.pdf).</sub>

## Meet the team

**Team member #1: Gary Tou** ([@garyhtou](https://github.com/garyhtou))
- Creating and connecting TCP socket (Shell)
- Executing commands (Shell)
- home, append, rm (FileSys)
- Receiving messages for Client (Helper)
- Object Orientated wrapped classes for the BasicFileSys (WrappedFileSys)

**Team member #2: Harry Rudolph** ([@hankrud](https://github.com/HankRud))
- Parsing the command line (Shell)
- Formatting and outputting command messages (Shell)
- mkdir, cd, stat, rmdir in (FileSys)
- Receiving messages for Server (Helper)
- Message formatting

**Team member #3: Castel Villalobos** ([@impropernoun](https://github.com/impropernoun))
- Remote procedure call commands (Shell)
- ls, create, head, cat (FileSys)
- Parsing and executing (server)
- Sending messages (Helper)
- Error Handling
