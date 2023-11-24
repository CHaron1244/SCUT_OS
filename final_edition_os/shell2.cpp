#include <fstream> // file input and output operations
#include <iostream>
#include <limits>
#include <sstream>   // used for string stream operations. Provides the stringstream class, which allows you to process strings as streams and perform conversions between strings and numbers. Useful for converting numbers to strings or extracting numbers from strings.
#include <windows.h> // contains functions and macros for Windows operating system programming. It allows you to perform various system-level operations in a Windows environment, such as window management and message processing.

using namespace std;

#define BUF_SIZE 8192

void LogIn();
void SysUserInteractiveUI();
void ComUserInteractiveUI();

// other functions
void ShareMemoryClear(LPVOID& ShareMemoryPointer);            // clear shared memory
void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer); // copy the contents of shared memory

/* Instruction space for transmitting instructions from the client to the server */
// create a shared file handle
HANDLE InstructionSpace = CreateFileMapping(
    INVALID_HANDLE_VALUE, // physical file handle
    NULL,                 // default security level

    PAGE_READWRITE,    // readable and writable
    0,                 // high-order file size
    BUF_SIZE,          // low-order file size
    "InstructionSpace" // shared memory name
);

// map the buffer view to get a pointer to the shared memory instruction send??
LPVOID InstructionSend = MapViewOfFile(
    InstructionSpace,    // shared memory handle
    FILE_MAP_ALL_ACCESS, // read-write permission??
    0,
    0,
    BUF_SIZE);

/* Permission space for informing the server of the user's identity */
HANDLE AccSpace = CreateFileMapping(
    INVALID_HANDLE_VALUE, // physical file handle
    NULL,                 // default security level
    PAGE_READWRITE,       // readable and writable
    0,                    // high-order file size
    BUF_SIZE,             // low-order file size
    "AccSpace"            // shared memory name
);

// map the buffer view to get a pointer to the shared memory acc send??
LPVOID AccSend = MapViewOfFile(
    AccSpace,            // shared memory handle
    FILE_MAP_ALL_ACCESS, // read-write permission
    0,
    0,
    BUF_SIZE);

int main()
{
    LogIn();
    return 0;
}

void LogIn()
{
    fstream f;
    string userName; // username
    string password; // password
    cout << "Please input your account number:";
    cin >> userName;
    cout << "Please input your password:";
    cin >> password;
    if (userName.compare("123") == 0 && password.compare("123") == 0)
        SysUserInteractiveUI(); // interactive interface
    else if (userName.compare("321") == 0 && password.compare("321") == 0)
        ComUserInteractiveUI(); // interactive interface
    return;
}

void ComUserInteractiveUI()
{
    char instruction[BUF_SIZE] = {};
    char directory[100] = "\\root";
    fstream f;
    while (1) {
        cout << directory << ":$";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear the data in the input stream buffer
        cin.get(instruction, BUF_SIZE);
        char ReplyInfoRecBuffer[BUF_SIZE] = { 0 }; // receive response information?
        if (strcmp(instruction, "EXIT") == 0)
            break;

        // copy the data to shared memory
        strcpy_s((char*)InstructionSend, strlen(instruction) + 1, instruction);
        strcpy_s((char*)AccSend, strlen("1") + 1, "1");

        /*对于write是否占用的判断*/
        HANDLE WriteSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "WriteSpace"); // 不断地从回复信息空间中获取数据
        LPVOID WriteRec = MapViewOfFile(WriteSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        istringstream InputSS(instruction);
        string WritingFile;
        if (WriteRec) {
            WritingFile = string((char*)WriteRec);
        }
        else {
            WritingFile = "";
        }

        string InputInstruction;
        string InputFile;
        InputSS >> InputInstruction;
        InputSS >> InputFile;
        if (InputFile == WritingFile && InputInstruction == "write") {
            cout << "当前文件正在被其他用户写入，请稍后再试" << endl;
            cout << ReplyInfoRecBuffer << endl;
            char NULLChar[] = ""; // 清空输入
            strcpy_s((char*)InstructionSend, strlen(NULLChar) + 1, NULLChar);
            continue;
        }

        while (1) {
            HANDLE ReplyInfoSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "ReplyInfoSpace"); // constantly getting data from the response information space
            LPVOID ReplyInfoRec = MapViewOfFile(ReplyInfoSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (!ReplyInfoRec)
                continue;
            if (strlen((char*)ReplyInfoRec)) {
                MemoryInfoCpy(ReplyInfoRecBuffer, ReplyInfoRec);
                break; // break if the acquired data is received
            }
        }

        if (ReplyInfoRecBuffer[0] == '/') {
            for (int i = 0; ReplyInfoRecBuffer[i] != '\0'; i++)
                if (ReplyInfoRecBuffer[i] == '/')
                    ReplyInfoRecBuffer[i] = '\\';
            memcpy(directory, ReplyInfoRecBuffer, 100);
            continue;
        }
        else {
            /*Output the returned information to the message box*/
            cout << ReplyInfoRecBuffer << '\n';
            continue;
        }
    }

    /* Release the file mapping of the instruction space */
    UnmapViewOfFile(InstructionSend);
    CloseHandle(InstructionSpace);
}

