#include "stdio.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <io.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <windows.h>
using namespace std;
void InputIntArr(fstream &Disk_Pointer, int *IntArr, int Length)
{ // IntArr是0/1数组
    cout << "现在指向位置:" << Disk_Pointer.tellg() << endl;
    for (int i = 0; i < Length; i++)
        Disk_Pointer << to_string(IntArr[i]); // 向当前位置写入数组中的数据(0 or 1)
    Disk_Pointer << '\n';
}
void InputChar(fstream &Disk_Pointer, char Char)
{ // 把Char这个字符的8位二进制表示写入当前位置(写入这个字符)
    int *InputInt = new int[8];
    int IntNum = int(Char);       // 将字符转换为ASCII码
    for (int i = 0; i < 8; i++) { // 用除n取余法转换为二进制数
        InputInt[7 - i] = IntNum % 2;
        IntNum /= 2;
    }
    cout << "输入这个字符对应的数组:" << endl;
    for (int i = 0; i <= 7; i++)
        cout << "第" << i << "个数是" << InputInt[i] << " ";
    cout << endl;
    InputIntArr(Disk_Pointer, InputInt, 8);
}
void InputCharArr(fstream &Disk_Pointer, char *CharArr)
{ // 把每个字符数据的8位二进制表示写入当前位置(写入这个字符数组)
    for (int i = 0; i < strlen(CharArr); i++) {
        cout << "要写入这个字符:" << CharArr[i] << endl;
        InputChar(Disk_Pointer, CharArr[i]);
    }
    cout << "写完了!" << endl;
}
int main()
{
    std::fstream Disk_Pointer;
    Disk_Pointer.open("E:\\Desktop\\os_filesystem\\test_for_os\\test.txt", std::ios::in | std::ios::out);
    
    if (Disk_Pointer.is_open()) {
        Disk_Pointer.seekg(0, std::ios::beg);
        std::cout << "现在指向位置: " << Disk_Pointer.tellg() << std::endl;
    } else {
        std::cout << "无法打开文件或无权限访问。" << std::endl;
    }
    char *str = "JayJoey";
    cout << str << endl;
    cout << strlen(str) << endl;

    char str1[] = "Hello,world!";
    cout << strlen(str1) << endl;

    char RootDirName[] = "root\0\0\0\0";
    InputCharArr(Disk_Pointer, RootDirName);
    cout << strlen(RootDirName) << endl; // 4
    cout << RootDirName << endl;         // root
}
