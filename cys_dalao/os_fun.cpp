#include "os.h"
using namespace std;

void preparation()
{ // Preparations before logging in to the system, including variable initialization + registration + installation re;
    // initialization variable
    isLogin = false;
    strcpy(curUserName, "root");

    // Get host name
    memset(curHostName, 0, sizeof(curHostName));
    DWORD k = 100;                    // k Specifies the length of the host name
    GetComputerName(curHostName, &k); // this is a Windows API

    // Root inode address, current directory address and name
    rootDirAddr = InodeAddr; // the first inode address
    curDirAddr = rootDirAddr;
    strcpy(curDirName, "/");

    char c;
    cout << "Is it formatted? [y/n]" << endl;
    while (c = getch()) {
        fflush(stdin); // seriously??
        if (c == 'y') {
            printf("\n");
            printf("Formatting file system...\n");
            if (!Formatting()) {
                printf("File system formatting failed\n");
                return;
            }
            printf("Formatting completed\n");
            break;
        } else if (c == 'n') {
            printf("\n");
            break;
        }
    }

    if (!Installing()) {
        printf("fail to install the file system\n");
        return;
    }
}
bool Formatting()
{ // Format a  file system
    // Initialize the superblock
    superblock->s_inodeNum = INODE_NUM;
    superblock->s_blockNum = BLOCK_NUM;
    superblock->s_SuperBlockSize = sizeof(SuperBlock); // 36
    superblock->s_inodeSize = INODE_SIZE;              // 128 Byte
    superblock->s_blockSize = BLOCK_SIZE;
    superblock->s_freeInodeNum = INODE_NUM; // initially all inodes are free
    superblock->s_freeBlockNum = BLOCK_NUM;
    superblock->s_SuperBlockAddr = SuperBlockAddr;
    superblock->s_BlockBitmapAddr = BlockBitmapAddr;
    superblock->s_InodeBitmapAddr = InodeBitmapAddr;
    superblock->s_BlockAddr = BlockAddr;
    superblock->s_InodeAddr = InodeAddr;

    // Initialize the inode bit diagram & block bit diagram
    // Set all bitmaps to 0 before writing them to file
    memset(InodeBitmap, 0, sizeof(InodeBitmap));
    fseek(fw, InodeBitmapAddr, SEEK_SET); // Writes the bitmap array InodeBitmap to the file at the specified location, InodeBitmapAddr
    fwrite(InodeBitmap, sizeof(InodeBitmap), 1, fw);
    memset(BlockBitmap, 0, sizeof(BlockBitmap));
    fseek(fw, BlockBitmapAddr, SEEK_SET);
    fwrite(BlockBitmap, sizeof(BlockBitmap), 1, fw);

    // write superblock
    fseek(fw, SuperBlockAddr, SEEK_SET);
    fwrite(superblock, sizeof(SuperBlock), 1, fw);

    fflush(fw); // use fflush when switching between the fw and fr Pointers

    // Read the inode bitmap
    fseek(fr, InodeBitmapAddr, SEEK_SET);
    fread(InodeBitmap, sizeof(InodeBitmap), 1, fr);

    // Read the block bitmap
    fseek(fr, BlockBitmapAddr, SEEK_SET);
    fread(BlockBitmap, sizeof(BlockBitmap), 1, fr);

    fflush(fr);

    // Creating root directory "/"
    Inode cur;

    // apply for inode
    int inoAddr = ialloc(); // Assign the inode area function and return the inode address

    //  apply a block for this inode
    int blockAddr = balloc();

    // ��������̿������һ����Ŀ "."
    DirItem dirList[8] = {0}; // one block can store up to 8 directory entries
    strcpy(dirList[0].itemName, ".");
    dirList[0].inodeAddr = inoAddr;

    // д�ش��̿�
    fseek(fw, blockAddr, SEEK_SET);
    fwrite(dirList, sizeof(dirList), 1, fw);

    // ��inode��ֵ
    cur.i_id = 0;
    cur.i_mode = MODE_DIR | DIR_DEF_PERMISSION;
    cur.i_size = superblock->s_blockSize;
    cur.i_block = blockAddr;

    // д��inode
    fseek(fw, inoAddr, SEEK_SET);
    fwrite(&cur, sizeof(Inode), 1, fw);
    fflush(fw);

    // ����Ŀ¼�������ļ�
    mkdir(rootDirAddr, "home"); // �û�Ŀ¼
    cd(rootDirAddr, "home");
    mkdir(curDirAddr, "root");

    cd(curDirAddr, "..");
    mkdir(curDirAddr, "etc"); // �����ļ�Ŀ¼
    cd(curDirAddr, "etc");

    char buf[1000] = {0};

    sprintf(buf, "root:x:%d:%d\n", nextUID++, nextGID++); // ������Ŀ���û������������룺�û�ID���û���ID
    create(curDirAddr, "passwd", buf);                    // �����û���Ϣ�ļ�

    sprintf(buf, "root:root\n");       // ������Ŀ���û���������
    create(curDirAddr, "shadow", buf); // �����û������ļ�
    chmod(curDirAddr, "shadow", 0660); // �޸�Ȩ�ޣ���ֹ�����û���ȡ���ļ�

    sprintf(buf, "root::0:root\n");           // ���ӹ���Ա�û��飬�û����������һ��Ϊ�գ�����û��ʹ�ã������ʶ�ţ������û��б��ã��ָ���
    sprintf(buf + strlen(buf), "user::1:\n"); // ������ͨ�û��飬�����û��б�Ϊ��
    create(curDirAddr, "group", buf);         // �����û�����Ϣ�ļ�

    cd(curDirAddr, ".."); // �ص���Ŀ¼

    return true;
}
bool Installing()
{ // Install the file system, read the key information from the virtual disk file into memory, such as the superblock.

    // Read and write the virtual disk file, read the superblock, read the inode bitmap, block bitmap, read the root directory, read the etc directory, read the admin directory for the administrator, read the xiao directory for the user, and read the passwd file.

    // Read the superblock.
    fseek(fr, SuperBlockAddr, SEEK_SET);
    fread(superblock, sizeof(SuperBlock), 1, fr);

    // Read the inode bitmap.
    fseek(fr, InodeBitmapAddr, SEEK_SET);
    fread(InodeBitmap, sizeof(InodeBitmap), 1, fr);

    // Read block bitmap.
    fseek(fr, BlockBitmapAddr, SEEK_SET);
    fread(BlockBitmap, sizeof(BlockBitmap), 1, fr);

    return true;
}
void infoOfSuperBlock()
{ // show the information of superblock
    printf("\n");
    printf("Free inode count / Total inode count: %d / %d\n", superblock->s_freeInodeNum, superblock->s_inodeNum);
    printf("Free block count / Total block count: %d / %d\n", superblock->s_freeBlockNum, superblock->s_blockNum);
    printf("Block size of this system: %d bytes, each inode occupies %d bytes (actual size: %d bytes)\n", superblock->s_blockSize, superblock->s_inodeSize, sizeof(Inode));
    printf("\tSuperblock size: %d bytes (actual size: %d bytes)\n", superblock->s_blockSize, superblock->s_SuperBlockSize);
    printf("Disk distribution:\n");
    printf("\tStart position of the superblock: %d B\n", superblock->s_SuperBlockAddr);
    printf("\tStart position of the inode bitmap: %d B\n", superblock->s_InodeBitmapAddr);
    printf("\tStart position of the block bitmap: %d B\n", superblock->s_BlockBitmapAddr);
    printf("\tStart position of the inode area: %d B\n", superblock->s_InodeAddr);
    printf("\tStart position of the block area: %d B\n", superblock->s_BlockAddr);
    printf("\n");
}
void printInodeBitmap()
{ // Print inode usage
    printf("\n");
    printf("Inode Bitmap: [Used:%d %d/%d]\n", superblock->s_inodeNum - superblock->s_freeInodeNum, superblock->s_freeInodeNum, superblock->s_inodeNum);
    int i = 0;
    printf("0 ");
    while (i < superblock->s_inodeNum) {
        if (InodeBitmap[i]) // in using
            printf("*");
        else
            printf(".");
        i++;
        if (i != 0 && i % 32 == 0) {
            printf("\n");
            if (i != superblock->s_inodeNum)
                printf("%d ", i / 32);
        }
    }
    printf("\n");
}
void printBlockBitmap(int num)
{ // Print block usage
    printf("\n");
    printf("Block Bitmap: [Used:%d %d/%d]\n", superblock->s_blockNum - superblock->s_freeBlockNum, superblock->s_freeBlockNum, superblock->s_blockNum);
    int i = 0;
    printf("0 ");
    while (i < num) {
        if (BlockBitmap[i])
            printf("*");
        else
            printf(".");
        i++;
        if (i != 0 && i % 32 == 0) {
            printf("\n");
            if (num == superblock->s_blockNum)
                getchar();
            if (i != superblock->s_blockNum)
                printf("%d ", i / 32);
        }
    }
    printf("\n");
}
int balloc()
{ // Disk block allocation function
    // Use the free block stack in the superblock
    // Calculate the current stack top
    if (superblock->s_freeBlockNum == 0) { // No remaining free block
        printf("No available blocks to allocate\n");
        return -1; // No available free block, return -1
    } else {       // Still have remaining blocks
        // Update the superblock
        int i;
        for (i = 0; i < superblock->s_blockNum; i++)
            if (BlockBitmap[i] == 0)
                break;

        // update the superblock
        superblock->s_freeBlockNum--;
        fseek(fw, SuperBlockAddr, SEEK_SET);
        fwrite(superblock, sizeof(SuperBlock), 1, fw);

        // Update the block bitmap
        BlockBitmap[i] = 1;
        fseek(fw, BlockBitmapAddr + i, SEEK_SET);      //(retAddr-Block_StartAddr)/BLOCK_SIZE indicates which free block it is
        fwrite(&block_bitmap[i], sizeof(bool), 1, fw); // write 1 at position i

        fflush(fw);

        return retAddr;
    }
}
bool bfree(int addr)
{ // Disk block deallocation function
    // The address is not the start address of a disk block
    if ((addr - BlockAddr) % superblock->s_blockSize != 0) {
        printf("Address error, the position is not the start position of a block\n");
        return false;
    }
    unsigned int bno = (addr - BlockAddr) / superblock->s_blockSize; // Inode number
    // The address is not in use, cannot free the space
    if (block_bitmap[bno] == 0) {
        printf("The block is not in use, cannot be freed\n");
        return false;
    }

    // clear the block content
    char temp[BLOCK_SIZE] = {0};
    fseek(fw, addr, SEEK_SET);
    fwrite(temp, sizeof(temp), 1, fw);

    // Update the superblock
    superblock->s_freeBlockNum++; // Increment the number of free blocks
    fseek(fw, SuperBlockAddr, SEEK_SET);
    fwrite(superblock, sizeof(SuperBlock), 1, fw);

    // Update the block bitmap
    BlockBitmap[bno] = 0;
    fseek(fw, bno + BlockBitmapAddr, SEEK_SET); //(addr-Block_StartAddr)/BLOCK_SIZE indicates which free block it is
    fwrite(&BlockBitmap[bno], sizeof(bool), 1, fw);

    fflush(fw);

    return true;
}

