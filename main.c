
/* * * * * * * * * * *
* Karl Estes		 *
* ID: 11467854	   *
* CPTS 360: Lab7	 *
* * * * * * * * * * */
#include "init.h"
#include "commands.h" 

//Global variables
MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
char   gpath[256];
char   *name[64];
int    nname;
int    fd, dev;
int    nblocks, ninodes, bmap, imap, inode_start;
char   line[256], cmd[32], pathname[256], pathname2[256];
char   buf[BLKSIZE];
char *GREEN = "\033[1m\033[32m", *NORMAL =  "\033[0m", *RED = "\033[1m\033[91m";
char *LAVENDER = "\033[0m\033[35m", *PURPLE = "\033[1m\033[35m", *DARKBLUE = "\033[1m\033[34m";
char *BLUE = "\033[0m\033[34m", *YELLOW = "\033[1m\033[33m", *BRIGHT = "\033[0m\033[1m";
bool DEBUG = false;

int main(int argc, char* argv[], char* env[])
{
    int index = -1; //Used in running commands
    bool menuSwitch = false;
    struct passwd* login = getpwuid(getuid()); //Get the current login for the user
    char* computer = getenv("NAME");
    switch(argc)
    {
        case 1: printf("%sPlease enter a DISKNAME%s\n", YELLOW, NORMAL); exit(3); break;
        case 2: break;
        case 3:
                if(strcmp("-d", argv[2]) == 0) //DEBUG is turned on
                {
                    DEBUG = true;
                    printf("DEBUG is set to true\n");
                    sleep(2);
                }
                else //Just keep DEBUG at default
                {
                    printf("Second parameter not recognized... DEBUG is set to false\n");
                    sleep(2);
                }
                break;
        default: printf("Too many parameters passed to program\n"); exit(3); break;
    }

    system("clear"); //Clear the current terminal screen

    //Initialize the data structures
    init();
    //Mount the root
    mountRoot(argv[1]);
    printf("%sType %s? %sto pull up the menu%s\n",
    YELLOW, LAVENDER, YELLOW, NORMAL);

    //Process commands
    while(true)
    {
        login = getpwuid(running->uid);
        printf("%s%s@%s%s:%s", GREEN, login->pw_name, computer, NORMAL, DARKBLUE);
        pwd_noNewline();
        printf("%s$ ", NORMAL);


        fgets(line, 256, stdin); //Read in a line
        line[strlen(line) - 1] = '\0'; //Set null terminator
        sscanf(line, "%s %s %s", cmd, pathname, pathname2);

        index = findCommand(cmd); //Find the command

        if(DEBUG){printf("cmd:%s - arg1:%s - arg2:%s\n", cmd, pathname, pathname2);}

        if(index != -1)
            fptr[index](pathname, pathname2); //Run the found command
        else if (strcmp(cmd, "?") == 0)//Check if ? was answered
            printMenu();
        else
            printf("Invalid command: %s\n", cmd);

       //Reset variables to 0
       memset(line, 0, 256);
       memset(cmd, 0, 32);
       memset(pathname, 0, 256);

    }

    close(dev); 
}