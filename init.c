#include "init.h"

void init()
{
    //Initialize the PROC's
    for(int i = 0; i < NPROC; i++)
    {
        proc[i].uid = 0;
        proc[i].pid = 0;
        proc[i].gid = 0;
        proc[i].status = FREE;
        proc[i].next = NULL;
        proc[i].parent = NULL;
        proc[i].child = NULL;
        proc[i].sibling = NULL;
        proc[i].cwd = NULL;
        for(int j = 0; j < 16;proc[i].fd[j] = NULL, j++);
    }

    //Create P0 and P1
    proc[0].uid = 0;
    proc[0].pid = 0;
    proc[0].gid = 0;
    proc[0].status = READY;
    proc[0].next = NULL;
    proc[0].child = &proc[1];
    proc[0].parent = &proc[0];
    proc[0].sibling = NULL;
    proc[0].cwd = NULL;

    proc[1].uid = 0;
    proc[1].pid = 0;
    proc[1].gid = 0;
    proc[1].status = READY;
    proc[1].next = NULL;
    proc[1].child = NULL;
    proc[1].parent = &proc[0];
    proc[1].sibling = NULL;
    proc[1].cwd = NULL;

    //Initialize the minode array
    for(int i = 0; i < NMINODE; i++)
    {
        minode[i].dev = 0;
        minode[i].dirty = 0;
        minode[i].ino = 0;
        minode[i].mounted = 0;
        minode[i].mptr = NULL;
        minode[i].refCount = 0;
    }

    //Initialize global root
    root = NULL;

    printf("Processes and Data Structures Initialized\n");

}

void mountRoot(char* diskName)
{
    //Open device for RW
    dev = open(diskName, O_RDWR);

    if(dev == -1) //Failed to open disk
    {
        fprintf(stderr, "%sERROR (%d) on disk open: %s%s\n", RED, errno, strerror(errno), NORMAL);
        exit(3);
    }

    //Read super block and verify EXT2FS 
    if(get_block(dev, 1, buf) == -1)
    {
        fprintf(stderr, "%sERROR (%d) SUPERBLOCK READ: %s%s\n", RED, errno, strerror(errno), NORMAL);
        close(dev);
        exit(3);
    }

    sp = (SUPER*)buf;

    if(sp->s_magic != EXT2_FS) //EXT2FS system check failed
    {
        printf("%sEXT2 File System Check: FAILED%s\n", RED, NORMAL);
        close(dev);
        exit(3);
    }
    printf("%sEXT2 File System Check: OK%s\n", GREEN, NORMAL);

    //Record information for nblocks and ninodes
    nblocks = sp->s_blocks_count;
    ninodes = sp->s_inodes_count;

    printf("Recorded %snblocks: %d%s and %sninodes: %d%s\n", 
    BLUE, nblocks, NORMAL, BLUE, ninodes, NORMAL);

    //Read in Group Descriptor and record global info
    if(get_block(dev, 2, buf) == -1)
    {
        fprintf(stderr, "ERROR (%d) GROUP DESCRIPTOR READ: %s\n", errno, strerror(errno));
		close(dev);
		exit(3);
    }

    gp = (GD*)buf;
    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;

    printf("Recorded %sbmap block: %d%s, %simap block: %d%s, and %sinodes_start block: %d%s\n", 
    BLUE, imap, NORMAL, BLUE, bmap, NORMAL, BLUE, inode_start, NORMAL);

    //Get the root inode
    root = iget(dev, 2);

    //Set cwd of P0 and P1 to root minode
    proc[0].cwd = iget(dev, 2);
    proc[1].cwd = iget(dev, 2);

    //Set runnning proc to P0
    running = &proc[0];

    printf("%sRoot has been mounted%s\n", GREEN, NORMAL);
}