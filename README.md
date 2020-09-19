# CPTS 360 Final Project
### A custom shell and ext2 file system 

***Disclaimer:*** This project was built as a requirement for the computer science 360 class at Washington State University under the instruction of K.C. Wang. The structure for the filesystem was provided by K.C. Wang, but the logic for shell and interaction with the filesystem was created by me within the provided parameters of the project. 

---
### Description
This project implements a custom shell over a version of the ext2 file system. The shell allows for someone to interact via the terminal with the file system. A user can create/delete/modify files, and mount/unmount new partititions. 

### Languages and Tools
<img align="left" height="32" width="32" src="https://raw.githubusercontent.com/github/explore/80688e429a7d4ef2fca1e82350fe8e3517d3494d/topics/c/c.png" />
<img align="left" height="32" width="32" src="https://raw.githubusercontent.com/github/explore/80688e429a7d4ef2fca1e82350fe8e3517d3494d/topics/linux/linux.png" />
<img align="left" height="32" width="32" src="https://raw.githubusercontent.com/github/explore/80688e429a7d4ef2fca1e82350fe8e3517d3494d/topics/visual-studio-code/visual-studio-code.png" />
<img align="left" height="32" width="32" src="https://raw.githubusercontent.com/github/explore/80688e429a7d4ef2fca1e82350fe8e3517d3494d/topics/terminal/terminal.png" />
<br>


### How to run program 
```
I will provide instructions soon!
```

### Commands
The following is a list of the possible commands a user can run in the terminal. Unless otherwise stated in a comment in the code, the logic for these function were written by me
| Command   | Description |
| :-------- | :---------- |
| ls        | lists the contents of your current directory |
| cd        | allows you to change your current directory (you can user . & .. as usual) |
| pwd       | prints your current directory path to the terminal |
| mkdir     | creates a new directory with the provided name |
| creat     | creates a new file with the provided name |
| rmdir     | removes the specified directory from the disk |
| rm        | removes the specified file from the disk |
| link      | creates a hard link to a specified file |
| unlink    | deleted the specified ink from the file system, and removed the file if it is the last remaning link |
| symlink   | creates a symbolic link to a specified file |
| readlink  | prints the contents of the symbolic link file |
| touch     | Updates the access and modification time of the provided file and creates the file if the provided filename does not alreay exists |
| stat      | prints out information including file name, size, location on disk, access time, modification time, and number of links |
| chmod     | changes the specified file permissions based on input |
| open      | open a file with a specified mode |
| close     | closes an open file |
| lseek     | moves to a specified byte location within the file |
| pfd       | prints the specified file's descriptor |
| read      | reads a specified number of bytes from the file |
| cat       | prints the contents of a specified file to the screen |
| write     | writes a stream to a file |
| cp        | copies the contents of one file to another and will create the destination file if it does not already exist |
| mv        | moves the contents of one file to another location |
| mount     | mounts another disk or filesystem at the specified mountpoint |
| unmount   | unmounts a disk or filesystem from the provided path |
| quit      | exits the program |
| debug_on  | turns on debug messages |
| debug_off | turns off debug messages |





