/*************** type.h file ************************/
#ifndef _TYPEH_
#define _TYPEH_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;  

#define BLKSIZE           1024
#define ISIZE              128

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))


// Table sizes
#define NMINODE          64
#define NMOUNT            4
#define NPROC             2
#define NFD              16
#define NOFT             32

// Default dir, regular file, and link modes
#define DIR_MODE      0x41ED 
#define FILE_MODE     0x81A4
#define LINK_MODE     0xA194

#define EXT2_FS       0xEF53   

// Proc status
#define FREE              0
#define BUSY              1
#define READY             2

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  // for level-3
  int mounted;
  struct mount *mptr;
}MINODE;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  int          pid;
  int          uid;
  int          gid;
  int          status;

  MINODE      *cwd;
  OFT         *fd[NFD];

  struct proc *next;
  struct Proc *parent;
  struct Proc *child;
  struct Proc *sibling;
}PROC;

// Mount Table structure
typedef struct mount{
        int    dev;   
        int    ninodes;
        int    nblocks;
        int    imap, bmap, iblk; 
        MINODE *mounted_inode;
        char   name[256]; 
        char   mount_name[64];
} MOUNT;

#endif
