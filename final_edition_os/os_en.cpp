#include <cmath>
#include <cstring>
#include <fstream>
#include <io.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <windows.h>
using namespace std;
#define BUF_SIZE 4096
// example command:
// info
// dir
// dir /s
// md /root/123 (only absolute)
// rd /root/123 (only absolute)
// cd /root/123 (only absolute)
// newfile 123.txt 111111 helloworld
// cat /root/123.txt (only absolute)
// write /root/123.txt another (only absolute)
// del /root/123.txt (only absolute)
// copy C:\Users\CHaron\Desktop\os_filesystem\final_edition_os\123.txt /root host
// copy /root/123.txt /root/456.txt(premise:/root/456.txt have existed in system)
struct INode {             // size: 196B
    char FileName[8];      // File name
    int type;              // File type, 0 for directory, 1 for file
    int AdAccess[3];       // Administrator file access, representing read/write/execute access
    int UserAccess[3];     // Regular user file access, representing read/write/execute access
    char ParNode[32];      // Parent node, 32 represents the address bits of the parent node, all 0 means no parent node
    char ChildNode[3][32]; // Child nodes, files have no child nodes, each directory can have up to 3 child nodes
    char DiskAddress[32];  // Content corresponding to the file, directory content is empty by default

    INode() { // Default constructor
        // Set directory name as empty
        for (int i = 0; i < 8; i++)
            FileName[i] = 0;
        type = 0; // Initialized as directory
        // Set access as not readable, writable, and executable for both administrator and regular users
        for (int i = 0; i < 3; i++) {
            AdAccess[i] = 0;
            UserAccess[i] = 0;
        }
        // Set as no parent node and empty content
        for (int i = 0; i < 32; i++) {
            ParNode[i] = 0;
            DiskAddress[i] = 0;
        }
        // Set as no child nodes
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 32; j++)
                ChildNode[i][j] = 0;
    }

    INode(char* FN, int t, int* AdAcc, int* UserAcc, char* PNode, char** CNode, char* DAdd) {
        // Set file name
        for (int i = 0; i < 8; i++)
            FileName[i] = FN[i];
        type = t; // Set file type
        // Set access
        for (int i = 0; i < 3; i++) {
            AdAccess[i] = AdAcc[i];
            UserAccess[i] = UserAcc[i];
        }
        // Set parent node and disk block address
        for (int i = 0; i < 32; i++) {
            ParNode[i] = PNode[i];
            DiskAddress[i] = DAdd[i];
        }
        // Set child nodes
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 32; j++)
                ChildNode[i][j] = CNode[i][j];
    }
};
// Translation Functions
// int *Char2EightInt(char Char);     // Convert a symbol to an 8-bit binary integer array for input text
char EightIntToChar(int* EightInt); // Convert an 8-bit binary integer array to a symbol for output text

// int PointerSeq2BlockSeq(int PointerSeq); // Convert pointer sequence number to disk block sequence number
// 32-bit array int address <==> disk block address
int* BlockSeqToAdd(int BlockSeq); // Convert disk block sequence number to 32-bit address
int AddToBlockSeq(int* Add);      // Convert 32-bit address to disk block sequence number

// Utility Functions
// void Init(fstream& Disk_Pointer);                                      // Initialization function to initialize the hard disk
int GetBlockSeq(fstream& Disk_Pointer);                                // Get the current disk block sequence number for a specific pointer
int MatchDocName(fstream& Disk_Pointer, int BlockSeq, string DocName); // Compare the corresponding file name with all the sub-nodes' file names in a specific disk block. If there is a match, return the disk block sequence number; if not, return 0
void JumpPointer(fstream& Disk_Pointer, int BlockSeq, int BlockIn);    // Jump the pointer to the specified position. BlockSeq is the disk block sequence number and BlockIn is the block internal address
void Reply(string ReplyInfo, LPVOID& ShareMemoryPointer);              // Copy the reply information
void ShareMemoryClear(LPVOID& ShareMemoryPointer);                     // Clear the shared memory
void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer);          // Copy the content of shared memory
void BitMapChange(fstream& Disk_Pointer, int BlockSeq, int Tag);       // Set a certain value in the bitmap area to 1 or 0

// Input Functions
void InputIntArr(fstream& Disk_Pointer, int* IntArr, int Length);                               // Input binary numbers directly into the hard disk
void InputChar(fstream& Disk_Pointer, char Char);                                               // Convert a symbol to a binary number and input it into the hard disk
void InputCharArr(fstream& Disk_Pointer, char* CharArr);                                        // Convert a symbol string to binary numbers and input them into the hard disk
void InputDocName(fstream& Disk_Pointer, int BlockSeq, string DocName);                         // Input the file name into the hard disk, and return the pointer to the original disk block after input
void InputParNode(fstream& Disk_Pointer, int BlockSeq, int* IntArr);                            // Input the parent pointer into the hard disk, and return the pointer to the original disk block after input
void InputAcc(fstream& Disk_Pointer, int BlockSeq, string Acc);                                 // Input the protection type into the hard disk
void InputFileContext(fstream& Disk_Pointer, int BlockSeq, int NewDocBlockSeq, string Context); // Input the file content into the corresponding disk block address and input the file content into the corresponding file disk block
void InputFileType(fstream& Disk_Pointer, int BlockSeq, int FileType);                          // Input the file type into the disk

// Output Functions
int* Output32Bit(fstream& Disk_Pointer); // Output a 32-bit binary number
int* Output8Bit(fstream& Disk_Pointer);  // Output an 8-bit binary number

// Search Functions
int FindFreeAreaInINode(fstream& Disk_Pointer);                 // Find a free area in the bitmap for the i-node, return the disk block sequence number of the i-node, and return to the beginning of the original disk block after searching
int FindFreeAreaInDoc(fstream& Disk_Pointer);                   // Find a free area in the bitmap for the file, return the disk block sequence number of the file block, and return to the beginning of the original disk block after searching
int FindFreeChildNode(fstream& Disk_Pointer, int BlockSeq);     // Check if there is a free child node in a specific disk block. If yes, return the child node number (1/2/3); if not, return 0 and position to the beginning of the disk block
char* FindDocName(fstream& Disk_Pointer, int BlockSeq);         // Find the file name corresponding to a 32-bit address of a file disk block, return the file name array, and return to the beginning of the original disk block after searching
int FindDocType(fstream& Disk_Pointer, int BlockSeq);           // Find the file type corresponding to a disk block
int* FindDocSysAcc(fstream& Disk_Pointer, int BlockSeq);        // Find the system protection type corresponding to a disk block
int* FindDocComAcc(fstream& Disk_Pointer, int BlockSeq);        // Find the common user protection type corresponding to a disk block
int* FindDocParNodeAdd(fstream& Disk_Pointer, int BlockSeq);    // Find the parent node address corresponding to an i-node disk block
int** FindDocChildNodeAdd(fstream& Disk_Pointer, int BlockSeq); // Find the child node addresses corresponding to a disk block
int* FindDocContextNode(fstream& Disk_Pointer, int BlockSeq);   // Find the address of the content corresponding to a disk block

// Specialized Functions
// info
int GetINodeRemain(fstream& Disk_Pointer); // Get the remaining space of i-nodes
int GetDocRemain(fstream& Disk_Pointer);   // Get the remaining space of file area

// cd
string cdAddLocation(fstream& Disk_Pointer, string AddBuf); // Jump the pointer to the directory corresponding to the path and return the path; if the path is incorrect, return an error message

// dir
void ShowAllChildNodeName(fstream& Disk_Pointer, int BlockSeq, string& ReplyInfo); // Find the names of all sub-nodes of the target disk block and record the information in ReplyInfo
void ShowAllInfo(fstream& Disk_Pointer, int BlockSeq, string& ReplyInfo);          // Return all information of the corresponding disk block to ReplyInfo
int dirAddLocation(fstream& Disk_Pointer, string AddBuf);                          // Jump the pointer to the directory corresponding to the path; if the path is correct, return the disk block sequence number of the corresponding file; if the path is incorrect, return 0

// md
string mdAddLocation(fstream& Disk_Pointer, string AddBuf); // Jump the pointer to the directory corresponding to the path and return the file name to be created; if the path is incorrect, return an empty string

// rd
int IsDirEmpty(fstream& Disk_Pointer, int BlockSeq);     // Check if a directory disk block is empty
int rdAddLocation(fstream& Disk_Pointer, string AddBuf); // Jump the pointer to the directory corresponding to the path and return the path; if the path is incorrect, return an error message
void rdDir(fstream& Disk_Pointer, int BlockSeq);         // Delete a directory, including its sub-directories if any

// newfile
string FileNameTest(string FileName);                            // Check if a file name is in the correct format
string FileAccTest(string Acc);                                  // Check if the protection type is in the correct format
string newfileAddLocation(fstream& Disk_Pointer, string AddBuf); // Jump the pointer to the directory corresponding to the path and return the file name to be created; if the path is incorrect, return an empty string

// cat
int catAddLocation(fstream& Disk_Pointer, string AddBuf);   // Jump the pointer to the directory corresponding to the path and return the path; if the path is incorrect, return an error message
string GetFileContext(fstream& Disk_Pointer, int BlockSeq); // Get the content of the corresponding disk block

// copy
string GetWindowsFileName(string path);                        // 获取windows下一个文件的文件名
string CopyNewFile(fstream& Disk_Pointer, istringstream& Buf); // 在Copy指令下新建一个文件

