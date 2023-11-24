#ifndef OS_H
#define OS_H
#include <bits/stdc++.h>
#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
/**
 * overall annotation
 * total capacity:100Mb=12.5MB / block size:1kb=128B
 * therefore 100k blocks can be used
 */
/**
 * inode/block的分配/回收
        inode的分配和回收较为简单，采用顺序分配和回收。
        需要分配时，从inode位图中顺序查找一个空闲的inode，查找成功返回inode的编号。
        回收的时候，更新inode位图即可。
        分配和回收都需要更新inode位图
 *
*/
// macro definition(unit:byte)
#define BLOCK_SIZE 128 // A block size is 128B
#define INODE_SIZE 128 // A Inode size is 128B
#define MAX_NAME_SIZE 12

#define INODE_NUM 32768 // one inode,one file(therefore inode:32k;block:32k)
#define BLOCK_NUM 32768 // BLOCK_NUM*BLOCK_SIZE=capacity

#define MODE_DIR 01000           // Directory flag
#define MODE_FILE 00000          // File flag
#define OWNER_R 4 << 6           // Read permission for owner
#define OWNER_W 2 << 6           // Write permission for owner
#define OWNER_X 1 << 6           // Execute permission for owner
#define GROUP_R 4 << 3           // Read permission for group
#define GROUP_W 2 << 3           // Write permission for group
#define GROUP_X 1 << 3           // Execute permission for group
#define OTHERS_R 4               // Read permission for others
#define OTHERS_W 2               // Write permission for others
#define OTHERS_X 1               // Execute permission for others
#define FILE_DEF_PERMISSION 0664 // Default file permission
#define DIR_DEF_PERMISSION 0755  // Default directory permission

#define FILESYSNAME "os.sys"

// Structure declaration
struct SuperBlock {            // Byte:2+2 2+2+4+4 2+2+2+2 4+4+4+4+4 = 36B anyhow < 128(1 block'size)
    unsigned short s_inodeNum; // the number of inode(32768<65535)
    unsigned short s_blockNum; // the number of block(32768<65535)

    unsigned short s_freeInodeNum; // the number of free inode
    unsigned short s_freeBlockNum; // the number of free block

    unsigned short s_blockSize;      // always 128B(the size of a block)
    unsigned short s_inodeSize;      // always 128B(the size of a block)
    unsigned short s_SuperBlockSize; // in dev-C++,it is 36B

    // Disk distribution
    int s_SuperBlockAddr;  // 0
    int s_InodeBitmapAddr; // 1 * BLOCK_SIZE
    int s_BlockBitmapAddr; // 33 * BLOCK_SIZE
    int s_InodeAddr;       // 65 * BLOCK_SIZE
    int s_BlockAddr;       // 32833 * BLOCK_SIZE
};
struct Inode {           // Byte:2+2+4=8(In Linux,anyhow a Inode should occupy a block)
    unsigned short i_id; // Compact mode:one inode->one block
    unsigned short i_mode;
    unsigned int i_size;
    int i_block; // the block'number that corresponds to this i node(just add 32768)
};
struct DirItem {                  // 16B(one block can store up to 8 directory entries)
    char itemName[MAX_NAME_SIZE]; // the name of dir or file
    int inodeAddr;                // the inode'address of dir or file
};

// Global variable declaration
extern SuperBlock *superblock;
extern const int SuperBlockAddr; // initial address
extern const int InodeBitmapAddr;
extern const int BlockBitmapAddr;
extern const int InodeAddr;
extern const int BlockAddr;
extern const int FileMaxSize;
extern const int SumSize;

extern int rootDirAddr;         // Root directory inode address(8320)
extern int curDirAddr;          // current directory
extern char curDirName[12];     // current directory'name
extern char curUserName[12];    // default:root
extern char curHostName[12];    // current host name(LEVENT)
extern char curUserDirName[12]; // Current login user directory name
extern bool isLogin;            // Whether a user has logged in

extern FILE *fw; // Virtual disk file write file pointer
extern FILE *fr; // Virtual disk file read file pointer
extern bool InodeBitmap[INODE_NUM];
extern bool BlockBitmap[BLOCK_NUM];

extern char buffer[12500000]; // 100Mb=12.5MB->caching the entire virtual disk file

// function declaration
// fundamental function

// final functions
void info();
void cd(); // need parameters
void dir(int dirName);
void md(int curPath, int dirName); // also need a path parameter
void rd(int curPath, int dirName);
void newfile(int curPath, int fileName);
void cat(int curPath, int fileName);
void copy(); // need parameters
void del(int curPath, int fileName);
void check();
#endif
