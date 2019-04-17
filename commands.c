#include "commands.h"

//Globals

char *cmds[] = {"ls", "pwd", "cd", "mkdir", "creat", "rmdir", "rm", "link", "unlink", "symlink", "readlink", "touch", "stat", "chmod", "quit", "debug_on", "debug_off", NULL};
int (*fptr[])(char *, char*) = {(int *)ls, (int *)pwd, (int *)cd, (int *)mk_dir, (int *)creat_file, (int *)rm_dir, (int *)rm_file, (int*)mylink, 
                                (int*)myunlink, (int*)mysymlink, (int*)myreadlink, (int*)my_touch, (int*)my_stat, (int*)my_chmod, (int *)quit, 
                                (int *)debug_on, (int *)debug_off};

int findCommand(char *command)
{
    for (int i = 0; cmds[i] != NULL; i++) //Loop through command list
    {
        if (strcmp(cmds[i], command) == 0)
            return i; //Return index of found command
    }

    return -1;
}

void ls(char *path)
{
    //printf("\n\n======ls on path %s=====\n", path);

    ls_dir(path);
}

void ls_dir(char *dirname)
{
    char lbuf[BLKSIZE]; //Local buffer
    int ino;
    MINODE *mip;
    char *cp, temp[256];
    int check = -1;

    if (dirname != NULL && *dirname != 0) //Run only if there is a provided pathname
    {
        ino = getino(dirname); //Get inode matching dirname

        if (ino == 0) //Provided name could not be found
            return;

        //printf("ino: %d\n", ino);

        mip = iget(dev, ino);

        if (S_ISREG(mip->INODE.i_mode)) //If the provided file is not a directory, list just information about that file
        {
            ls_file(mip->ino, dirname, NULL);
            iput(mip);
            return;
        }

        //printf("mip->ino: %d, mip->dev: %d, INODE.iblock[0]: %d\n", mip->ino, mip->dev, mip->INODE.i_block[0]);
        //printf("dev: %d\n", dev);

        //Get the block from the root node
        check = get_block(mip->dev, mip->INODE.i_block[0], lbuf);
        if (check == -1)
        {
            fprintf(stderr, "Error (%d) on get_block for ino: %d, mip->INODE.i_block[0]: %d || %s\n", errno, ino, mip->INODE.i_block[0], strerror(errno));
            iput(mip);
            return;
        }

        ip = (INODE *)lbuf;
        //iput(mip);
    }
    else //Get data block from current working directory
    {
        get_block(running->cwd->dev, running->cwd->INODE.i_block[0], lbuf);
        ip = (INODE *)lbuf;
    }

    //Set global dir to beginning of buffer
    dp = (DIR *)lbuf;
    cp = lbuf;
    while (cp < lbuf + BLKSIZE) //Loop through all entries in the directory
    {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = '\0';

        if (temp[0] != NULL) //Only print when there is a file name
        {
            //printf("dir file type = %u\n", (unsigned int)(dp->file_type));
            ls_file(dp->inode, temp, dirname);
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
        memset(temp, 0, 256);
    }

}

void ls_file(int ino, char *filename, char* dirname)
{
    MINODE *mip;
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";
    char ftime[64], filebuf[256], fullpath[256];
    struct group *grp;  //I added so I can print the gid name
    struct passwd *pwd; //I added so I can print the uid name
    int len;

    mip = iget(running->cwd->dev, ino);
    ip = &(mip->INODE);

    //Print the information about the file

    //Print the mode
    if (S_ISDIR(ip->i_mode))
        printf("%c", 'd');
    else if (S_ISREG(ip->i_mode))
        printf("%c", '-');
    else if (S_ISLNK(ip->i_mode))
        printf("%c", 'l');

    //Print permissions
    for (int i = 8; i >= 0; i--)
    {
        if (ip->i_mode & (1 << i)) //print r/w/x
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }

    //Print the link count
    printf("%4d", ip->i_links_count);

    //Print the gid
    grp = getgrgid(ip->i_gid);

    if (grp != NULL)
        printf("  %s ", grp->gr_name); // gid
    else
        printf("%4d ", ip->i_gid);

    //Print the uid
    pwd = getpwuid(ip->i_uid);

    if (pwd != NULL)
        printf("  %s ", pwd->pw_name); // uid
    else
        printf("%4d ", ip->i_uid);

    //Print file size
    printf("%7d", ip->i_size);

    //Print time
    strcpy(ftime, ctime(&ip->i_mtime)); // print time in calendar form
    ftime[strlen(ftime) - 1] = 0;       // kill \n at end
    printf("   %s ", ftime);

    //Print name
    printf("  %s", filename);

    //Print linkname for symoblic file	
    if(S_ISLNK(ip->i_mode)) //If the file is a link
    {
        if(dirname && dirname[0] == '/' && dirname[1] == '\0') //Parent dir is root
        {
            strcpy(fullpath, "/");
            strcat(fullpath, filename);
        }
        else if (dirname && strlen(dirname) >= 2)
        {
            strcpy(fullpath, dirname);
            strcat(fullpath, "/");
            strcat(fullpath, filename);
        }
        else
            strcpy(fullpath, filename);

        //printf("DIRNAME: %s\nFILENAME: %s\nFULL PATH: %s\n", dirname, filename, fullpath);
        printf(" -> ");
        myreadlink_nonewline(fullpath);
    }
    
    printf("\n");

    iput(mip);
}

void cd(char *path)
{
    int ino;
    MINODE *mip;

    //If no pathname, change to root
    if (path == NULL || path[0] == '\0')
        path = "/";

    //Get inode of provided path
    ino = getino(path);
    //printf("My INO: %d\n", ino);

    //Get minode from provided ino if valid
    if (ino == 0) //Invalid path
    {
        printf("Invalid path\n");
        return;
    }

    mip = iget(dev, ino);

    //Verify mip->INODE is a DIR
    if (!S_ISDIR(mip->INODE.i_mode)) //Exit if not a DIR
    {
        printf("Path does not lead to a DIR\n");
        iput(mip);
        return;
    }

    iput(running->cwd);
    running->cwd = mip; //Change directory
    if(DEBUG){printf("Changed cwd to ");}
    pwd();
}

void pwd()
{
    if (running->cwd == root)
    {
        printf("/\n");
        return;
    }
    rpwd(running->cwd); //Recurse through current working directory
    printf("\n");
}

void pwd_noNewline()
{
    if (running->cwd == root)
    {
        printf("/");
        return;
    }
    rpwd(running->cwd); //Recurse through current working directory
}

void rpwd(MINODE *wd)
{
    int ino, pino;
    char lbuf[BLKSIZE];
    char *cp;
    MINODE *pip;
    char temp[256];

    if (wd == root) //Base case
        return;

    //From i_block[0], get inode of . and ..
    get_block(wd->dev, wd->INODE.i_block[0], lbuf); //Get data block

    dp = (DIR *)lbuf;
    cp = lbuf;

    ino = dp->inode; //Get ino of current

    cp += dp->rec_len;
    dp = (DIR *)cp;

    pino = dp->inode; //Get ino of parent

    //Call iget on parent inode number
    pip = iget(wd->dev, pino);

    //From pip->INODE.i_block[0]: get myname string as LOCAL
    get_block(pip->dev, pip->INODE.i_block[0], lbuf); //Get data block
    dp = (DIR *)lbuf;
    cp = lbuf;

    while (cp < lbuf + BLKSIZE)
    {
        if (dp->inode == ino) //Found current inode
        {
            strncpy(temp, dp->name, dp->name_len); //Copy the string into temp
            temp[dp->name_len] = '\0';
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    rpwd(pip); //Recursive call on parent

    iput(pip);

    printf("/%s", temp);
}

void mk_dir(char* path)
{
    char * newDir = basename(path); //Name of new direc to be created
    char* parents=dirname(path); //Path of all parent directories
    int pino; //Keeps track of parent inode
    MINODE* pip; //Keeps track of parents info

    if(path[0] == NULL)
    {
        printf("Please provide a name for the new directory\n");
        return;
    }

    pino = getino(parents); //Search path of all parent directories

    if(pino == 0) //Specified path failed
        return;
    
    pip = iget(dev, pino); //Get the minode for searched ino number

    if(!S_ISDIR(pip->INODE.i_mode)) //Check if the found parent is a directory
    {
        printf("Error: %s is not a directory\n", basename(parents));
        iput(pip);
        return;
    }

    if(search(pip, newDir) != 0) //A dir with that name already exists
    {
        printf("A directory with the name %s already exists\n", newDir);
        iput(pip);
        return;
    }

    mymkdir(pip, newDir); //Actually create the new directory

    //Increment parent inode's link count by 1
    pip->INODE.i_links_count++;

    //Touch parents atime 
    pip->INODE.i_atime = time(0L);

    //Mark parent DIRTY
    pip->dirty = 1;

    iput(pip);
}

void mymkdir(MINODE* pip, char* name)
{
    int newino, newbno; //Keeps track of new allocated ino and bno
    MINODE *mip;

    newino = ialloc(pip->dev); //Allocate new ino in imap
    newbno = balloc(pip->dev); //Allocate new bno in bmap

    //printf("Allocated ino=%d and bno=%d\n", newino, newbno);

    mip = iget(pip->dev, newino); //Load the inode into an MINODE

    mip->INODE.i_mode = DIR_MODE; // OR 040755: DIR type and permissions
    mip->INODE.i_uid  = running->cwd->INODE.i_uid; // Owner uid 
    mip->INODE.i_gid  = running->cwd->INODE.i_gid; // Group Id
    mip->INODE.i_size = BLKSIZE; // Size in bytes 
    mip->INODE.i_links_count = 2; // Links count=2 because of . and ..
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);  // set to current time
    mip->INODE.i_blocks = 2; // LINUX: Blocks count in 512-byte chunks 
    mip->INODE.i_block[0] = newbno; // new DIR has one data block   
    for(int i = 1; i < 15; i++)
        mip->INODE.i_block[i] = 0;
    mip->dirty = 1; // mark minode dirty
    iput(mip); // write INODE to disk

    memset(buf, 0, BLKSIZE); //Make sure buf is empty
    dp = (DIR *)buf;

    //Make entry for .
    dp->inode = newino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';

    //Make entry for ..
    dp = (char*)dp + 12;
    dp->inode = pip->ino;
    dp->rec_len = BLKSIZE - 12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';

    put_block(mip->dev, newbno, buf); //Write the block to disk

    enter_name(pip, newino, name); //Put new dir into parent directory



}

void creat_file(char *path)
{
    char * newFile = basename(path); //Name of new direc to be created
    char* parents=dirname(path); //Path of all parent directories
    int pino; //Keeps track of parent inode
    MINODE* pip; //Keeps track of parents info

    if(path[0] == NULL)
    {
        printf("Please provide a name for the new file\n");
        return;
    }

    pino = getino(parents); //Search path of all parent directories

    if(pino == 0) //Specified path failed
        return;

    pip = iget(dev, pino); //Get the minode for searched ino number
    
    if(!S_ISDIR(pip->INODE.i_mode)) //Check if the found parent is a directory
    {
        printf("Error: %s is not a directory\n", basename(parents));
        iput(pip);
        return;
    }

    if(search(pip, newFile) != 0) //A dir with that name already exists
    {
        printf("A file with the name %s already exists\n", newFile);
        iput(pip);
        return;
    }

    mycreat(pip, newFile); //Actually create the new directory

    //Touch parents atime 
    pip->INODE.i_atime = time(0L);

    //Mark parent DIRTY
    pip->dirty = 1;

    iput(pip);
}

void mycreat(MINODE *pip, char* name)
{
    int newino, newbno; //Keeps track of new allocated ino and bno
    MINODE *mip;

    newino = ialloc(pip->dev); //Allocate new ino in imap

    //printf("Allocated ino=%d and bno=%d\n", newino, newbno);

    mip = iget(pip->dev, newino); //Load the inode into an MINODE

    mip->INODE.i_mode = FILE_MODE; // OR 040755: DIR type and permissions
    mip->INODE.i_uid  = running->cwd->INODE.i_uid; // Owner uid 
    mip->INODE.i_gid  = running->cwd->INODE.i_gid; // Group Id
    mip->INODE.i_size = 0; // Size in bytes 
    mip->INODE.i_links_count = 1; // Links count=1 because of . 
    mip->INODE.i_atime = time(0L);
    mip->INODE.i_ctime = time(0L);
    mip->INODE.i_mtime = time(0L);  // set to current time
    mip->INODE.i_blocks = 2; // LINUX: Blocks count in 512-byte chunks   
    for(int i = 0; i < 15; i++)
        mip->INODE.i_block[i] = 0;
    mip->dirty = 1; // mark minode dirty
    iput(mip); // write INODE to disk

    enter_name(pip, newino, name); //Put new dir into parent directory


}

void mycreat_symlink(MINODE *pip, char* name, char* oldname)
{
    int newino, newbno; //Keeps track of new allocated ino and bno
    MINODE *mip;

    newino = ialloc(pip->dev); //Allocate new ino in imap

    //printf("Allocated ino=%d and bno=%d\n", newino, newbno);

    mip = iget(pip->dev, newino); //Load the inode into an MINODE

    mip->INODE.i_mode = LINK_MODE; // OR 040755: DIR type and permissions
    mip->INODE.i_uid  = running->cwd->INODE.i_uid; // Owner uid 
    mip->INODE.i_gid  = running->cwd->INODE.i_gid; // Group Id
    mip->INODE.i_size = strlen(oldname); // Size in bytes 
    mip->INODE.i_links_count = 1; // Links count=1 because of . 
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);  // set to current time
    mip->INODE.i_blocks = 2; // LINUX: Blocks count in 512-byte chunks 
    for(int i = 0; i < 15; i++)
        mip->INODE.i_block[i] = 0;

    strcpy(mip->INODE.i_block, oldname); //Copy the oldname into i_block[]

    mip->dirty = 1; // mark minode dirty
    iput(mip); // write INODE to disk

    enter_name(pip, newino, name); //Put new dir into parent directory

}


void enter_name(MINODE *pip, int childIno, char* childName)
{
    int newbno, i;
    bool inserted = false;

    for(i = 0; i < 12; i++)
    {
        char* cp;
        int idealLength = 0, remainingLength = 0, needLength = 0, tempLength = 0;

        if(pip->INODE.i_block[i] == 0) //Can't write to an unallocated block
            break;

        get_block(pip->dev, pip->INODE.i_block[i], buf); //Read in the data block

        cp = buf;
        dp = (DIR*)cp;

        while(cp + dp->rec_len < buf + BLKSIZE) //Loop until last entry of dir is found
        {
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }

        //Calculate ideal and remaining length for last entry in current DIR
        idealLength = 4*((8 + dp->name_len + 3)/4);
        remainingLength = dp->rec_len - idealLength;

        //Calulate the needed length for the new entry 
        needLength = 4*((8 + strlen(childName) + 3)/4);

        if(needLength <= remainingLength) //Check if new entry can be added to this disk block
        {
            //Trim old entry to new length
            dp->rec_len = idealLength;

            //Move DIR pointer to where new entry will be
            cp += dp->rec_len;
            dp = (DIR*)cp;

            //Set the information for the new DIR
            dp->rec_len = remainingLength; 
            dp->inode = childIno;
            strcpy(dp->name, childName);
            dp->name_len = strlen(childName);
            inserted = true;
            //printf("%s was inserted into block[%d] of parent inode %d", dp->name, pip->INODE.i_block[i], pip->ino);
            break;
        }
    }

    if(inserted != true) //New block will have to be allocated
    {
        if (i < 12) //Direct block needs to be allocated
        {
            newbno = balloc(pip->dev); //Allocated a new block
            if(newbno == 0) //Could not allocated new block
            {
                printf("Block allocation failed\n");
                return;
            }

            //Add new block to parent i_block table
            pip->INODE.i_size += BLKSIZE;
            pip->INODE.i_block[i] = newbno;

            get_block(pip->dev, newbno, buf); //Get the new block

            dp = (DIR*)buf;
             //Set the information for the new DIR
            dp->rec_len = BLKSIZE; 
            dp->inode = childIno;
            strcpy(dp->name, childName);
            dp->name_len = strlen(childName);
        }

        //TODO: Add functionality for adding indirect and double indirect blocks
    }

    put_block(pip->dev, pip->INODE.i_block[i], buf);

}

void rm_dir(char* path)
{
    //printf("In rmdir\n");
    MINODE *mip = NULL;

    if(*path == NULL) //No path was provided
    {
        printf("Please provide a directory to delete\n");
        return;
    }

    if(strcmp(path, "/") == 0)
    {
        printf("Cannot delete the root directory\n");
        return;
    }

    int ino = getino(path); //Search path for inode

    if(ino == 0) //Inode could not be found
        return;

    mip = iget(running->cwd->dev, ino); //Load into MINODE

   myrmdir(mip, path);

}

void myrmdir(MINODE *mip, char* path)
{
    //printf("In myrmdir\n");
    char* cp;
    MINODE *pip = NULL;
    char name[255], parents[255];
    memset(name, 0, 255);
    memset(parents, 0, 255);
    strcpy(name, basename(path));
    strcpy(parents, dirname(path));




    //printf("Got name %s from path:%s\n", name, path);


    if(running->uid != 0) //User is not super user
    {
        if(mip->INODE.i_uid != running->uid) //User does not own this file
            {
                printf("You do not have permission to delete this directory\n");
                iput(mip);
                return;
            }
    }

    if(running->cwd->ino == mip->ino) //You are currently working in the directory you are trying to delete
    {
        printf("You cannot delete the directroy you are working in\n");
        iput(mip);
        return;
    }

    if(!S_ISDIR(mip->INODE.i_mode)) //If the path doesn't lead to a DIR
    {
        printf("%s is not a directory\n", name);
        iput(mip);
        return;
    }

    //Check if DIR is empty
    if(mip->INODE.i_links_count > 2) //For sure means the dir is not empty
    {
        printf("%s is not empty\n", name);
        iput(mip);
        return;
    }

    //Now need to check if files exist on the data block
    get_block(mip->dev, mip->INODE.i_block[0], buf);

    cp = buf;
    dp = (DIR *)cp;

    while(cp + dp->rec_len < buf + BLKSIZE) //Loop through all entries in data block
    {
        if(strncmp(".", dp->name, dp->name_len) != 0 && strncmp("..", dp->name, dp->name_len) != 0) //There is an entry other than . or ..
        {
             printf("%s is not empty\n", name);
             iput(mip);
            return;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    //Deallocate data blocks
    for(int i = 0; i < 12; i++)
    {
        if(mip->INODE.i_block[i] == 0) //No data block allocated there
            continue;
        bdealloc(mip->dev, mip->INODE.i_block[i]);
    }
    //TODO: Add delete for indirect blocks

    //Deallocate inode
    idealloc(mip->dev, mip->ino);

    //Iput mip
    iput(mip);

    //Get the parent DIR's Minode
    //printf("dirname:%s, name:%s\n", parents, name);
    //exit(0);

    pip = iget(mip->dev, getino(parents)); //Get the inode of the parent

    //Remove child entry from parent directory
    rm_child(pip, name);

    //Decrement parents link count by 1
    pip->INODE.i_links_count--;

    //Touch parents atime and mtime fields
    pip->INODE.i_atime = time(0L);
    pip->INODE.i_ctime = time(0L);

    //Mark parent dirty
    pip->dirty = 1;

    //iput parent
    iput(pip);
}

void rm_child(MINODE *pip, char* name)
{
    //printf("In remove child with name:%s\n", name);
    char* cp, *newcp;
    DIR* newdp = NULL;
    int curSize = 0, totalsize = 0;

    for(int i = 0; i < 12; i++)
    {
        //printf("On loop: %d\n", i);
        if(pip->INODE.i_block[i] == 0) //Next loop interation if no block
            continue;

        get_block(pip->dev, pip->INODE.i_block[i], buf); //Get the data block

        cp = buf;
        dp = (DIR *)cp;

        while(cp + dp->rec_len <= buf + BLKSIZE) //Loop through all entries in block
        {
            //printf("Comparing name:%s with dp->name:%s\n", name, dp->name);
            if(strncmp(name, dp->name, dp->name_len) == 0) //Found what to remove
            {
                //printf("Found matching dir:%s\n", name);
                //Only
                if(dp->rec_len == BLKSIZE) //Only entry in the data block
                {
                    //printf("Only entry in block\n");
                    bdealloc(pip->dev, pip->INODE.i_block[i]); //Deallocate the data block

                    //Move all the other data blocks up one
                    for(; i < 11; i++)
                    {
                        pip->INODE.i_block[i] = pip->INODE.i_block[i+1];
                    }
                    pip->INODE.i_block[11] = 0;
                    pip->INODE.i_size -= BLKSIZE;
                    return;
                }

                //End
                else if(dp->rec_len + curSize == BLKSIZE) //Last entry
                {
                    //printf("Last entry in block\n");
                    newdp->rec_len += dp->rec_len; //Add current rec_length to previous
                    put_block(pip->dev, pip->INODE.i_block[i], buf); //Write back to disk after modifying
                    return;
                }

                //Middle
                else
                {
                    //printf("Middle entry in block\n");
                    newcp = cp;
                    newdp = (DIR *)cp;
                    totalsize = curSize;

                    while((totalsize + newdp->rec_len) != BLKSIZE) //Loop until you found the last entry
                    {
                        //printf("totalsize:%d + current rec lenngth:%d = %d\n", totalsize, newdp->rec_len, (totalsize + newdp->rec_len));
                        totalsize += newdp->rec_len;
                        newcp += newdp->rec_len;
                        newdp = (DIR *)newcp;
                        //printf("new totalsize:%d and new rec length:d\n", totalsize);
                    }
                    //printf("Passed loop\n");

                    newdp->rec_len += dp->rec_len; //Add the rec_len of deleting entry to end

                    //printf("Attempting to memcpy\n");
                    memcpy(cp, (cp + dp->rec_len), (BLKSIZE - curSize - dp->rec_len)); //Copy memory over
                    //printf("performed memcpy\n");

                    put_block(pip->dev, pip->INODE.i_block[i], buf); //Write back to disk after modifying
                    return;

                }



            }

            newdp = dp;
            curSize += dp->rec_len;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

    }
}

void rm_file(char* path)
{
    //printf("In rmdir\n");
    MINODE *mip = NULL;

    if(*path == NULL) //No path was provided
    {
        printf("Please provide a file to delete\n");
        return;
    }

    if(strcmp(path, "/") == 0)
    {
        printf("The root directory is not a file dummy\n");
        return;
    }

    int ino = getino(path); //Search path for inode

    if(ino == 0) //Inode could not be found
        return;

    mip = iget(running->cwd->dev, ino); //Load into MINODE

   myrm(mip, path);
}

void myrm(MINODE *mip, char *path)
{
    //printf("In myrmdir\n");
    char* cp;
    MINODE *pip = NULL;
    char name[255], parents[255];
    memset(name, 0, 255);
    memset(parents, 0, 255);
    strcpy(name, basename(path));
    strcpy(parents, dirname(path));




    //printf("Got name %s from path:%s\n", name, path);


    if(running->uid != 0) //User is not super user
    {
        if(mip->INODE.i_uid != running->uid) //User does not own this file
            {
                printf("You do not have permission to delete this file\n");
                iput(mip);
                return;
            }
    }

    if(!S_ISREG(mip->INODE.i_mode)) //If the path doesn't lead to a FILE
    {
        printf("%s is not a regular file\n", name);
        iput(mip);
        return;
    }

    if(mip->INODE.i_links_count <= 1) //Make sure the file isn't linked
    {
        //Deallocate inode
        idealloc(mip->dev, mip->ino);
    }
    else
    {
        mip->INODE.i_links_count--;
    }

    //Iput mip
    iput(mip);

    //Get the parent DIR's Minode
    //printf("dirname:%s, name:%s\n", parents, name);
    //exit(0);

    pip = iget(mip->dev, getino(parents)); //Get the inode of the parent

    //Remove child entry from parent directory
    rm_child(pip, name);

    //Touch parents atime and mtime fields
    pip->INODE.i_atime = time(0L);
    pip->INODE.i_ctime = time(0L);

    //Mark parent dirty
    pip->dirty = 1;

    //iput parent
    iput(pip);
}

void mylink(char* oldname, char* newname)
{
    //printf("\n\n In mylink()\n\n");
    int ino;
    MINODE *mip, *pip; 
    char parents[256], newlink[256];

    memset(parents, 0, 256);
    memset(newlink, 0, 256);

    if(oldname == NULL || *oldname == NULL) //No name provided
    {
        printf("Please provided a file to link\n");
        return;
    }
    else if(newname == NULL || *newname == NULL)
    {
        printf("Please provided a name for the new file\n");
        return;
    }

    //printf("oldname:%s\nnewname:%s\n", oldname, newname);
    //printf("Under Construction\n");

    //Copy over the newlink name and the parent path
    strcpy(newlink, basename(newname));
    strcpy(parents, dirname(newname));

    //printf("parents:%s\nnewname:%s\n", parents, newlink);


    ino = getino(oldname); //Check to see if the oldname is a valid path
    //printf("\nGET INO\n");

    if(ino == 0) //Invalid path
    {
        printf("Invalid path to file to link to\n");
        return;
    }

    mip = iget(running->cwd->dev, ino); //Get MINODE of file to create link from
    //printf("\nIGET MIP\n");

    if(S_ISDIR(mip->INODE.i_mode)) //You cannot hard link to a directory
    {
        printf("You cannot create a link to a directory\n");
        iput(mip);
        return;
    }

    ino = getino(parents); //Get the ino number of the parent path for new file
    //printf("\nGET INO PARENTS\n");

    if(ino == 0) //Invalid path
    {
        printf("Invalid path to directory in which to place new link\n");
        iput(mip);
        return;
    }

    pip = iget(mip->dev, ino); //Get the MINODE of the directory in which to palce new file
    //printf("\nIGET PIP\n");

    if(!S_ISDIR(pip->INODE.i_mode)) //If the parent is not a directory
    {
        printf("You can only add files to a directory\n");
        iput(mip);
        iput(pip);
        return;
    }

    ino = search(pip, newlink); //Search to see if file by same name already exists in parent dir
    //printf("\nSEARCH\n");

    if(ino != 0) //A file by the same name already exists
    {
        if(strcmp(parents, ".") == 0)
            strcpy(parents, "/");
        printf("A file by the name %s already exists in %s\n", newlink, parents);
    }


    enter_name(pip, mip->ino, newlink); //Put the child into the parent directory with inode of file linking too

    //printf("Ref Count Link:%d\n", mip->refCount);
    mip->INODE.i_links_count++; //Increment links count by one

    mip->dirty = 1;
    pip->dirty = 1;
    //Write both parent and copied INODE back to disk
    iput(mip);
    iput(pip);

    return;

    
}

void myunlink(char* pathname)
{
    int pino, ino;
    MINODE *mip, *pip;
    
    char child[256], parents[256];

    memset(child, 0, 256);
    memset(parents, 0, 256);

    if(pathname == NULL || *pathname == NULL) //No file path was provided
    {
        printf("You need to tell me what file to unlink\n");
        return;
    }

    strcpy(child, basename(pathname));
    strcpy(parents, dirname(pathname));

    pino = getino(parents); //Get inode of provided path 

    if(ino == 0) //The specified file/path does not exist
    {
        printf("Invalid Path\n");
        return;
    }

    pip = iget(running->cwd->dev, pino); //Load the MINODE for the corresponding INODE

    ino = search(pip, child); //Search the parent for the child

    if(ino == 0) //Could not find child
    {
        printf("Invalid Path\n");
        iput(pip);
        return;
    }

    mip = iget(pip->dev, ino);

    if(S_ISDIR(mip->INODE.i_mode)) //Cannot unlink a DIR
    {
        printf("Cannot unlink a directory\n");
        iput(pip);
        iput(mip);
        return;
    }

    //printf("mip old link counts: %d\n", mip->INODE.i_links_count);
    mip->INODE.i_links_count--; //Decrement link counts
    //printf("mip new link counts: %d\n", mip->INODE.i_links_count);

    if(mip->INODE.i_links_count == 0) //This was the last file pointing to that block
    {
        if(!S_ISLNK(mip->INODE.i_mode))
            mytruncate(mip); //Remova all data blocks
        idealloc(mip->dev, mip->ino); //Deallocate the inode
    }

    rm_child(pip, child);

    mip->dirty = 1;
    pip->dirty = 1;

    iput(mip);
    iput(pip);
}

void mysymlink(char* oldname, char* newname)
{
    int ino;
    MINODE *pip; 
    char parents[256], newlink[256];

    memset(parents, 0, 256);
    memset(newlink, 0, 256);

    //Check to make sure arguments were provided
    if(oldname == NULL || *oldname == NULL)
    {
        printf("Please provide a filename to link from\n");
        return;
    }

    if(newname == NULL || *newname == NULL)
    {
        printf("Please provide a filename to link to\n");
        return;
    }

    if(strlen(oldname) > 83) //TO many characters to put in link
    {
        printf("Provided existing file path is too long to create link too\n");
        return;
    }

    //Copy over the newlink name and the parent path
    strcpy(newlink, basename(newname));
    strcpy(parents, dirname(newname));

    ino = getino(oldname); //Check to see if the oldname is a valid path
    //printf("\nGET INO\n");

    if(ino == 0) //Invalid path
    {
        printf("Invalid path to file to link to\n");
        return;
    }

    ino = getino(parents);

    if(ino == 0) //Invalid path
    {
        printf("Invalid path to newlink file\n");
        return;
    }

    pip = iget(running->cwd->dev, ino); //Get parent MINODE

    if(!S_ISDIR(pip->INODE.i_mode)) //Can't create link if it's not a DIR
    {
        printf("Cannot create new file inside something other than a Directory\n");
        iput(pip);
        return;
    }

    //printf("Creaing link\n");
    mycreat_symlink(pip, newlink, oldname); //Create the link file
    //printf("Created link\n");

    pip->dirty = 1; //Mark parent dirty

    iput(pip);
}

void myreadlink(char* link)
{
    int ino;
    MINODE *mip;

    if(link == NULL || *link == NULL)
    {
        printf("Please provide a pathname to a link to read\n");
        return;
    }

    ino = getino(link);

    if(ino == 0) //Invalid path
    {
        printf("Invalid path to file\n");
        return;
    }

    mip = iget(running->cwd->dev, ino); //Load the inode into memory

    if(!S_ISLNK(mip->INODE.i_mode)) //Can't read link if it's not a link
    {
        printf("Path does not lead to a link\n");
        iput(mip);
        return;
    }

    printf("%s\n", mip->INODE.i_block);
}

void myreadlink_nonewline(char* link)
{
    int ino;
    MINODE *mip;

    if(link == NULL || *link == NULL)
    {
        printf("Please provide a pathname to a link to read\n");
        return;
    }

    ino = getino(link);

    if(ino == 0) //Invalid path
    {
        printf("Invalid path to file\n");
        return;
    }

    mip = iget(running->cwd->dev, ino); //Load the inode into memory

    if(!S_ISLNK(mip->INODE.i_mode)) //Can't read link if it's not a link
    {
        printf("Path does not lead to a link\n");
        iput(mip);
        return;
    }

    printf("%s", mip->INODE.i_block);

}

void my_touch(char* filename)
{
    int ino;
    MINODE* mip;
    if(filename == NULL || *filename == NULL) //Check if file exists
    {
        printf("Please provide a file to touch\n");
        return;
    }

    ino = getino(filename); //Search for the inode of provided path

    if(ino == 0) //Could not find along provided path
        creat_file(filename); //Create the file
    else 
    {
        //Get MINODE
        mip = iget(running->cwd->dev, ino);

        //Modify access times
        mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);

        mip->dirty=1;
        iput(mip);
    }

    if(DEBUG){printf("touch successfully performed on %s\n",filename);}
    return;

}
void my_stat(char* filename)
{
    int ino;
    MINODE* mip;
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";
    char ftime[64];
    char permissions[11];
    int permissionOctal[3], j = 0, count = 1, index = 1;
    time_t timer;

    memset(permissions, 0, 11);
    permissionOctal[0] = permissionOctal[1] = permissionOctal[2] = 0;


    if(filename == NULL || *filename == NULL) //No provided path
    {
        printf("Please provide the name of a file to stat\n");
        return;
    }

    ino = getino(filename); //See if file exists

    if(ino == 0) //No file at the path
    {
        printf("Invalid path\n");
        return;
    }

    mip = iget(running->cwd->dev, ino); //Load Inode into memory

    //Print information about the file
    printf("  File: %s \n", basename(filename));
    printf("  Size: %-8d Blocks: %-8d IO Block: %-8d", mip->INODE.i_size, mip->INODE.i_blocks, BLKSIZE);

    if(S_ISDIR(mip->INODE.i_mode))
        printf("directory\n");
    else if(S_ISREG(mip->INODE.i_mode))
        printf("regular file\n");
    else if(S_ISLNK(mip->INODE.i_mode))
        printf("symbolic link\n");
    else
        printf("unknown type\n");
    

    printf("Device: %-8d Inode: %-8d Links: %d\n", mip->dev, mip->ino, mip->INODE.i_links_count);
    printf("Access: ");

     //Print the mode
    if (S_ISDIR(mip->INODE.i_mode))
    {       
        strcpy(permissions, "d");
    }
    else if (S_ISREG(mip->INODE.i_mode))
    {
        strcpy(permissions, "-");
    }
    else if (S_ISLNK(mip->INODE.i_mode))
    {
        strcpy(permissions, "l");
    }

    //Print permissions
    for (int i = 8; i >= 0; i--)
    {
        if (mip->INODE.i_mode & (1 << i)) //print r/w/x
        {
            permissions[index] = t1[i];

            if(i == 8 || i == 5 || i == 2)
                permissionOctal[j] += 4;
            else if (i == 7 || i == 4 || i == 1)
                permissionOctal[j] += 2;
            else if (i == 6 || i == 3 || i == 0)
                permissionOctal[j] += 1;
        }
        else
        {
            permissions[index] = t2[i];
        }

        count++;
        index++;

        if(count > 3)
        {
            count = 1;
            j++;
        }
    }

    printf("(0%d%d%d/%s)", permissionOctal[0], permissionOctal[1], permissionOctal[2], permissions);
    printf(" Uid: %d  Gid: %d\n", mip->INODE.i_uid, mip->INODE.i_gid);
    
    memset(ftime, 0, 64);
    timer = mip->INODE.i_atime;
    strcpy(ftime, ctime(&timer));
    ftime[strlen(ftime) - 1] = 0;       // kill \n at end

    printf("Access: %s\n", ftime);

    memset(ftime, 0, 64);
    timer = mip->INODE.i_mtime;
    strcpy(ftime, ctime(&timer));
    ftime[strlen(ftime) - 1] = 0;       // kill \n at end

    printf("Modify: %s\n", ftime);

    memset(ftime, 0, 64);
    timer = mip->INODE.i_ctime;
    strcpy(ftime, ctime(&timer));
    ftime[strlen(ftime) - 1] = 0;       // kill \n at end

    printf("Change: %s\n", ftime);

    iput(mip);


}

void my_chmod(char* mode, char* filename)
{
    int ino, val;
    MINODE *mip;
    bool rd = false, wr = false, ex = false;


    if(mode == NULL || *mode == NULL)
    {
        printf("Please provide a mode value\n");
        return;
    }
    else if(strlen(mode) < 3 || strlen(mode) > 3 ||
            mode[0] > '7' || mode[1] > '7' || mode[2] > '7')
    {
        printf("mode:%s - Length:%d\n", mode, strlen(mode));

        printf("Please provide a valid mode\n");
        return;
    }

    if(filename == NULL || *filename == NULL)
    {
        printf("Please provide a path name\n");
        return;
    }

    ino = getino(filename);

    if(ino == 0) //Could not find provided file
    {
        printf("Please provide a valid filepath\n");
        return;
    }

    mip = iget(running->cwd->dev, ino); //Load inode into memory

    //Keep the file mode
    if(S_ISDIR(mip->INODE.i_mode))
        mip->INODE.i_mode = __S_IFDIR;
    else if(S_ISREG(mip->INODE.i_mode))
        mip->INODE.i_mode = __S_IFREG;
    else if(S_ISREG(mip->INODE.i_mode))
        mip->INODE.i_mode = __S_IFLNK;
    else
        mip->INODE.i_mode = 0;
    

    for(int i = 0; i < 3; i++) //Loop through three chmod values
    {
        rd = wr = ex = false;

        val = mode[i] - '0'; //Conver mode to int

        //Determine what modes to add
        if(val >= 4)
        {
            val -= 4;
            rd = true;
        }
        if(val >= 2)
        {
            val -= 2;
            wr = true;
        }
        if(val >= 1)
        {
            val -= 1;
            ex = true;
        }

        switch(i)
        {
            case 0: //Owner
                if(rd)
                    mip->INODE.i_mode += S_IRUSR;
                if(wr)
                    mip->INODE.i_mode += S_IWUSR;
                if(ex)
                    mip->INODE.i_mode += S_IXUSR;
                break;
            case 1: //Group
                if(rd)
                    mip->INODE.i_mode += S_IRGRP;
                if(wr)
                    mip->INODE.i_mode += S_IWGRP;
                if(ex)
                    mip->INODE.i_mode += S_IXGRP;
                break;
            case 2: //Others
                if(rd)
                    mip->INODE.i_mode += S_IROTH;
                if(wr)
                    mip->INODE.i_mode += S_IWOTH;
                if(ex)
                    mip->INODE.i_mode += S_IXOTH;
                break;
        }

    }

    mip->dirty = 1;
    iput(mip);
}


void printMenu()
{
    printf("|================== MENU ==================|\n");
    printf("|Level 1: ls, pwd, mkdir, creat, rmdir, rm |\n");
    printf("|         link, symlink, unlink, readlink  |\n");
    printf("|         stat, touch, chmod               |\n");
    printf("|Level 2: Coming...                        |\n");
    printf("|Level 3: Coming...                        |\n");
    printf("|==========================================|\n");
}

void quit()
{
    //iput() on all minodes with (refCount > 0 && DIRTY);
    for (int i = 0; i < NMINODE; i++)
    {
        if ((minode[i].refCount > 0) && minode[i].dirty)
        {
            printf("Calling iput() on minode[%d]\n", i);
            minode[i].refCount = 1;
            iput(&minode[i]);
        }
    }

    printf("Quitting the program\n");
    exit(1);
}

void debug_on()
{
  if(DEBUG) //Debug is already on
    printf("Debug is already on ya nimwhit\n");
  else
  {
    DEBUG = true;
    printf("Debug messages will now print\n");
  }
  
}

void debug_off()
{
  if(!DEBUG) //Debug is already on
    printf("Debug is already off ya knucklehead\n");
  else
  {
    DEBUG = false;
    printf("Debug messages will no longer print\n");
  }
  
}