// del
void delDoc(fstream& Disk_Pointer, int BlockSeq); // 删除一个文件（指定磁盘块）

// check
void ConsistencyCheck(fstream& Disk_Pointer, int BlockSeq); // 一致性检查

// Translation function
// int *CharToEightInt(char Char) { //eg:'a'==>{0,1,1,0,0,0,0,1}
//     int IntNum = int(Char);       // Convert the character to ASCII code
//     int *BiInt = new int[8];      // Output array
//     for (int i = 0; i < 8; i++) { // Convert to binary using division and remainder method
//         BiInt[7 - i] = IntNum % 2;
//         IntNum = IntNum / 2;
//     }
//     return BiInt;
// }

char EightIntToChar(int* EightInt) { // eg:{0,1,1,0,0,0,0,1}==>'a'
    int IntNum = 0;
    for (int i = 7; i >= 0; i--) // Accumulate the binary number
        IntNum += EightInt[i] * pow(2, 7 - i);
    return char(IntNum); // Return ASCII code value
}

int* BlockSeqToAdd(int BlockSeq) {
    int* Add = new int[32];
    for (int i = 0; i < 32; i++)
        Add[i] = 0;
    for (int i = 21; i >= 5; i--) { // Convert to binary using division and remainder method
        Add[i] = BlockSeq % 2;
        BlockSeq = BlockSeq / 2;
    }
    return Add;
}

int AddToBlockSeq(int* Add) {
    int BlockSeq = 0;
    for (int i = 21; i >= 5; i--) // Translate 17-bit binary number to disk block number
        BlockSeq += Add[i] * pow(2, 21 - i);
    return BlockSeq;
}

// Functional functions
int GetBlockSeq(fstream& Disk_Pointer) { // Convert pointer index to disk block number
    // tellg gets the current read position of the file stream and returns an integer value representing the file pointer position
    return Disk_Pointer.tellg() / 1024;
}

int MatchDocName(fstream& Disk_Pointer, int BlockSeq, string DocName) {           // Compare the corresponding file name with all child node file names in a specific disk block. If there is a match, return the disk block number; if there is no match, return 0.
    int OriSeq = GetBlockSeq(Disk_Pointer);                                       // Original disk block number of the pointer
    for (int i = 0; i < 3; i++) {                                                 // Loop 3 times to check the file names of child nodes
        JumpPointer(Disk_Pointer, BlockSeq, 112 + 32 * i);                        // Jump to the child node section
        int* BitDocName = Output32Bit(Disk_Pointer);                              // Read in 32-bit child node addresses
        char* CharDocName = FindDocName(Disk_Pointer, AddToBlockSeq(BitDocName)); // Find the file name of the child node
        if (string(CharDocName) == DocName) {                                     // Check if the file name of the child node matches
            JumpPointer(Disk_Pointer, OriSeq, 0);
            return AddToBlockSeq(BitDocName); // If it matches, return the disk block number of the child node
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return 0; // If it doesn't match, return 0
}

void JumpPointer(fstream& Disk_Pointer, int BlockSeq, int BlockIn) {
    Disk_Pointer.seekg(1024 * BlockSeq + BlockIn, ios::beg); // The seekg function is used to set the file pointer position, and the ios::beg parameter indicates offset relative to the beginning of the file
}

void Reply(string ReplyInfo, LPVOID& ShareMemoryPointer) {
    strcpy_s((char*)ShareMemoryPointer, strlen(ReplyInfo.c_str()) + 1, ReplyInfo.c_str()); // Send error message
    cout << "Return Info:" << ReplyInfo << endl;
}

// Clear shared memory
void ShareMemoryClear(LPVOID& ShareMemoryPointer) {
    char NULLChar[] = "";
    strcpy_s((char*)ShareMemoryPointer, strlen(NULLChar) + 1, NULLChar);
}

void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer) {
    strcpy_s(Buffer, strlen((char*)ShareMemoryPointer) + 1, (char*)ShareMemoryPointer);
    ShareMemoryClear(ShareMemoryPointer);
}

void BitMapChange(fstream& Disk_Pointer, int BlockSeq, int Tag) { // Set a specific value in the bitmap area to 1 or 0
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, 1, BlockSeq); // The bitmap starts from the first block
    Disk_Pointer << Tag;
    JumpPointer(Disk_Pointer, OriSeq, 0); // Restore
}

// Input functions
void InputCharArr(fstream& Disk_Pointer, char* CharArr) {
    for (int i = 0; i < strlen(CharArr); i++)
        InputChar(Disk_Pointer, CharArr[i]);
}

void InputChar(fstream& Disk_Pointer, char Char) {
    int* InputInt = new int[8];
    int IntNum = int(Char);
    for (int i = 0; i < 8; i++) {
        InputInt[7 - i] = IntNum % 2;
        IntNum /= 2;
    }
    InputIntArr(Disk_Pointer, InputInt, 8);
}

void InputIntArr(fstream& Disk_Pointer, int* IntArr, int Length) {
    for (int i = 0; i < Length; i++)
        Disk_Pointer << IntArr[i];
    return;
}

void InputDocName(fstream& Disk_Pointer, int BlockSeq, string DocName) {
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSeq, 0);
    int DocNameLength = DocName.length();
    char* CharDocName = new char[8];
    for (int i = 0; i < 8; i++) CharDocName[i] = '0';
    InputCharArr(Disk_Pointer, CharDocName);
    JumpPointer(Disk_Pointer, BlockSeq, 0);
    InputCharArr(Disk_Pointer, const_cast<char*>(DocName.c_str()));
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

void InputParNode(fstream& Disk_Pointer, int BlockSeq, int* IntArr) {
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSeq, 80);
    InputIntArr(Disk_Pointer, IntArr, 32);
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

void InputAcc(fstream& Disk_Pointer, int BlockSeq, string Acc) {
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSeq, 72);
    int* TempInputIntArr = new int[8];// TempInputIntArr={0, , , ,0, , , }
    for (int i = 0; i < 8; i++)
        TempInputIntArr[i] = 0;
    for (int i = 1; i <= 3; i++)
        TempInputIntArr[i] = Acc[i - 1] - 48;// admin
    for (int i = 5; i <= 7; i++)
        TempInputIntArr[i] = Acc[i - 2] - 48;// user
    InputIntArr(Disk_Pointer, TempInputIntArr, 8);
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

void InputFileContext(fstream& Disk_Pointer, int BlockSeq, int NewDocBlockSeq, string Context) {
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSeq, 208);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(NewDocBlockSeq), 32);
    JumpPointer(Disk_Pointer, NewDocBlockSeq, 0);
    char* arr = new char[128];
    for (int i = 0; i < 128; i++)
        arr[i] = '0';
    InputCharArr(Disk_Pointer, arr);
    JumpPointer(Disk_Pointer, NewDocBlockSeq, 0);
    InputCharArr(Disk_Pointer, const_cast<char*>(Context.c_str()));
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

void InputFileType(fstream& Disk_Pointer, int BlockSeq, int FileType) {
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSeq, 71);
    Disk_Pointer << FileType;
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

// Output functions
int* Output32Bit(fstream& Disk_Pointer) {
    int* Output32Bit = new int[32];
    char TempChar;//'0' or '1'
    for (int i = 0; i < 32; i++) {
        Disk_Pointer >> TempChar;
        Output32Bit[i] = int(TempChar) - 48;
    }
    return Output32Bit;
}

int* Output8Bit(fstream& Disk_Pointer) {
    int* Output8Bit = new int[8];
    char TempChar;
    for (int i = 0; i < 8; i++) {
        Disk_Pointer >> TempChar;
        Output8Bit[i] = int(TempChar) - 48;
    }
    return Output8Bit;
}

// Find function
int FindFreeAreaInINode(fstream& Disk_Pointer) { // Used to find the free area of the corresponding i-node in the bitmap, and return the disk block number of the i-node
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, 1, 102); // The first block is the bitmap
    for (int i = 102; i < 50102; i++) { // Traverse the portion of the i-node in the bitmap area
        char TempChar;
        Disk_Pointer >> TempChar;
        if (TempChar == '0') {
            JumpPointer(Disk_Pointer, OriSeq, 0);
            return i;
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0); // After the search is completed, return the pointer to the beginning of the original disk block
    return 0;                             // If not found, return 0
}

