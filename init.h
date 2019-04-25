/*********** init.h file ****************/
#ifndef _INITH_
#define _INITH_

#include "util.h"

//The main initialization function
//At this points it initializes:
    //2 PROCS's (P0 and P1)
    //minode[64] with all ref count = 0 
    //global root = 0
void init();

//Opens the device and verifies it's EX2FS
//Mounts root is so
//Also sets globals for:    
    //bmap, imap, and inodes_start
    //nblocks and ninodes
void mountRoot(char* diskName);

#endif