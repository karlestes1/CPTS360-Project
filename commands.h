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
void creat_file(char *path);

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
void mylink(char* oldname, char* newname);

//Deletes a name from the file system
//If it is the last link to the file, it removes the file
void myunlink(char* pathname);

//Creates a symbolic link to the passed file (oldname)
void mysymlink(char* oldname, char* newname);

//Prints the contents of the symlink file
void myreadlink(char* link);

//Prints the contents of the symlink file with no newline at the end
void myreadlink_nonewline(char* link);

//Prints out the menu of all the commands
void printMenu();

//Quit the program
//iput() on all minodes with (refCount > 0 && DIRTY);
void quit();

#endif