int FindFreeAreaInDoc(fstream& Disk_Pointer) { // Used to find the free area of the corresponding file in the bitmap and return the disk block number corresponding to the file block. After the search is completed, return the pointer to the beginning of the original disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, 1, 50102);
    for (int i = 50102; i < 100102; i++) { // Traverse the part of the file in the bitmap area
        char TempChar;
        Disk_Pointer >> TempChar;
        if (TempChar == '0') {
            JumpPointer(Disk_Pointer, OriSeq, 0);
            return i;
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return 0; // If not found, return 0
}

int FindFreeChildNode(fstream& Disk_Pointer, int BlockSeq) { // Determine whether there is a free sub-node in a specific disk block. If yes, return the sub-node number (1/2/3); if not, return 0 and locate it at the beginning of the disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char IsFree;

    for (int i = 1; i <= 3; i++) { // Traverse each sub-node in sequence
        JumpPointer(Disk_Pointer, BlockSeq, 112 + 32 * (i - 1));
        Disk_Pointer >> IsFree;
        if (IsFree == '0') {                  // If the sub-node is not in use
            Disk_Pointer.seekg(-1, ios::cur); // Move back one bit
            JumpPointer(Disk_Pointer, OriSeq, 0);
            return i; // Return the sub-node number
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return 0;
}

char* FindDocName(fstream& Disk_Pointer, int BlockSeq) { // Find the file name corresponding to the file disk block corresponding to a certain 32-bit address, return the file name array, and after the search is completed, return the pointer to the beginning of the original disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char* DocName; // The file name to be returned
    int num = 0; // The length of the file name
    JumpPointer(Disk_Pointer, BlockSeq, 0);
    while (EightIntToChar(Output8Bit(Disk_Pointer)) != '0') num++;
    DocName = new char[num];
    int* TempIntArr; // Used to load the numbers for translation
    JumpPointer(Disk_Pointer, BlockSeq, 0); // Jump to the corresponding position
    for (int i = 0; i < num; i++) {
        TempIntArr = Output8Bit(Disk_Pointer);
        DocName[i] = EightIntToChar(TempIntArr); // Convert the corresponding binary array to a symbol
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);

    return DocName;                       // Return the file name
}

int FindDocType(fstream& Disk_Pointer, int BlockSeq) { // Find the file type corresponding to a certain disk block (return 0 for directory and 1 for file)
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    JumpPointer(Disk_Pointer, BlockSeq, 71);
    Disk_Pointer >> TempChar;
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return int(TempChar) - 48; // 0 or 1
}

int* FindDocSysAcc(fstream& Disk_Pointer, int BlockSeq) { // Find the system protection type corresponding to a certain disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    int* SysAcc = new int[3];
    JumpPointer(Disk_Pointer, BlockSeq, 73);
    for (int i = 0; i < 3; i++) {
        Disk_Pointer >> TempChar;
        SysAcc[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return SysAcc;
}

int* FindDocComAcc(fstream& Disk_Pointer, int BlockSeq) { // Find the common user protection type corresponding to a certain disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    int* ComAcc = new int[3];
    JumpPointer(Disk_Pointer, BlockSeq, 77);
    for (int i = 0; i < 3; i++) {
        Disk_Pointer >> TempChar;
        ComAcc[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return ComAcc;
}

int* FindDocParNodeAdd(fstream& Disk_Pointer, int BlockSeq) { // Find the address of the parent node corresponding to a certain i-node disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    int* ParNodeAdd = new int[32];
    JumpPointer(Disk_Pointer, BlockSeq, 80);
    for (int i = 0; i < 32; i++) {
        Disk_Pointer >> TempChar;
        ParNodeAdd[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return ParNodeAdd;
}

int** FindDocChildNodeAdd(fstream& Disk_Pointer, int BlockSeq) { // Find the address of the child node corresponding to a certain disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    int** ChildNodeAdd;
    ChildNodeAdd = new int* [3];
    for (int i = 0; i < 3; i++)
        ChildNodeAdd[i] = new int[32];
    JumpPointer(Disk_Pointer, BlockSeq, 112);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 32; j++) {
            Disk_Pointer >> TempChar;
            ChildNodeAdd[i][j] = TempChar - 48;
        }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return ChildNodeAdd;
}

int* FindDocContextNode(fstream& Disk_Pointer, int BlockSeq) { // Find the address of the content (file content address corresponding to the i-node) corresponding to a certain disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    int* ParNodeAdd;
    ParNodeAdd = new int[32];
    JumpPointer(Disk_Pointer, BlockSeq, 208);
    for (int i = 0; i < 32; i++) {
        Disk_Pointer >> TempChar;
        ParNodeAdd[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return ParNodeAdd;
}

// Utility functions
// info
int GetINodeRemain(fstream& Disk_Pointer) { // Get the remaining space of i-nodes (traverse bitmap)
    int OriSeq = GetBlockSeq(Disk_Pointer);
    int CountRemain = 0;
    JumpPointer(Disk_Pointer, 1, 102); // The first 100 blocks are used for bitmap
    char TempChar;
    for (int i = 102; i < 50102; i++) {
        Disk_Pointer >> TempChar;
        if (TempChar == '0')
            CountRemain++;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return CountRemain;
}

int GetDocRemain(fstream& Disk_Pointer) { // Get the remaining space of files (traverse bitmap)
    int OriSeq = GetBlockSeq(Disk_Pointer);
    int CountRemain = 0;
    JumpPointer(Disk_Pointer, 1, 50102);
    char TempChar;
    for (int i = 50102; i < 100102; i++) {
        Disk_Pointer >> TempChar;
        if (TempChar == '0')
            CountRemain++;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return CountRemain;
}

// cd   eg:cd /root/123
string cdAddLocation(fstream& Disk_Pointer, string AddBuf) { // Jump the pointer to the corresponding directory of the path and return the path; if the path is incorrect, return an error message.
    int OriSeq = GetBlockSeq(Disk_Pointer);
    string OriAddBuf = AddBuf;
    string CurDirName;        // Used to temporarily store the current directory.
    if (AddBuf.length() == 0) // If the address is empty, return an error message.
        return "Jump address missing!";
    if (AddBuf[0] == '/') {
        JumpPointer(Disk_Pointer, 101, 0); // Start searching from the root node.
        /* Preprocess the / symbol in the input command*/
        for (int i = 0; i < AddBuf.length(); i++) // Process the / symbol in the address first.
            if (AddBuf[i] == '/')
                AddBuf[i] = ' '; // Replace / with a space to facilitate input.

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /* Determine if the root address is incorrect.*/
        if (CurDirName != "root") {               // If the root address is incorrect
            JumpPointer(Disk_Pointer, OriSeq, 0); // Jump back to the original position.
            return "Address error!";
        }
        if (AddStr.eof()) {
            JumpPointer(Disk_Pointer, 101, 0); // Root directory.
            return "/root";
        }
        /* Locate it continuously through the child nodes */
        while (true) { // When there is still content in the input string AddressBuf
            AddStr >> CurDirName; // Input the next directory.

            int ChildBlockSeq = MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName);
            if (ChildBlockSeq) { // If the corresponding child file is found
                if (FindDocType(Disk_Pointer, GetBlockSeq(Disk_Pointer)) != 0)
                    return "Address error!";
                JumpPointer(Disk_Pointer, ChildBlockSeq, 0); // Jump to the corresponding position.
            }
            else {                                         // If there is no corresponding child file
                JumpPointer(Disk_Pointer, OriSeq, 0);        // Jump back to the original position.
                return "Address error!";
            }
            if (AddStr.eof()) // End when the bottom of a directory is reached after entering a directory.
                return OriAddBuf;
        }
    }
    else
        return "Address error!";
}

// dir
void ShowAllChildNodeName(fstream& Disk_Pointer, int BlockSeq, string& ReplyInfo) { // Used to find the names of all child nodes of the target disk block and enter the corresponding information into ReplyInfo.
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char* ChildNodeName;
    int* ChildNodeAdd;
    int FileType;

    for (int i = 1; i <= 3; i++) { // Traverse child nodes
        JumpPointer(Disk_Pointer, BlockSeq, 112 + 32 * (i - 1));
        ChildNodeAdd = Output32Bit(Disk_Pointer);
        FileType = FindDocType(Disk_Pointer, AddToBlockSeq(ChildNodeAdd));
        if (ChildNodeAdd[0] == 1) { // If this child node is occupied
            int ChildNodeSeq = AddToBlockSeq(ChildNodeAdd);
            ChildNodeName = FindDocName(Disk_Pointer, ChildNodeSeq);
            ReplyInfo = ReplyInfo + "\n" + string(ChildNodeName);
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

void ShowAllInfo(fstream& Disk_Pointer, int BlockSeq, string& ReplyInfo) { // Return all information of the corresponding disk block to ReplyInfo.
    char TempChar;
    int OriSeq = GetBlockSeq(Disk_Pointer);
    ReplyInfo = ReplyInfo + "\n" + "Directory name: " + string(FindDocName(Disk_Pointer, BlockSeq));

    JumpPointer(Disk_Pointer, BlockSeq, 73); // Jump to the administrator protection type position
    ReplyInfo = ReplyInfo + "\n" + "Administrator protection type: ";
    for (int i = 0; i < 3; i++) { // Read administrator protection type
        Disk_Pointer >> TempChar;
        ReplyInfo += TempChar;
    }

    JumpPointer(Disk_Pointer, BlockSeq, 77); // Jump to the user protection type position
    ReplyInfo = ReplyInfo + "\n" + "User protection type: ";
    for (int i = 0; i < 3; i++) { // Read user protection type
        Disk_Pointer >> TempChar;
        ReplyInfo += TempChar;
    }

    JumpPointer(Disk_Pointer, BlockSeq, 80); // Jump to the parent node position
    ReplyInfo = ReplyInfo + "\n" + "Parent node address: ";
    for (int i = 0; i < 32; i++) { // Read parent node address
        Disk_Pointer >> TempChar;
        ReplyInfo += TempChar;
    }

    JumpPointer(Disk_Pointer, BlockSeq, 112); // Jump to the child node position
    ReplyInfo = ReplyInfo + "\n" + "Child node address: ";
    for (int i = 0; i < 3; i++) { // Read child node address
        ReplyInfo += "\n";
        for (int j = 0; j < 32; j++) {
            Disk_Pointer >> TempChar;
            ReplyInfo += TempChar;
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
}

int dirAddLocation(fstream& Disk_Pointer, string AddBuf) { // Jump the pointer to the corresponding path directory; If the path is correct, return the disk block number of the corresponding file; If the path is incorrect, return 0.
    string CurDirName;        // Used to temporarily store the current location
    if (AddBuf.length() == 0) // If the address is empty, return the current disk block number
        return GetBlockSeq(Disk_Pointer);

    if (AddBuf[0] == '/') {
        JumpPointer(Disk_Pointer, 101, 0); // Start checking from the root node
        /*Preprocess the input instruction delimiter '/'*/
        for (int i = 0; i < AddBuf.length(); i++) // Process the '/' symbol in the address first
            if (AddBuf[i] == '/')
                AddBuf[i] = ' '; // Replace '/' with a space for input convenience

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*Check if the root address is correct*/
        if (CurDirName != "root") // If the root address is wrong
            return 0;
        /*Continuously locate through child nodes*/
        while (true) {               // When there is still content in the input string AddressBuf
            AddStr >> CurDirName; // Enter the next directory
            int ChildBlockSeq = MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName);
            if (ChildBlockSeq)                               // If the corresponding child file is found
                JumpPointer(Disk_Pointer, ChildBlockSeq, 0); // Jump to the corresponding position
            else                                             // If no corresponding child file is found
                return 0;
            if (AddStr.eof()) // When reaching the bottom after entering a directory, end
                return GetBlockSeq(Disk_Pointer);
        }
    }
    else
        return 0;
}

// md
string mdAddLocation(fstream& Disk_Pointer, string AddBuf) // eg:md /root/123
{                                                          // Jump the pointer to the corresponding path directory and return the file name to be created. If the path is incorrect, return an empty string.
    string ErrorReturn = "";
    string CurDirName;        // Used to temporarily store the current location
    if (AddBuf.length() == 0) // If the address is empty, error
        return ErrorReturn;

    /*Check whether the given address is an absolute address. If it is, jump to the corresponding position and return the file name to be created. If not, directly return the file name to be created.*/
    if (AddBuf[0] == '/') {
        JumpPointer(Disk_Pointer, 101, 0); // Start checking from the root node
        /*Preprocess the input instruction delimiter '/'*/
        for (int i = 0; i < AddBuf.length(); i++) // Process the '/' symbol in the address first
            if (AddBuf[i] == '/')
                AddBuf[i] = ' '; // Replace '/' with a space for input convenience

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*Check if the root address is correct*/
        if (CurDirName != "root") // If the root address is wrong
            return ErrorReturn;

        /*Continuously locate through child nodes*/
        while (true) {               // When there is still content in the input string AddressBuf
            AddStr >> CurDirName; // Enter the next directory
            if (AddStr.eof())     // When reaching the bottom after entering a directory, the name at this time is the file name to be created
                break;
            int ChildBlockSeq = MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName);
            if (ChildBlockSeq) {                                   // If the corresponding child file is found
                if (FindDocType(Disk_Pointer, ChildBlockSeq) != 0) // If the child file is not a directory
                    return ErrorReturn;
                JumpPointer(Disk_Pointer, ChildBlockSeq, 0); // Jump to the corresponding position
            }
            else                                           // If no corresponding child file is found
                return ErrorReturn;
        }
    }
    else {  // Directly return the file name to be created
        for (int i = 1; i < AddBuf.length(); i++) // Check if there is a '/' symbol in the file name
            if (AddBuf[i] == '/')
                return ErrorReturn;
        CurDirName = AddBuf;
    }
    for (int i = 0; i < CurDirName.length(); i++)
        if (CurDirName[i] == '.') // Do not allow "."
            return ErrorReturn;
    return CurDirName;
}

// rd   eg:rd /root/123
int IsDirEmpty(fstream& Disk_Pointer, int BlockSeq) { // Check if a directory block is empty (whether the directory inode contains files or not, return 0 if there are files, return 1 if empty)
    int OriSeq = GetBlockSeq(Disk_Pointer);
    char TempChar;
    for (int i = 0; i < 3; i++) {
        JumpPointer(Disk_Pointer, BlockSeq, 112 + 32 * i);
        Disk_Pointer >> TempChar;
        if (TempChar == '1') { // If it's a file
            JumpPointer(Disk_Pointer, OriSeq, 0);
            return 0;
        }
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return 1;
}

int rdAddLocation(fstream& Disk_Pointer, string AddBuf) { // Jump the pointer to the corresponding directory according to the path and return the path; If the path is incorrect, return an error message.
    string CurDirName;                                  // Used to temporarily store the current location
    if (AddBuf.length() == 0)                           // If the address is empty, return the current block number.
        return GetBlockSeq(Disk_Pointer);

    if (AddBuf[0] == '/') {
        JumpPointer(Disk_Pointer, 101, 0); // Start searching from the root node
        /* Preprocess the command by replacing / with spaces */
        for (int i = 0; i < AddBuf.length(); i++) // Process the / symbol in the address first
            if (AddBuf[i] == '/')
                AddBuf[i] = ' '; // Replace / with a space for ease of input

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /* Check if the root address is correct */
        if (CurDirName != "root") // If the root address is incorrect
            return 0;
        /* Continuously locate through sub-nodes */
        while (true) {               // When there is still content in the input string AddressBuf
            AddStr >> CurDirName; // Input the next directory
            int ChildBlockSeq = MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName);
            if (ChildBlockSeq)                               // If the corresponding child file is found
                JumpPointer(Disk_Pointer, ChildBlockSeq, 0); // Jump to the corresponding position
            else                                             // If the corresponding child file is not found
                return 0;
            if (AddStr.eof()) {                                                // When reaching the end after inputting a directory
                if (FindDocType(Disk_Pointer, GetBlockSeq(Disk_Pointer)) != 0) // If it is 1, it is a file
                    return 0;
                return GetBlockSeq(Disk_Pointer); // If it is 0, it is a directory
            }
        }
    }
    else
        return 0;
}

void rdDir(fstream& Disk_Pointer, int BlockSeq) {  // Delete a directory and its subdirectories if they exist
    if (FindDocType(Disk_Pointer, BlockSeq) == 0) { // If it is a directory
        if (IsDirEmpty(Disk_Pointer, BlockSeq)) {   // If the directory is empty
            /* Clear the corresponding child node address of the parent node */
            int* ParNode = FindDocParNodeAdd(Disk_Pointer, BlockSeq);
            char* DocName = FindDocName(Disk_Pointer, BlockSeq);
            int** ParChildNode = FindDocChildNodeAdd(Disk_Pointer, AddToBlockSeq(ParNode));
            if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[0])))) {
                JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112);
                for (int i = 0; i < 32; i++)
                    Disk_Pointer << 0;
            }
            else if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[1])))) {
                JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112 + 32);
                for (int i = 0; i < 32; i++)
                    Disk_Pointer << 0;
            }
            else if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[2])))) {
                JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112 + 64);
                for (int i = 0; i < 32; i++)
                    Disk_Pointer << 0;
            }

            /* Clear the content of the directory i-node */
            JumpPointer(Disk_Pointer, BlockSeq, 0);
            BitMapChange(Disk_Pointer, GetBlockSeq(Disk_Pointer), 0); // Clear the corresponding position in the bitmap for i-node
            for (int i = 0; i < 1024; i++)
                Disk_Pointer << 0;
            return;
        }
        else {   // If the directory is not empty
            int** TempIntArr = FindDocChildNodeAdd(Disk_Pointer, BlockSeq); // Child node addresses corresponding to the disk block
            /* Delete child nodes */
            for (int i = 0; i <= 2; i++)
                if (TempIntArr[i][0] == 1)
                    rdDir(Disk_Pointer, AddToBlockSeq(TempIntArr[i]));

            if (IsDirEmpty(Disk_Pointer, BlockSeq)) /* Delete itself */
                rdDir(Disk_Pointer, BlockSeq);
        }
    }
    else { // If it is a file
        int* DocContextAdd = FindDocContextNode(Disk_Pointer, BlockSeq);

        /* Clear the content of the file node */
        BitMapChange(Disk_Pointer, AddToBlockSeq(DocContextAdd), 0);
        JumpPointer(Disk_Pointer, AddToBlockSeq(DocContextAdd), 0);
        for (int i = 0; i < 1024; i++)
            Disk_Pointer << 0;

        /* Clear the corresponding child node address of the parent node */
        int* ParNode = FindDocParNodeAdd(Disk_Pointer, BlockSeq);
        char* DocName = FindDocName(Disk_Pointer, BlockSeq);
        int** ParChildNode = FindDocChildNodeAdd(Disk_Pointer, AddToBlockSeq(ParNode));
        if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[0])))) {
            JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112);
            for (int i = 0; i < 32; i++)
                Disk_Pointer << 0;
        }
        else if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[1])))) {
            JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112 + 32);
            for (int i = 0; i < 32; i++)
                Disk_Pointer << 0;
        }
        else if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[2])))) {
            JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112 + 64);
            for (int i = 0; i < 32; i++)
                Disk_Pointer << 0;
        }

        /* Clear the content of the file i-node */
        BitMapChange(Disk_Pointer, BlockSeq, 0);
        JumpPointer(Disk_Pointer, BlockSeq, 0);
        for (int i = 0; i < 240; i++)
            Disk_Pointer << 0;
        return;
    }
}