int ialloc()
{ // Allocate inode function, return inode address
    // Sequentially search for a free inode in the inode bitmap.If found,return the inode address
    if (superblock->s_freeInodeNum == 0) {
        printf("No available inodes to allocate\n");
        return -1;
    } else {
        // Sequentially search for a free inode
        int i;
        for (i = 0; i < superblock->s_inodeNum; i++)
            if (InodeBitmap[i] == 0) // Found a free inode
                break;

        // Update the superblock
        superblock->s_freeInodeNum--;                  // Decrease the number of free inodes
        fseek(fw, SuperBlockAddr, SEEK_SET);           // move fw to the SuperBlockAddr
        fwrite(superblock, sizeof(SuperBlock), 1, fw); // Writes the data pointed to by superblock to a file

        // Update the inode bitmap
        InodeBitmap[i] = 1;
        fseek(fw, InodeBitmapAddr + i, SEEK_SET);
        fwrite(&InodeBitmap[i], sizeof(bool), 1, fw); // Write 1 at position i to file
        fflush(fw);

        return InodeAddr + i * superblock->s_inodeSize;
    }
}

bool ifree(int addr)
{                                                            // Free inode area function
    if ((addr - InodeAddr) % superblock->s_inodeSize != 0) { // Check
        printf("Address error, the position is not the start position of an inode\n");
        return false;
    }
    unsigned short ino = (addr - InodeAddr) / superblock->s_inodeSize; // Inode number
    if (inode_bitmap[ino] == 0) {
        printf("The inode is not in use, cannot be freed\n");
        return false;
    }

    // Clear the inode content
    Inode tmp = {0};
    fseek(fw, addr, SEEK_SET);
    fwrite(&tmp, sizeof(tmp), 1, fw); // write the 0 in this address

    superblock->s_freeInodeNum++; // Update the superblock(Increment the number of free inodes)
    fseek(fw, SuperBlockAddr, SEEK_SET);
    fwrite(superblock, sizeof(SuperBlock), 1, fw);

    // Update the inode bitmap
    InodeBitmap[ino] = 0;
    fseek(fw, InodeBitmapAddr + ino, SEEK_SET);
    fwrite(&InodeBitmap[ino], sizeof(bool), 1, fw); // Write 0 at position ino to file

    fflush(fw);

    return true;
}
bool mkdir(int parinoAddr, char name[])
{ // Ŀ¼������������������һ��Ŀ¼�ļ�inode��ַ ,Ҫ������Ŀ¼��
    if (strlen(name) >= MAX_NAME_SIZE) {
        printf("NO WAY!");
        return false;
    }
    DirItem dirList[8];
    Inode cur;
    fseek(fr, parinoAddr, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int i = 0;
    int posi = -1, posj = -1;
    while (i<8) { // one direct block,each block contains at most 8 direcotry items
        
        // ȡ�����ֱ�ӿ飬Ҫ�����Ŀ¼��Ŀ��λ��
        fseek(fr, cur.i_block, SEEK_SET);
        fread(dirList, sizeof(dirList), 1, fr);
        fflush(fr);

        // ����ô��̿��е�����Ŀ¼��
        int j;
        for (j = 0; j < 8; j++) {
            if (strcmp(dirList[j].itemName, name) == 0) {
                Inode tmp;
                fseek(fr, dirList[j].inodeAddr, SEEK_SET);
                fread(&tmp, sizeof(Inode), 1, fr);
                if (((tmp.i_mode >> 9) & 1) == 1) { // ����Ŀ¼
                    printf("Ŀ¼�Ѵ���\n");
                    return false;
                }
            } else if (strcmp(dirList[j].itemName, "") == 0) {
                // �ҵ�һ�����м�¼������Ŀ¼���������λ��
                // ��¼���λ��
                if (posi == -1) {
                    posi = dno;
                    posj = j;
                }
            }
            i++;
        }
    }
    if (posi != -1) { // �ҵ��������λ��

        // ȡ�����ֱ�ӿ飬Ҫ�����Ŀ¼��Ŀ��λ��
        fseek(fr, cur.i_dirBlock[posi], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        // �������Ŀ¼��
        strcpy(dirlist[posj].itemName, name); // Ŀ¼��
        // д��������¼ "." ".."���ֱ�ָ��ǰinode�ڵ��ַ���͸�inode�ڵ�
        int chiinoAddr = ialloc(); // ���䵱ǰ�ڵ��ַ
        if (chiinoAddr == -1) {
            printf("inode����ʧ��\n");
            return false;
        }
        dirlist[posj].inodeAddr = chiinoAddr; // ������µ�Ŀ¼�����inode��ַ

        // ��������Ŀ��inode
        Inode p;
        p.i_ino = (chiinoAddr - Inode_StartAddr) / superblock->s_INODE_SIZE;
        p.i_atime = time(NULL);
        p.i_ctime = time(NULL);
        p.i_mtime = time(NULL);
        strcpy(p.i_uname, Cur_User_Name);
        strcpy(p.i_gname, Cur_Group_Name);
        p.i_cnt = 2; // �������ǰĿ¼,"."��".."

        // �������inode�Ĵ��̿飬�ڴ��̺���д��������¼ . �� ..
        int curblockAddr = balloc();
        if (curblockAddr == -1) {
            printf("block����ʧ��\n");
            return false;
        }
        DirItem dirlist2[16] = {0}; // ��ʱĿ¼���б� - 2
        strcpy(dirlist2[0].itemName, ".");
        strcpy(dirlist2[1].itemName, "..");
        dirlist2[0].inodeAddr = chiinoAddr; // ��ǰĿ¼inode��ַ
        dirlist2[1].inodeAddr = parinoAddr; // ��Ŀ¼inode��ַ

        // д�뵽��ǰĿ¼�Ĵ��̿�
        fseek(fw, curblockAddr, SEEK_SET);
        fwrite(dirlist2, sizeof(dirlist2), 1, fw);

        p.i_dirBlock[0] = curblockAddr;
        int k;
        for (k = 1; k < 10; k++) {
            p.i_dirBlock[k] = -1;
        }
        p.i_size = superblock->s_BLOCK_SIZE;
        p.i_indirBlock_1 = -1; // ûʹ��һ����ӿ�
        p.i_mode = MODE_DIR | DIR_DEF_PERMISSION;

        // ��inodeд�뵽�����inode��ַ
        fseek(fw, chiinoAddr, SEEK_SET);
        fwrite(&p, sizeof(Inode), 1, fw);

        // ����ǰĿ¼�Ĵ��̿�д��
        fseek(fw, cur.i_dirBlock[posi], SEEK_SET);
        fwrite(dirlist, sizeof(dirlist), 1, fw);

        // д��inode
        cur.i_cnt++;
        fseek(fw, parinoAddr, SEEK_SET);
        fwrite(&cur, sizeof(Inode), 1, fw);
        fflush(fw);

        return true;
    } else {
        printf("û�ҵ�����Ŀ¼��,Ŀ¼����ʧ��");
        return false;
    }
}
bool rmdir(int parinoAddr, char name[])
{ // Ŀ¼ɾ������
}
bool create(int parinoAddr, char name[], char buf[])
{ // �����ļ�����
}
bool del(int parinoAddr, char name[])
{ // ɾ���ļ�����
}
void ls(int parinoaddr)
{ // ��ʾ��ǰĿ¼�µ������ļ����ļ���
}
void cd(int parinoaddr, char name[])
{ // ���뵱ǰĿ¼�µ�nameĿ¼
}
void gotoxy(HANDLE hOut, int x, int y)
{ // �ƶ���굽ָ��λ��
}
void writefile(Inode fileInode, int fileInodeAddr, char buf[])
{ // ��buf����д���ļ��Ĵ��̿�
}
bool login()
{ // ��½����
    char username[10] = {0};
    char passwd[10] = {0};
    printf("username:");
    scanf("%s", username); // �û���
    printf("password:");
    scanf("%s", passwd);         // password
    if (check(username, passwd)) // �˶��û���������
        return isLogin = true;
    else
        return isLogin = false;
}
bool check(char username[], char passwd[])
{ // �˶��û���������
}
void gotoRoot()
{                                                      // return to the root directory
    memset(curUserName, 0, sizeof(curUserName));       // ��յ�ǰ�û���
    memset(curUserDirName, 0, sizeof(curUserDirName)); // ��յ�ǰ�û�Ŀ¼
    curDirAddr = rootDirAddr;                          // ��ǰ�û�Ŀ¼��ַ��Ϊ��Ŀ¼��ַ
    strcpy(curDirName, "/");
}
void logout()
{ // �û�ע��
    gotoRoot();
    isLogin = false;
    system("pause");
    system("cls");
}
bool useradd(char username[])
{ // �û�ע��
}
bool userdel(char username[])
{ // �û�ɾ��
}
void chmod(int parinoAddr, char name[], int pmode)
{ // �޸��ļ���Ŀ¼Ȩ��
}
void touch(int parinoAddr, char name[], char buf[])
{ // touch������ļ��������ַ�
}
void help()
{ // ��ʾ���������嵥
    printf("\n");
    printf("enter your command according ot the following prompts\n");
    printf("there are 10 main functions that you can choose...\n");
    printf("info - display the entire file system information\n");
    printf("cd - change directory\n");
    printf("dir - display directory\n");
    printf("md - make directory\n");
    printf("rd - remove directory\n");
    printf("newfile - create a new file\n");
    printf("cat - open a file\n");
    printf("copy - copy a file\n");
    printf("del -delete a file\n");
    printf("check - check and restore the file system\n");
    printf("\n");
}
void cmd(char str[])
{ // Process the input command
    char p1[100];
    char p2[100];
    char p3[100];
    char buf[100000];
    int tmp = 0;
    int i;
    sscanf(str, "%s", p1);       // Extracts a string from the string str and stores the extracted result in the character array p1
    if (strcmp(p1, "ls") == 0) { //
        ls(curDirAddr);          // ��ʾ��ǰĿ¼
    } else if (strcmp(p1, "cd") == 0) {
        sscanf(str, "%s%s", p1, p2);
        cd(curDirAddr, p2);
    } else if (strcmp(p1, "mkdir") == 0) {
        sscanf(str, "%s%s", p1, p2);
        mkdir(curDirAddr, p2);
    } else if (strcmp(p1, "rmdir") == 0) {
        sscanf(str, "%s%s", p1, p2);
        rmdir(curDirAddr, p2);
    } else if (strcmp(p1, "super") == 0) {
        infoOfSuperBlock();
    } else if (strcmp(p1, "inode") == 0) {
        printInodeBitmap();
    } else if (strcmp(p1, "block") == 0) {
        sscanf(str, "%s%s", p1, p2);
        tmp = 0;
        if ('0' <= p2[0] && p2[0] <= '9') {
            for (i = 0; p2[i]; i++)
                tmp = tmp * 10 + p2[i] - '0';
            printBlockBitmap(tmp);
        } else
            printBlockBitmap();
    } else if (strcmp(p1, "touch") == 0) {
        sscanf(str, "%s%s", p1, p2);
        touch(curDirAddr, p2, buf);     // ��ȡ���ݵ�buf
    } else if (strcmp(p1, "rm") == 0) { // ɾ��һ���ļ�
        sscanf(str, "%s%s", p1, p2);
        del(curDirAddr, p2);
    } else if (strcmp(p1, "cls") == 0) {
        system("cls");
    } else if (strcmp(p1, "logout") == 0) {
        logout();
    } else if (strcmp(p1, "useradd") == 0) {
        p2[0] = '\0';
        sscanf(str, "%s%s", p1, p2);
        if (strlen(p2) == 0)
            printf("��������\n");
        else
            useradd(p2);
    } else if (strcmp(p1, "userdel") == 0) {
        p2[0] = '\0';
        sscanf(str, "%s%s", p1, p2);
        if (strlen(p2) == 0)
            printf("��������\n");
        else
            userdel(p2);
    } else if (strcmp(p1, "chmod") == 0) {
        p2[0] = '\0';
        p3[0] = '\0';
        sscanf(str, "%s%s%s", p1, p2, p3);
        if (strlen(p2) == 0 || strlen(p3) == 0) {
            printf("��������\n");
        } else {
            tmp = 0;
            for (i = 0; p3[i]; i++)
                tmp = tmp * 8 + p3[i] - '0';
            chmod(Cur_Dir_Addr, p2, tmp);
        }
    } else if (strcmp(p1, "help") == 0) {
        help();
    } else if (strcmp(p1, "format") == 0) {
        if (strcmp(curUserName, "root") != 0) {
            printf("Ȩ�޲��㣺����ҪrootȨ��\n");
            return;
        }
        Ready();
        logout();
    } else if (strcmp(p1, "exit") == 0) {
        printf("�˳�MingOS\n");
        exit(0);
    } else
        printf("��Ǹ��û�и�����\n");
}