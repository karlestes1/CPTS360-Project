/*********** commands.h file ****************/
#ifndef _COMMANDSH_
#define _COMMANDSH_

#include "util.h"
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio_ext.h>
#include <unistd.h>

//Global Variables
extern int (*fptr[])(char*, char*);
extern char* cmds[];

//Commands

//Will search the cmds[] global list and see if the passed command matches
//If found, returns th index of that command, otherwise returns -1;
int findCommand(char* command);

void ls(char* path);

//Will look through a directory to read all of the files
//This function was adapted based on code from lab 4
void ls_dir(char* dirname);

//The following is used to print information about a single file
//It was adapted based on code from lab4
void ls_file(int ino, char* filename, char* dirname);

//Attempts to change the running processes cwd to the specified path
void cd(char* path);

//Prints the current working directory
//Calls a recursive helper function to do so
void pwd();

//Same as pwd() just doesn't print a newline at the end
void pwd_noNewline();

//Recursively traverses your working directory to print out your pwd
void rpwd(MINODE *wd);

//Creates a new directory on the disk
//Does so by calling mymkdir which actually allocates for the directory
void mk_dir(char* path);

//Does the actual allocation for a new directory on the disk
//Algorithm idea was taken from the lab assignment mkdir_creat for class
void mymkdir(MINODE* pip, char* name);

//Creates a new file on the disk
//Does so by calling mycreat which actually allocated for the new file
bool creat_file(char *path);

//Does the actual allocation for a new file on the disk
//Algorithm idea was taken from the lab assignment mkdir_creat for class
void mycreat(MINODE *pip, char* name);

//Does the actual allocation for a new link file on the disk
//Algorithm idea was taken from lab assignment mkdir_creat for class
void mycreat_symlink(MINODE *pip, char* name, char* oldname);

//Places the name of the newly created file into the parent directory's block
//Main algorithm idea was taken from the lab assignment mkdir_creat for class
void enter_name(MINODE *pip, int myino, char* myname);

//Removes a directory on the disk
//Calls myrmdir which actually removes the files
void rm_dir(char* path);

//Does the actually removal of the inode from the disk
//Calls rm_child() to remove child name from parent DIR
void myrmdir(MINODE *mip, char* path);

//Removes the inode entry (name) from parent inode data block
void rm_child(MINODE *pip, char* name);

//Removes a file from the disk
//Does so by calling myrm
void rm_file(char *path);

//Does the actually removal of the inode from the disk
//Calls rm_child() to remoe child name from the parent DIR
void myrm(MINODE *mip, char* path);

//Creates a hard link to the file
bool mylink(char* oldname, char* newname);

//Deletes a name from the file system
//If it is the last link to the file, it removes the file
bool myunlink(char* pathname);

//Creates a symbolic link to the passed file (oldname)
void mysymlink(char* oldname, char* newname);

//Prints the contents of the symlink file
void myreadlink(char* link);

//Prints the contents of the symlink file with no newline at the end
void myreadlink_nonewline(char* link);

//Works just as linux touch does
//Update access and modification time of the provided file
//If file does not exist, it will create the file
void my_touch(char* filename);

//Prints out information about a file
//Prints in the following way
    //File: 
    //Size:     Blocks:     IO Block:       $filetype
    //Device:   Inode:      Links:
    //Access:   Uid:        Gid:
    //Access:   $time
    //Modify:   $time
    //Change:   $time
void my_stat(char* filename);

//Changes the file mode bits
//Accepts ### filename
void my_chmod(char* mode, char* filename);

//Prints out the menu of all the commands
void printMenu();

//Opens a file with the specific mode
//Modes include R(0)|W(1)|RW(2)|APPEND(3)
    //Either the R|W|RW|APPEND or 0|1|2|3 may be used to denote mode
int my_open(char* mode, char* filename);

//If the passed file descriptor is open, closes the file
void my_close(char* fileDescriptor);

//Changes the offset of the file descriptor to a certain position
int my_lseek(char* fileDescriptor, char* position);

//Prints all currently open file descriptors, as well as mode, offset, and INODE
void pfd();

//Allows the user to test the read function
//Calls readFile() to do the actual reading
//Will break the reading down into calls of sizes no greater than BLKSIZE
void my_read(char* fileDescriptor, char* numBytes);

//Actually reads in the specified number of bytes into the file
int readFile(int fd, char *lbuf, int numBytes);

//Acts just as linux cat does
void my_cat(char* filename);

//Allows the user to test the write function
//calls writeFile() to do the actual writing
void my_write(char* fileDescriptor);

//Writes a specified number of bytes from lbuf to the file at fd
//Will only write if mode is proper
int writeFile(int fd, char lbuf[], int numBytes);

//Copies src into dest
//Creates dest if it does not already exist
bool my_cp(char* src, char* dest);

//Moves the file from src to destination
//Acts just at mv does on Linux
void my_mv(char* src, char* dest);

//Mounts a filesystem to a specific mount point
//If no filesystem is provided, lists the current mounted file system
void my_mount(char* filesystem, char* mountpoint);

//Unmounts a file system if it's no longer active
void my_umount(char* filesys);

//Quit the program
//iput() on all minodes with (refCount > 0 && DIRTY);
void quit();

//Personal functions to turn DEBUG global on or off
//debug_on() will mean console log messages will print
//debug_off() will mean no console log messages will print
void debug_on();
void debug_off();

#endif