// newfile(eg:newfile 123.txt 111111 Iamastring!)
string FileNameTest(string FileName) { // Check if the format of a file name is correct
    string Right = "";
    if (FileName.length() == 0 || FileName.length() > 8)
        return "Invalid address or file name is too long, failed to create!";
    int CountPoint = 0;
    for (int i = 0; i < FileName.length(); i++) {
        if (FileName[i] == '.') // eg:123.txt, the dot in the middle
            CountPoint++;
        if (FileName[i] == '*' || FileName[i] == '/')
            return "Invalid file name format, failed to create!";
    }
    if (CountPoint != 1)
        return "Invalid file name format, failed to create!";
    return Right;
}

string FileAccTest(string Acc) { // Check if the format of protection type is correct (should be a 6-character string containing only 0s and 1s)
    string Right = "";
    if (Acc.length() != 6)
        return "Invalid protection type length, failed to create!";
    for (int i = 0; i < Acc.length(); i++)
        if (Acc[i] != '0' && Acc[i] != '1')
            return "Invalid protection type format, failed to create!";
    return Right;
}

string newfileAddLocation(fstream& Disk_Pointer, string AddBuf) { // Jump the pointer to the corresponding path directory and return the file name to be created; if the path is incorrect, return an empty string
    string ErrorReturn = "";
    string CurDirName;        // Temporarily store the current position
    if (AddBuf.length() == 0) // If the address is empty, return an error
        return ErrorReturn;

    /* Check if the given address is an absolute address, if yes, jump to the corresponding position and return the file name to be created; if not, directly return the file name to be created*/
    if (AddBuf[0] == '/') {
        JumpPointer(Disk_Pointer, 101, 0); // Start searching from the root node
        /* Preprocess the input instruction using '/' as the delimiter */
        for (int i = 0; i < AddBuf.length(); i++) // Process the '/' symbol in the address first
            if (AddBuf[i] == '/')
                AddBuf[i] = ' '; // Replace '/' with a space for easier input

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /* Check if the root address is correct*/
        if (CurDirName != "root") // If the root address is incorrect
            return ErrorReturn;

        /* Continuously locate through child nodes */
        while (true) {               // When there are still contents in the input string AddressBuf
            AddStr >> CurDirName; // Enter the next directory
            if (AddStr.eof())     // When reaching the end of the input after entering a directory, then the name is the file name to be created
                break;
            int ChildBlockSeq = MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName);
            if (ChildBlockSeq) {                                   // If the corresponding child file is found
                if (FindDocType(Disk_Pointer, ChildBlockSeq) != 0) // If the child file is not a directory
                    return ErrorReturn;
                JumpPointer(Disk_Pointer, ChildBlockSeq, 0); // Jump to the corresponding position
            }
            else                                           // If the corresponding child file is not found
                return ErrorReturn;
        }
    }
    else
        for (int i = 1; i < AddBuf.length(); i++) // Check if there is a '/' symbol in the file name
            if (AddBuf[i] == '/')                 // '/' is not allowed
                return ErrorReturn;
    CurDirName = AddBuf;
    return CurDirName;
}

