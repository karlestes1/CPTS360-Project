/*********** util.h file ****************/
//The code for this file was provided by KC Wang as part of the asignment
//I defined tokenize() and search()
//I added the findCmd() functionality
#include "type.h"
#ifndef _UTILH_
#define _UTILH_

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    nname;
extern int    fd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[256];
extern char   buf[BLKSIZE];
extern char *GREEN, *NORMAL, *RED, *LAVENDER, *PURPLE, *DARKBLUE, *BLUE, *YELLOW, *BRIGHT;
extern bool DEBUG; //Used to turn debugging on and off

//Gets the data block from the device at the specified blk number
//Returns 0 if block  cannot be grabbed
int get_block(int dev, int blk, char *buf);

//Puts the passed buffer into the specified blk number on the provided dev
//Returns 0 if the block cannot be placed
int put_block(int dev, int blk, char *buf);

//Tests wether a bit is 0 or 1
//Returns 0 or 1 respectively
int tst_bit(char *pbuf, int bit);

//Sets the specified bit in the buffer to 1
int set_bit(char *pbuf, int bit);

//Sets the specified bit in the buffer to 2
int clr_bit(char *pbuf, int bit);

//Allocates a new inode in the inode_bitmap
//Returns 0 if all the inodes are full
int ialloc(int dev);

//Allocates a new data block in the block_bitmap
//Returns 0 if all of the data blocks are full
int balloc(int dev);

//Deallocates the specified inode in the inode_bitmap
void idealloc(int dev, int ino);

//Deallocates the specified data block in the block_bitmap
void bdealloc(int dev, int blkno);

// tokenize pathname in GLOBAL gpath[]
//The algorithm for this function was taken from page 325 of the textbook for 360
int tokenize(char *path);

//Loads a specified ino into MINODE table and returns a pointer to loaded MINODE
MINODE *iget(int dev, int ino);

//Writes the MINODE back to the disk
void iput(MINODE *mip);

//Search through a series of inodes for one with a name matching the passed paramter
//This code is based off lab6 and the search function on page 325 of the 360 textbook
//Returns inode number if found, else 0
int search(MINODE *mip, char *name);

//Searches the provided path for a DIR/FILE matching the name
//Returns ino number of DIR/FILE if found, otherwise returns 0
int getino(char *path);

//Deallocates all of the data blocks of INODE
void mytruncate(MINODE* mip);

//Loops through all possible processes and closes any open files
//Deallocates the allocated OFT
void closeAllFiles();

#endif