/*********** util.c file ****************/
//The code for this file was provided by KC wang as part of the assignment
//I defined tokenize() and search()
#include "util.h"

int get_block(int dev, int blk, char *lbuf)
{
   //printf("\n=====Calling get_block on dev=%d and blk=%d=====\n", dev, blk);
   lseek(dev, (long)blk*BLKSIZE, SEEK_SET);
   read(dev, lbuf, BLKSIZE);
}   

int put_block(int dev, int blk, char *pbuf)
{
   //printf("\n=====Calling put_block on dev=%d and blk=%d=====\n", dev, blk);
   lseek(dev, blk*BLKSIZE, 0);
   write(dev, pbuf, BLKSIZE);
}   

int tst_bit(char *pbuf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (pbuf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *pbuf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  pbuf[i] |= (1 << j);;
}

int clr_bit(char *pbuf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  pbuf[i] &= ~(1 << j);
}

int ialloc(int dev)
{
  int  i;
  char lbuf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, lbuf);

  for (i=0; i < ninodes; i++){ //Loop through all potential inodes in map
    if (tst_bit(lbuf, i)==0){ //Test if the inode is empty
       set_bit(lbuf,i); //Switch the bit to allocated in the map
       put_block(dev, imap, lbuf); //Put the new imap back onto the disk
       return i+1;
    }
  }
  return 0;
}

int balloc(int dev)
{
  int i;
  char lbuf[BLKSIZE];

  //Read in the data block bitmap block
  get_block(dev, bmap, lbuf);

  for(i = 0; i < nblocks; i++) //Loop through all potentials blocks in the map
  {
    if(tst_bit(lbuf, i) == 0) //Test if the data block is empty
    {
      set_bit(lbuf, i); //Switch the bit from 0 to 1 in the map
      put_block(dev, bmap, lbuf);
      return i+1;
    }
  }
  return 0;
}

void idealloc(int dev, int ino)
{
  //Read in the inode_bitmap
  get_block(dev, imap, buf);

  if(tst_bit(buf, ino) == 1) //Make sure this inode is actually allocated
  {
    clr_bit(buf, ino); //Deallocate the inode
    put_block(dev, imap, buf); //Write the modified imap back 
  }

}

void bdealloc(int dev, int blkno)
{
  //Read in the block_bitmap
  get_block(dev, bmap, buf);

  if(tst_bit(buf, blkno) == 1) //Make sure that block is allocated
  {
    clr_bit(buf, blkno);
    put_block(dev, bmap, buf);
  }
}

int tokenize(char *path)
{
  char* s;
  strcpy(gpath, path); //Copy path into global path
  nname = 0;
  s = strtok(gpath, "/");
  while(s != NULL && *s != NULL) //Loop until fill path has been parsed
  {
    //printf("Tokenizing Global Path: Placing %s at name[%d]\n", s, nname);
    name[nname++] = s; //Copy into name index and then increment nname by 1
    name[nname] = NULL;
    s = strtok(NULL, "/");
  }

  //printf("Done Tokenizing: nname=%d\n", nname);

}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       if(DEBUG) {printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);}
       //printf("minode[%d]->refCount=%d\n", i, mip->refCount);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
      if(DEBUG) {printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);}
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk  = (ino-1) / 8 + inode_start;
       disp = (ino-1) % 8;

       if(DEBUG) {printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);}

       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->INODE
       mip->INODE = *ip;

       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

void iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;

 //printf("mip->refCount = %d\n", mip->refCount);
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write back */
 if(DEBUG) {printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);}

 block =  ((mip->ino - 1) / 8) + inode_start;
 offset =  (mip->ino - 1) % 8;

 /* first get the block containing this inode */
 get_block(mip->dev, block, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->INODE;

 //printf("mip->INODE->link count: %d\n", mip->INODE.i_links_count);

 put_block(mip->dev, block, buf);

 mip->dirty = 0; //Reset INODE for writeback

} 

int search(MINODE *mip, char *name)
{
  //printf("=====Calling search on mip=[%d %d] and %s=====\n", mip->dev, mip->ino, name);
  
  char sbuf[BLKSIZE], temp[256];
    char *cp;
    int i;

    for (i=0; i < 12; i++){  // assume DIR at most 12 direct blocks
        //printf("Loop iteration i:%d || mip->INODE.i_block[0] = %d\n", i, mip->INODE.i_block[0]);
        if (mip->INODE.i_block[i] == 0)
           break;
        get_block(mip->dev, mip->INODE.i_block[i], sbuf);

        dp = (DIR *)sbuf;
        cp = sbuf;
        while(cp < sbuf + BLKSIZE){
           strncpy(temp, dp->name, dp->name_len);
           temp[dp->name_len] = '\0';
           //printf("%8d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp); 
           if(strcmp(temp, name) == 0)
           {
             //printf("found %s: inumber%d\n", name, dp->inode);
           	 return dp->inode;
           }

           cp += dp->rec_len;
           dp = (DIR *)cp;
           memset(temp, 0, 256);
       }
    }

    return 0;
}

int getino(char *path)
{
  int i, ino, blk, disp;
  INODE *ip;
  MINODE *mip;

  //printf("getino: path=%s\n", path);
  if (strcmp(path, "/")==0)
      return 2;

  if (path[0]=='/')
  {
    mip = iget(dev, 2);
    //printf("Absolute Path\n");
  }
  else
  {
    mip = iget(running->cwd->dev, running->cwd->ino);
    //printf("Relative Path\n");
  }


  tokenize(path);

  for (i=0; i<nname; i++){
      //printf("===========================================\n");
      ino = search(mip, name[i]);
      //printf("Search returned ino:%d\n", ino);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
   }
   return ino;
}

void mytruncate(MINODE* mip)
{
  if(mip != NULL) //Make sure MINODE was passed
  {
    for(int i = 0; i < 12; i++) //Loop through all inodes in MIP (ASSUME 12 DIRECT BLKS)
    {
      if(mip->INODE.i_block[i] == 0) //No data block assigned
        continue;

      bdealloc(mip->dev, mip->INODE.i_block[i]); //Deallocate the iblock
    }
  }
}