// cat (eg:cat /root/123.txt)
int catAddLocation(fstream& Disk_Pointer, string AddBuf) { // Jump the pointer to the corresponding path directory and return the path; if the path is incorrect, return an error message
    string CurDirName;        // Temporarily store the current position
    if (AddBuf.length() == 0) // If the address is empty, return the current block number
        return 0;

    if (AddBuf[0] == '/') {
        JumpPointer(Disk_Pointer, 101, 0); // Start searching from the root node
        /* Preprocess the input instruction using '/' as the delimiter */
        for (int i = 0; i < AddBuf.length(); i++) // Process the '/' symbol in the address first
            if (AddBuf[i] == '/')
                AddBuf[i] = ' '; // Replace '/' with a space for easier input

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /* Check if the root address is correct*/
        if (CurDirName != "root") // If the root address is incorrect
            return 0;
        /* Continuously locate through child nodes */
        while (true) { // When there are still contents in the input string AddressBuf
            AddStr >> CurDirName; // Enter the next directory
            int ChildBlockSeq = MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName);
            if (ChildBlockSeq)                               // If the corresponding child file is found
                JumpPointer(Disk_Pointer, ChildBlockSeq, 0); // Jump to the corresponding position
            else                                             // If the corresponding child file is not found
                return 0;
            if (AddStr.eof()) {                                                // When reaching the end of the input after entering a directory, end
                if (FindDocType(Disk_Pointer, GetBlockSeq(Disk_Pointer)) != 1) // If the file is a directory
                    return 0;
                return GetBlockSeq(Disk_Pointer);
            }
        }
    }
    else
        return 0;
}

string GetFileContext(fstream& Disk_Pointer, int BlockSeq) { // Get the content of the corresponding disk block
    int OriSeq = GetBlockSeq(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSeq, 0);
    string FileContext = "";
    char TempChar;
    int* TempOutputIntArr;
    for (int i = 0; i < 128; i++) {
        TempOutputIntArr = Output8Bit(Disk_Pointer); // 8 bits
        TempChar = EightIntToChar(TempOutputIntArr); // 1 char character
        if (TempChar != '0')
            FileContext += TempChar;
    }
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return FileContext;
}

// copy(eg:copy E:\Desktop\456.txt /root host)
string GetWindowsFileName(string path) { // Get the file name of a file on Windows
    for (int i = 0; i < path.length(); i++)
        if (path[i] == '\\')
            path[i] = ' ';
    istringstream TempInputStr(path);
    string TempStr;
    while (!TempInputStr.eof())
        TempInputStr >> TempStr;
    return TempStr;
}
// eg:copy /root/123.txt /root/456.txt(eg:newfile 123.txt 111111 Iamastring!)
string CopyNewFile(fstream& Disk_Pointer, istringstream& Buf) { // Create a new file under the Copy command
    int OriSeq = GetBlockSeq(Disk_Pointer);
    string newfileNameBuf; // Store the address part of the input content by the user
    string newfileAccBuf;
    string newfileContextBuf;
    Buf >> newfileNameBuf;    // eg:123.txt
    Buf >> newfileAccBuf;     // eg:111111
    Buf >> newfileContextBuf; // eg:Iamastring!

    string CurDirName = newfileAddLocation(Disk_Pointer, newfileNameBuf);
    /*File name format detection*/
    string ReplyInfo = FileNameTest(CurDirName);
    if (ReplyInfo != "")
        return ReplyInfo;

    /*Protection type format detection*/
    ReplyInfo = FileAccTest(newfileAccBuf);
    if (ReplyInfo != "")
        return ReplyInfo;

    int NewINodeBlockSeq = FindFreeAreaInINode(Disk_Pointer);
    int NewDocBlockSeq = FindFreeAreaInDoc(Disk_Pointer);

    /*Check if the file system still has space*/
    if (!NewINodeBlockSeq) // If the file system does not have space, creation fails
        return "No free space in the file system, creation failed!";
    if (!NewDocBlockSeq) // If the file system does not have space, creation fails
        return "No free space in the file system, creation failed!";

    /*Check if the created file name duplicates with the file names in the child nodes*/
    if (MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName))
        return "Duplicate file name, creation failed!";

    /*Check if the child node is full*/
    int FreeChildSeq = FindFreeChildNode(Disk_Pointer, GetBlockSeq(Disk_Pointer));
    if (FreeChildSeq) {                                                                      // Modify
        JumpPointer(Disk_Pointer, GetBlockSeq(Disk_Pointer), 112 + 32 * (FreeChildSeq - 1)); // Jump to the corresponding child node
        int* INode = BlockSeqToAdd(NewINodeBlockSeq);
        INode[0] = 1;                                            // Set the first bit to 1, indicating that the child node has been used
        InputIntArr(Disk_Pointer, INode, 32);                    // Enter 32 bits of the child node
        JumpPointer(Disk_Pointer, GetBlockSeq(Disk_Pointer), 0); // Jump back to the beginning
    }
    else
        return "The number of child files of the current file has reached the upper limit of 3, creation failed!";

    /*Initialize the new file*/
    InputDocName(Disk_Pointer, NewINodeBlockSeq, CurDirName);                               // Enter the file name
    InputParNode(Disk_Pointer, NewINodeBlockSeq, BlockSeqToAdd(GetBlockSeq(Disk_Pointer))); // Enter the parent node address
    InputAcc(Disk_Pointer, NewINodeBlockSeq, newfileAccBuf);                                // Enter the file protection type
    InputFileContext(Disk_Pointer, NewINodeBlockSeq, NewDocBlockSeq, newfileContextBuf);    // Enter the file content
    InputFileType(Disk_Pointer, NewINodeBlockSeq, 1);                                       // Enter the file type
    BitMapChange(Disk_Pointer, NewINodeBlockSeq, 1);
    BitMapChange(Disk_Pointer, NewDocBlockSeq, 1);
    JumpPointer(Disk_Pointer, OriSeq, 0);
    return "File created successfully!";
}

