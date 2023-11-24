#include "os.h"
using namespace std;
//
const int SuperBlockAddr = 0;                           // superblock occupies 1 block
const int InodeBitmapAddr = 1 * BLOCK_SIZE;             // inodeBitmap occupies 32 blocks(32768 bits)
const int BlockBitmapAddr = 33 * BLOCK_SIZE;            // blockBitmap also occupies 32 blocks
const int InodeAddr = 65 * BLOCK_SIZE;                  // inode occupies 32768 blocks
const int BlockAddr = 32833 * BLOCK_SIZE;               // block also occupies 32768 blocks
const int FileMaxSize = BLOCK_SIZE;                     // a file can occupy at most one block
const int SumSize = BlockAddr + BLOCK_SIZE * BLOCK_NUM; // Maximum available disk capacity(unit:Byte)

int rootDirAddr;        // Root directory inode address
int curDirAddr;         // current directory'address
char curDirName[12];     // current directory'name
char curUserName[12];    // current user'name (default:root)
char curHostName[12];    // current host name
char curUserDirName[12]; // Current login user directory name

bool isLogin;

extern FILE *fw; // Virtual disk file write file pointer
extern FILE *fr; // Virtual disk file read file pointer
extern bool InodeBitmap[INODE_NUM];
extern bool BlockBitmap[BLOCK_NUM];
SuperBlock *superblock = new SuperBlock;

char buffer[12500000] = {0}; // capacity:100Mb=12.5MB

int main()
{
    // Open the virtual disk file
    if ((fr = fopen(FILESYSNAME, "rb")) == NULL) { // read-only(binary file)
        fw = fopen(FILESYSNAME, "wb");             // if NULL-->create one
        if (fw == NULL) {
            cout << "Failed to open the disk file. Procedure!"
                 << "\n"; // using \n means not flushing right now
            return 0;
        }
        fr = fopen(FILESYSNAME, "rb"); // now having one,use it

        // initialization variable
        isLogin = false;
        strcpy(curUserName, "root"); // because of the char[], the = cannot be used(char* is fine)

        // Get host name(LEVENT)
        memset(curHostName, 0, sizeof(curHostName));
        DWORD k = 100;                    // k Specifies the length of the host name
        GetComputerName(curHostName, &k); // this is a Windows API

        // Root inode address, current directory address and name
        rootDirAddr = InodeAddr; // the first inode address give to the root
        curDirAddr = rootDirAddr;
        strcpy(curDirName, "/");

        printf("Formatting file system...\n");
        if (!Formatting()) {
            printf("Failed to format file system\n");
            return 0;
        }
        printf("Formatting completed\n");
        printf("Press any key to log in for the first time.\n");
        system("pause");
        system("cls");

        if (!Installing()) {
            printf("Failed to install file system\n");
            return 0;
        }
    } else { // already exist
        fread(buffer, SumSize, 1, fr);

        // Read file content and store it in a buffer, then open the file in write mode to overwrite its content
        fw = fopen(FILESYSNAME, "wb"); // Open the virtual disk file in write mode (binary file)
        if (fw == NULL) {
            printf("Failed to open the virtual disk file\n");
            return false; // Failed to open the file
        }
        fwrite(buffer, SumSize, 1, fw);
        /* Prompt for whether to format
         * Since it is not the first login, we will skip this step for now
         * Below, the variables need to be set manually
         preparation();
         system("pause");
         system("cls");
        */

        // initialization variable
        isLogin = false;
        strcpy(curUserName, "root"); // because of the char[], the = cannot be used(char* is fine)

        // Get host name
        memset(curHostName, 0, sizeof(curHostName));
        DWORD k = 100;                    // k Specifies the length of the host name
        GetComputerName(curHostName, &k); // this is a Windows API

        // Root inode address, current directory address and name
        rootDirAddr = InodeAddr; // the first inode address
        curDirAddr = rootDirAddr;
        strcpy(curDirName, "/");

        if (!Installing()) {
            printf("Failed to install file system\n");
            return 0;
        }
    }
    while (true) {
        if (isLogin) {
            char str[100];
            char *p;
            if ((p = strstr(curDirName, curUserDirName)) == NULL) // not found
                printf("[%s@%s %s]# ", curHostName, curUserName, curDirName);
            else
                printf("[%s@%s ~%s]# ", curHostName, curUserName, curDirName + strlen(curUserDirName));
            //The gets(str) statement waits for the user to enter a line of characters in the standard input stream, then stores it in the array str
            gets(str);
            cmd(str);
        } else {
            printf("Welcome to the file system,please first login\n");
            while (!login())
                ; // continue to wait
            printf("login successfully!\n");
            system("cls");
        }
    }
    fclose(fw);
    fclose(fr);
    return 0;
}