void SysUserInteractiveUI()
{
    char instruction[BUF_SIZE] = {};
    char directory[100] = "\\root";
    fstream f;
    while (1) {
        cout << directory << ":$";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cin.get(instruction, BUF_SIZE);
        char ReplyInfoRecBuffer[BUF_SIZE] = { 0 }; // receive response information?
        if (strcmp(instruction, "EXIT") == 0)
            break;

        // copy the data to shared memory
        strcpy_s((char*)InstructionSend, strlen(instruction) + 1, instruction);
        strcpy_s((char*)AccSend, strlen("0") + 1, "0");

        /*对于write是否占用的判断*/
        HANDLE WriteSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "WriteSpace"); // 不断地从回复信息空间中获取数据
        LPVOID WriteRec = MapViewOfFile(WriteSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        istringstream InputSS(instruction);
        string WritingFile;
        if (WriteRec)
            WritingFile = string((char*)WriteRec);
        else
            WritingFile = "";

        string InputInstruction;
        string InputFile;
        InputSS >> InputInstruction;
        InputSS >> InputFile;
        if (InputFile == WritingFile && InputInstruction == "write") {
            cout << "当前文件正在被其他用户写入，请稍后再试！" << endl;
            char NULLChar[] = ""; // 清空输入
            strcpy_s((char*)InstructionSend, strlen(NULLChar) + 1, NULLChar);
        }

        while (1) {
            HANDLE ReplyInfoSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "ReplyInfoSpace"); // constantly getting data from the response information space
            LPVOID ReplyInfoRec = MapViewOfFile(ReplyInfoSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (!ReplyInfoRec)
                continue;
            if (strlen((char*)ReplyInfoRec)) {
                MemoryInfoCpy(ReplyInfoRecBuffer, ReplyInfoRec);
                break; // break if the acquired data is received
            }
        }

        if (ReplyInfoRecBuffer[0] == '/') {
            for (int i = 0; ReplyInfoRecBuffer[i] != '\0'; i++)
                if (ReplyInfoRecBuffer[i] == '/')
                    ReplyInfoRecBuffer[i] = '\\';
            memcpy(directory, ReplyInfoRecBuffer, 100);
            continue;
        }
        else {
            /*Output the returned information to the message box*/
            cout << ReplyInfoRecBuffer << '\n';
            continue;
        }
    }

    /*Release the file mapping of the instruction space*/
    UnmapViewOfFile(InstructionSend);
    CloseHandle(InstructionSpace);
}

void ShareMemoryClear(LPVOID& ShareMemoryPointer)
{
    char NULLChar[] = "";
    strcpy_s((char*)ShareMemoryPointer, strlen(NULLChar) + 1, NULLChar);
}

void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer)
{
    strcpy_s(Buffer, strlen((char*)ShareMemoryPointer) + 1, (char*)ShareMemoryPointer);
    ShareMemoryClear(ShareMemoryPointer);
}