// del(eg: del /root/123.txt)
void delDoc(fstream& Disk_Pointer, int BlockSeq) { // Delete a file (specified disk block)
    int* DocContextAdd = FindDocContextNode(Disk_Pointer, BlockSeq);

    /*Clear the content of the file node*/
    BitMapChange(Disk_Pointer, AddToBlockSeq(DocContextAdd), 0);
    JumpPointer(Disk_Pointer, AddToBlockSeq(DocContextAdd), 0);
    for (int i = 0; i < 1024; i++)
        Disk_Pointer << 0;

    /*Clear the corresponding child node address of the parent node*/
    int* ParNode = FindDocParNodeAdd(Disk_Pointer, BlockSeq);
    char* DocName = FindDocName(Disk_Pointer, BlockSeq);
    int** ParChildNode = FindDocChildNodeAdd(Disk_Pointer, AddToBlockSeq(ParNode));

    if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[0])))) {
        JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112);
        for (int i = 0; i < 32; i++)
            Disk_Pointer << 0;
    }
    else if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[1])))) {
        JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112 + 32);
        for (int i = 0; i < 32; i++)
            Disk_Pointer << 0;
    }
    else if (string(DocName) == string(FindDocName(Disk_Pointer, AddToBlockSeq(ParChildNode[2])))) {
        JumpPointer(Disk_Pointer, AddToBlockSeq(ParNode), 112 + 64);
        for (int i = 0; i < 32; i++)
            Disk_Pointer << 0;
    }

    /*Clear the content of the file i node*/
    BitMapChange(Disk_Pointer, BlockSeq, 0);
    JumpPointer(Disk_Pointer, BlockSeq, 0);
    for (int i = 0; i < 240; i++)
        Disk_Pointer << 0;
    return;
}

// check
void ConsistencyCheck(fstream& Disk_Pointer, int BlockSeq) { // Consistency check
    JumpPointer(Disk_Pointer, BlockSeq, 0);
    int FileType = FindDocType(Disk_Pointer, BlockSeq);
    BitMapChange(Disk_Pointer, BlockSeq, 1);                                                      // Set the corresponding bit in the bitmap to 1 for the file
    if (FileType == 1) {
        BitMapChange(Disk_Pointer, AddToBlockSeq(FindDocContextNode(Disk_Pointer, BlockSeq)), 1); // Set the corresponding bit in the bitmap to 1 for the file content
        return;
    }
    else { // if a file
        char TempChar;
        for (int k = 0; k <= 2; k++) {
            int CheckPoint = 112 + 32 * k;
            JumpPointer(Disk_Pointer, BlockSeq, CheckPoint);
            Disk_Pointer >> TempChar;
            if (TempChar == '1') {
                JumpPointer(Disk_Pointer, BlockSeq, CheckPoint); // Move back to the original position
                int* ChildNodeAdd = Output32Bit(Disk_Pointer);
                ConsistencyCheck(Disk_Pointer, AddToBlockSeq(ChildNodeAdd));
            }
        }
        return;
    }
}

int main()
{
    fstream Disk_Pointer("C:\\Users\\CHaron\\Desktop\\os_filesystem\\final_edition_os\\disk.txt", ios::in | ios::out); // File stream object
    if (Disk_Pointer.is_open())
        cout << "successfully open the disk.txt" << endl;
    else {
        cout << "unsuccessfully open the disk.txt" << endl;
        exit(0);
    }
    /*Initializes the disk and finally adds it*/
    Disk_Pointer.seekg(0, ios::beg);
    for (int i = 0; i < 100 * 1024 * 1024; i++) //约23 seconds
        Disk_Pointer << 0;
    cout << "The disk is initialized!" << endl;
    /* Initialize root directory (root occupies the position 101 block) */
    Disk_Pointer.seekg(1024 * 101, ios::beg); // Move the file pointer of the file stream object Disk_Pointer to the beginning of the file and offset it to the location of the first inode, which is the root directory
    char RootDirName[] = "root0000";      // Root directory name
    InputCharArr(Disk_Pointer, RootDirName);
    int FileType[] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // Directory file type
    InputIntArr(Disk_Pointer, FileType, 8);
    int Access[] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // No access permissions for directories, all set to 0
    InputIntArr(Disk_Pointer, Access, 8);
    int ParNode[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    // Root directory has no parent node
    InputIntArr(Disk_Pointer, ParNode, 32);
    int ChildNode[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    // Root directory initially has no child nodes
    InputIntArr(Disk_Pointer, ChildNode, 96);
    // Directory has no file con1tent
    int DiskAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    InputIntArr(Disk_Pointer, DiskAddress, 32);

    /* Initialize superblock area */
    JumpPointer(Disk_Pointer, 0, 0);
    InputCharArr(Disk_Pointer, const_cast<char*>("JayJoey0")); // File system ownership
    JumpPointer(Disk_Pointer, 0, 64);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(100), 32); // Bitmap size, 100 blocks
    JumpPointer(Disk_Pointer, 0, 96);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(1024), 32); // Disk block size
    JumpPointer(Disk_Pointer, 0, 128);                  // Change
    InputIntArr(Disk_Pointer, BlockSeqToAdd(0), 32);    // Superblock number
    JumpPointer(Disk_Pointer, 0, 160);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(1), 32); // Bitmap number
    JumpPointer(Disk_Pointer, 0, 192);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(101), 32); // Root directory number
    JumpPointer(Disk_Pointer, 0, 224);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(102), 32); // Inode area number
    JumpPointer(Disk_Pointer, 0, 256);
    InputIntArr(Disk_Pointer, BlockSeqToAdd(50102), 32); // File area number

    /* Initialize bitmap area */
    JumpPointer(Disk_Pointer, 1, 0); // Jump to the bitmap area
    for (int i = 0; i < 102; i++)    // Set the first 101 disks as used
        Disk_Pointer << 1;
    for (int i = 102; i < 1024 * 100; i++) // Set the remaining disks as unused
        Disk_Pointer << 0;
    cout << "The root directory is initialized" << endl;
    JumpPointer(Disk_Pointer, 101, 0);

    /* Reply information space for the server to reply to the client */
    // Create a kernel object for a memory-mapped file
    HANDLE ReplyInfoSpace = CreateFileMapping(
        INVALID_HANDLE_VALUE, // Physical file handle, fixed writing
        NULL,                 // Default security level
        PAGE_READWRITE,       // Read-write permission
        0,                    // High-order DWORD of maximum length of file mapping
        BUF_SIZE,             // Low-order DWORD of maximum length of file mapping
        "ReplyInfoSpace"      // Shared memory name
    );

    // Map the memory-mapped file into the virtual address space of the process
    LPVOID ReplyInfoSend = MapViewOfFile(
        ReplyInfoSpace,      // Handle to the shared memory
        FILE_MAP_ALL_ACCESS, // Read-write permission
        0,                   // High-order DWORD of the starting offset within the file
        0,                   // Low-order DWORD of the starting offset within the file
        BUF_SIZE             // Number of bytes to map, 0 means map the entire file mapping object
    );

    /* Write space for high-speed clients' current write status */
    HANDLE WriteSpace = CreateFileMapping(
        INVALID_HANDLE_VALUE, // Physical file handle
        NULL,                 // Default security level
        PAGE_READWRITE,       // Read-write permission
        0,                    // High-order DWORD of maximum length of file mapping
        BUF_SIZE,             // Low-order DWORD of maximum length of file mapping
        "WriteSpace"          // Shared memory name
    );

    // Map the view of the buffer, getting a pointer to shared memory
    LPVOID WriteSend = MapViewOfFile(
        WriteSpace,          // Handle to the shared memory
        FILE_MAP_ALL_ACCESS, // Read-write permission
        0,                   // High-order DWORD of the starting offset within the file
        0,                   // Low-order DWORD of the starting offset within the file
        BUF_SIZE             // Number of bytes to map, 0 means map the entire file mapping object
    );
    char WriteSendBuf[BUF_SIZE] = { 0 }; // Buffer for writing information

    /* rd-related content */
    // Directory deletion part
    int rdIsNotEmpty = 0; // 01 flag variable, indicating whether the directory in rd is empty or not
    int rdOriSeq = 0;
    int rdTarSeq = 0;
    string rdBuf = ""; // Store the address part of the user's input content
    int WriteLock = 0;

    /*start accepting information*/
    while (true) {
        // Open the shared file object
        HANDLE InstructionSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "InstructionSpace"); // modify
        LPVOID InstructionRec = MapViewOfFile(InstructionSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        // Determine whether the pointer is empty (empty when shared memory is not created)
        if (!InstructionRec) {
            printf("Waiting command..................\n");
            Sleep(2000);
            continue;
        }

        if (strlen((char*)InstructionRec)) { // Check whether the shared memory has any content
            char UserAccBuf[BUF_SIZE] = { 0 };  // User rights cache
            HANDLE AccSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "AccSpace"); // mofidy
            LPVOID AccRec = MapViewOfFile(AccSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            MemoryInfoCpy(UserAccBuf, AccRec);

            char InstructionRecBuffer[BUF_SIZE] = { 0 };
            MemoryInfoCpy(InstructionRecBuffer, InstructionRec); // Copy the shared memory data
            cout << InstructionRecBuffer << endl;
            istringstream Buf(InstructionRecBuffer); // Converts content into an input stream
            string InstructionBuf;                   // Used to store the instruction part of the input content

            Buf >> InstructionBuf;                               // Enter the instructions entered by the user
            cout << "Get info:" << InstructionRecBuffer << endl; // Output the information entered by the client
            if (rdIsNotEmpty == 1) {
                if (InstructionBuf == "y") {
                    rdDir(Disk_Pointer, rdTarSeq);
                    BitMapChange(Disk_Pointer, rdTarSeq, 0);
                    Reply("delete completed", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSeq, 0);
                    rdIsNotEmpty = 0;
                    rdOriSeq = 0;
                    rdBuf = "";
                    rdTarSeq = 0;
                    continue;
                }
                else if (InstructionBuf == "n") {
                    Reply("Operation cancelled", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSeq, 0);
                    rdIsNotEmpty = 0;
                    rdOriSeq = 0;
                    rdBuf = "";
                    rdTarSeq = 0;
                    continue;
                }
                else {
                    Reply("Instruction error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSeq, 0);
                    rdIsNotEmpty = 0;
                    rdOriSeq = 0;
                    rdBuf = "";
                    rdTarSeq = 0;
                    continue;
                }
            }
            else if (InstructionBuf == "info") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string ReplyInfo = "";
                string DocSysHost;
                string DocSysSize;
                string DiskBlockSize;
                string SuperBlockSeq;
                string BitMapSeq;
                string RootSeq;
                string INodeSeq;
                string DocSeq;
                DocSysHost = FindDocName(Disk_Pointer, 0); // File system ownership
                JumpPointer(Disk_Pointer, 0, 64);
                DocSysSize = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // File system size
                JumpPointer(Disk_Pointer, 0, 96);
                DiskBlockSize = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // Disk block size
                JumpPointer(Disk_Pointer, 0, 128);
                SuperBlockSeq = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // Superblock sequence number
                JumpPointer(Disk_Pointer, 0, 160);
                BitMapSeq = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // Bitmap sequence number
                JumpPointer(Disk_Pointer, 0, 192);
                RootSeq = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // Root sequence number
                JumpPointer(Disk_Pointer, 0, 224);
                INodeSeq = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // I node area remaining space
                JumpPointer(Disk_Pointer, 0, 256);
                DocSeq = to_string(AddToBlockSeq(Output32Bit(Disk_Pointer))); // File area number
                ReplyInfo = ReplyInfo + "File system ownership:" + DocSysHost + "\n";
                ReplyInfo = ReplyInfo + "File system size:" + DocSysSize + " MB" + "\n";
                ReplyInfo = ReplyInfo + "Disk block size:" + DiskBlockSize + " B" + "\n";
                ReplyInfo = ReplyInfo + "Superblock sequence number:" + SuperBlockSeq + "\n";
                ReplyInfo = ReplyInfo + "Bitmap sequence number:" + BitMapSeq + "\n";
                ReplyInfo = ReplyInfo + "Root sequence number:" + RootSeq + "\n";
                ReplyInfo = ReplyInfo + "I Node area number:" + INodeSeq + "\n";
                // ReplyInfo = ReplyInfo + "I node area remaining space:" + to_string(GetINodeRemain(Disk_Pointer)) + "\n";
                ReplyInfo = ReplyInfo + "File area number:" + DocSeq + "\n";
                // ReplyInfo = ReplyInfo + "Remaining space in the file area:" + to_string(GetDocRemain(Disk_Pointer));
                JumpPointer(Disk_Pointer, OriSeq, 0);
                Reply(ReplyInfo, ReplyInfoSend);
                continue;
            }
            else if (InstructionBuf == "cd") {
                string cdBuf; // The user stores the address portion of the input
                Buf >> cdBuf;
                string ReplyInfo = cdAddLocation(Disk_Pointer, cdBuf);
                Reply(ReplyInfo, ReplyInfoSend);
            }
            else if (InstructionBuf == "dir") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string ReplyInfo = "";
                string dirBuf; // The user stores the address portion of the input
                Buf >> dirBuf;
                if (dirBuf == "/s") { // If the /s parameter is taken
                    ReplyInfo += "All files and subdirectories in the current directory are:";
                    ShowAllChildNodeName(Disk_Pointer, GetBlockSeq(Disk_Pointer), ReplyInfo);
                    Reply(ReplyInfo, ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                int CurBlockSeq = dirAddLocation(Disk_Pointer, dirBuf); // File pointer location
                if (CurBlockSeq == 0) {                                 // Error message is returned
                    Reply("Incorrect address, creation failed", ReplyInfoSend);
                    continue;
                }
                ReplyInfo += "The information in the current directory is:";
                ShowAllInfo(Disk_Pointer, CurBlockSeq, ReplyInfo);
                Reply(ReplyInfo, ReplyInfoSend);
                continue;

                JumpPointer(Disk_Pointer, OriSeq, 0);
            }
            else if (InstructionBuf == "md") { // Create a new directory
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string mdBuf; // The user stores the address portion of the input
                Buf >> mdBuf;

                string CurDirName = mdAddLocation(Disk_Pointer, mdBuf);
                if (CurDirName == "") {
                    Reply("The address is incorrect.creation failed", ReplyInfoSend);
                    continue;
                }

                /*Determine the length of the new directory*/
                if (CurDirName.length() > 8) { // The directory name cannot be larger than 8 characters
                    Reply("Failed to create the directory because the directory name is too long", ReplyInfoSend);
                    continue;
                }

                int NewINodeBlockSeq = FindFreeAreaInINode(Disk_Pointer);

                /*Check whether the file system has space*/
                if (!NewINodeBlockSeq) { // If the file system summary has no space, the creation fails
                    Reply("Failed to create the file system because there is no free space", ReplyInfoSend);
                    continue;
                }

                /*Determines whether the created file name is the same as the file name in the child node*/
                if (MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName)) {
                    Reply("Failed to create the file name because the file name is repeated", ReplyInfoSend);
                    continue;
                }

                /*Determines if the child node is full*/
                int FreeChildSeq = FindFreeChildNode(Disk_Pointer, GetBlockSeq(Disk_Pointer));
                if (FreeChildSeq) {                                                                      // If not, mark the child node as used and enter a 32-bit address
                    JumpPointer(Disk_Pointer, GetBlockSeq(Disk_Pointer), 112 + 32 * (FreeChildSeq - 1)); // Jump to the corresponding child node
                    int* INode = BlockSeqToAdd(NewINodeBlockSeq);
                    INode[0] = 1;                                            // Setting the first position to 1 indicates that the child is already in use
                    InputIntArr(Disk_Pointer, INode, 32);                    // Enter 32 child nodes
                    JumpPointer(Disk_Pointer, GetBlockSeq(Disk_Pointer), 0); // Jump back to start
                }
                else {
                    Reply("The number of subfiles of the current file has reached the upper limit", ReplyInfoSend);
                    continue;
                }

                /*Initialize the new file*/
                InputDocName(Disk_Pointer, NewINodeBlockSeq, CurDirName);
                InputParNode(Disk_Pointer, NewINodeBlockSeq, BlockSeqToAdd(GetBlockSeq(Disk_Pointer)));
                InputAcc(Disk_Pointer, NewINodeBlockSeq, "000001"); //notice
                //Initializes the child part of node I
                JumpPointer(Disk_Pointer, NewINodeBlockSeq, 112);
                for (int i = 0; i < 3 * 32; i++) {
                    Disk_Pointer << "0";
                }
                BitMapChange(Disk_Pointer, NewINodeBlockSeq, 1);
                Reply("Directory created successfully", ReplyInfoSend);
                JumpPointer(Disk_Pointer, OriSeq, 0);
                continue;
            }
            else if (InstructionBuf == "rd") {
                rdOriSeq = GetBlockSeq(Disk_Pointer);
                Buf >> rdBuf;
                rdTarSeq = rdAddLocation(Disk_Pointer, rdBuf); // Find the target disk block that should be deleted
                if (rdTarSeq == 1) {                           // If the root directory
                    Reply("The root directory cannot be deleted. Deleting the root directory fails", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSeq, 0);
                    continue;
                }
                if (FindDocType(Disk_Pointer, rdTarSeq) != 0) { // If the destination file is not a directory
                    Reply("The target file is a common file and fails to be deleted", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSeq, 0);
                    continue;
                }
                if (rdTarSeq) {
                    if (!IsDirEmpty(Disk_Pointer, rdTarSeq)) { // If not empty
                        rdIsNotEmpty = 1;
                        Reply("The target directory is not empty. Do you need to delete it? y/n", ReplyInfoSend);
                        continue;
                    }
                    else {                           // If  empty
                        rdDir(Disk_Pointer, rdTarSeq); // Delete corresponding node
                        BitMapChange(Disk_Pointer, rdTarSeq, 0);
                        Reply("The target directory is deleted", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, rdOriSeq, 0);
                        rdIsNotEmpty = 0; // Set all coefficients to 0
                        rdOriSeq = 0;
                        rdBuf = "";
                        rdTarSeq = 0;
                        continue;
                    }
                }
                else { // If the target disk block is not found, an error message is displayed
                    Reply("address error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSeq, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, rdOriSeq, 0);
            }
            else if (InstructionBuf == "newfile") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string newfileNameBuf; // The user stores the address portion of the input
                string newfileAccBuf;
                string newfileContextBuf;
                Buf >> newfileNameBuf;
                Buf >> newfileAccBuf;
                Buf >> newfileContextBuf;

                string CurDirName = newfileAddLocation(Disk_Pointer, newfileNameBuf);
                /*File name format detection*/
                string ReplyInfo = FileNameTest(CurDirName);
                if (ReplyInfo != "") {
                    Reply(ReplyInfo, ReplyInfoSend);
                    continue;
                }

                /*Protection type Format detection*/
                ReplyInfo = FileAccTest(newfileAccBuf);
                if (ReplyInfo != "") {
                    Reply(ReplyInfo, ReplyInfoSend);
                    continue;
                }

                int NewINodeBlockSeq = FindFreeAreaInINode(Disk_Pointer);
                int NewDocBlockSeq = FindFreeAreaInDoc(Disk_Pointer);

                /*Check whether the file system has space*/
                if (!NewDocBlockSeq) { // If the file system summary has no space, the creation fails
                    Reply("Failed to create the file system because there is no free space", ReplyInfoSend);
                    continue;
                }
                if (!NewDocBlockSeq) {
                    Reply("The file system has no free space. Creating the file failed.", ReplyInfoSend);
                    continue;
                }

                /*Determines whether the created file name is the same as the file name in the child node*/
                if (MatchDocName(Disk_Pointer, GetBlockSeq(Disk_Pointer), CurDirName)) {
                    Reply("Failed to create the file name because the file name is repeated", ReplyInfoSend);
                    continue;
                }

                /*Determines if the child node is full*/
                int FreeChildSeq = FindFreeChildNode(Disk_Pointer, GetBlockSeq(Disk_Pointer));
                if (FreeChildSeq) {                                                                      // If not, mark the child node as used and enter a 32-bit address
                    JumpPointer(Disk_Pointer, GetBlockSeq(Disk_Pointer), 112 + 32 * (FreeChildSeq - 1)); // Jump to the corresponding child node
                    int* INode = BlockSeqToAdd(NewINodeBlockSeq);
                    INode[0] = 1;                                            // Setting the first position to 1 indicates that the child is already in use
                    InputIntArr(Disk_Pointer, INode, 32);                    // Enter 32 child nodes
                    JumpPointer(Disk_Pointer, GetBlockSeq(Disk_Pointer), 0); // Jump back to start
                }
                else {
                    Reply("The number of subfiles of the current file has reached the upper limit", ReplyInfoSend);
                    continue;
                }

                /*Initialize the new file*/
                InputDocName(Disk_Pointer, NewINodeBlockSeq, CurDirName);                               // Input file name
                InputParNode(Disk_Pointer, NewINodeBlockSeq, BlockSeqToAdd(GetBlockSeq(Disk_Pointer))); // Enter the parent address
                InputAcc(Disk_Pointer, NewINodeBlockSeq, newfileAccBuf);                                // Enter the file protection type
                InputFileContext(Disk_Pointer, NewINodeBlockSeq, NewDocBlockSeq, newfileContextBuf);    // Input file content
                InputFileType(Disk_Pointer, NewINodeBlockSeq, 1);                                       // Input file type
                BitMapChange(Disk_Pointer, NewINodeBlockSeq, 1);
                BitMapChange(Disk_Pointer, NewDocBlockSeq, 1);
                Reply("File created successfully", ReplyInfoSend);
                JumpPointer(Disk_Pointer, OriSeq, 0);
                continue;
            }
            else if (InstructionBuf == "cat") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string mdBuf; // The user stores the address portion of the input
                Buf >> mdBuf;
                int TarSeq = catAddLocation(Disk_Pointer, mdBuf);
                if (FindDocType(Disk_Pointer, TarSeq) != 1) { // If the target file is not a normal file
                    Reply("Target file is not a normal file, instruction error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                if (TarSeq) {
                    int* ComAcc = FindDocComAcc(Disk_Pointer, TarSeq);                                             // Query the access permissions of common users
                    int* SysAcc = FindDocSysAcc(Disk_Pointer, TarSeq);                                             // Query the access permission of a system user
                    if (!((ComAcc[0] == 1 && UserAccBuf[0] == '1') || (SysAcc[0] == 1 && UserAccBuf[0] == '0'))) { // Determine whether you have permission to write to the file
                        Reply("No permission to read the file", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }
                    Reply(GetFileContext(Disk_Pointer, AddToBlockSeq(FindDocContextNode(Disk_Pointer, TarSeq))), ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                else {
                    Reply("address error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, OriSeq, 0);
            }
            else if (InstructionBuf == "write") { // eg:write /root/123.txt anotherString
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string writeBuf;  // The user stores the address portion of the input
                string InputInfo; // File content to be entered
                Buf >> writeBuf;
                Buf >> InputInfo;
                int TarSeq = catAddLocation(Disk_Pointer, writeBuf);
                if (FindDocType(Disk_Pointer, TarSeq) != 1) { // If the target file is not a normal file
                    Reply("Target file is not a normal file, instruction error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                if (TarSeq) {
                    int* ComAcc = FindDocComAcc(Disk_Pointer, TarSeq); // Query the access permission of a normal user
                    int* SysAcc = FindDocSysAcc(Disk_Pointer, TarSeq); // Query the access permission of a system user
                    if (InputInfo.length() > 128) {
                        Reply("The file is too large to be written", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }
                    if (!((ComAcc[1] == 1 && UserAccBuf[0] == '1') || (SysAcc[1] == 1 && UserAccBuf[0] == '0'))) { // Determine whether you have permission to write to the file
                        Reply("No permission to write to the file", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }
                    /*Clear source file*/
                    JumpPointer(Disk_Pointer, AddToBlockSeq(FindDocContextNode(Disk_Pointer, TarSeq)), 0);
                    for (int i = 0; i < 1024; i++)
                        Disk_Pointer << 0;
                    JumpPointer(Disk_Pointer, AddToBlockSeq(FindDocContextNode(Disk_Pointer, TarSeq)), 0);

                    /*Write time delay*/
                    strcpy_s((char*)WriteSend, writeBuf.length() + 1, const_cast<char*>(writeBuf.c_str()));
                    Sleep(20000);
                    ShareMemoryClear(WriteSend);

                    /*Write*/
                    InputCharArr(Disk_Pointer, const_cast<char*>(InputInfo.c_str()));
                    Reply("write successful", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                else {
                    Reply("address error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, OriSeq, 0);
            }
            else if (InstructionBuf == "copy") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string OriAdd;
                string TarAdd;
                string Parameter;

                Buf >> OriAdd;
                Buf >> TarAdd;
                Buf >> Parameter;

                if (Parameter == "") { // If there are no parameters

                    /*Find the source file content*/
                    int OriAddSeq = catAddLocation(Disk_Pointer, OriAdd);
                    string OriContext;
                    if (OriAddSeq)
                        OriContext = GetFileContext(Disk_Pointer, AddToBlockSeq(FindDocContextNode(Disk_Pointer, OriAddSeq)));
                    else {
                        Reply("source address error", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }

                    int TarAddSeq = catAddLocation(Disk_Pointer, TarAdd);
                    if (TarAddSeq) {
                        int* TarAddDocAdd = FindDocContextNode(Disk_Pointer, TarAddSeq);
                        JumpPointer(Disk_Pointer, AddToBlockSeq(TarAddDocAdd), 0);
                        for (int i = 0; i < 1024; i++)
                            Disk_Pointer << 0;
                        JumpPointer(Disk_Pointer, AddToBlockSeq(TarAddDocAdd), 0);
                        InputCharArr(Disk_Pointer, const_cast<char*>(OriContext.c_str()));
                        Reply("copy successful", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }
                    else {
                        Reply("Destination address error", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }
                }
                else if (Parameter == "host") {
                    string TempOriContext;
                    string OriContext;
                    string OriFileName = GetWindowsFileName(OriAdd);
                    fstream Disk_Pointer2(OriAdd, ios::in | ios::out); // modify

                    /*Find the source file content*/
                    if (Disk_Pointer2) {
                        while (!Disk_Pointer2.eof()) {
                            Disk_Pointer2 >> TempOriContext;
                            OriContext += TempOriContext;
                        }
                        // GetWindowsFileName(OriAdd,OriFileName)
                    }
                    else {
                        Reply("source address error", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSeq, 0);
                        continue;
                    }
                    Disk_Pointer2.close();

                    /*Copy to destination folder*/
                    string InputInstruciton = "";
                    InputInstruciton += OriFileName;
                    InputInstruciton += " 111111 ";
                    InputInstruciton += OriContext;
                    istringstream Inputsstr(InputInstruciton);
                    Reply(CopyNewFile(Disk_Pointer, Inputsstr), ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                else {
                    Reply("parameter error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, OriSeq, 0);
            }
            else if (InstructionBuf == "del") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                string delBuf; // The user stores the address portion of the input
                Buf >> delBuf;
                int TarSeq = catAddLocation(Disk_Pointer, delBuf);
                if (FindDocType(Disk_Pointer, TarSeq) != 1) { // If the target file is not a normal file
                    Reply("Target file is not a normal file, instruction error!", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                if (TarSeq) {
                    delDoc(Disk_Pointer, TarSeq);
                    Reply("delete successful", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
                else {
                    Reply("address error", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSeq, 0);
                    continue;
                }
            }
            else if (InstructionBuf == "check") {
                int OriSeq = GetBlockSeq(Disk_Pointer);
                ConsistencyCheck(Disk_Pointer, 101);
                JumpPointer(Disk_Pointer, OriSeq, 0);
                Reply("file consistency check successful", ReplyInfoSend);
                continue;
            }
            else {
                Reply("command error", ReplyInfoSend);
                continue;
            }
        }
        else {
            printf("Waiting command......\n");
            Sleep(2000);
        }
    }

    /*unmapping reply message space files*/
    UnmapViewOfFile(ReplyInfoSend);
    CloseHandle(ReplyInfoSpace);
    return 0;
}
