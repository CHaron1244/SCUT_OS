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
{ // IntArr��0/1����
    cout << "����ָ��λ��:" << Disk_Pointer.tellg() << endl;
    for (int i = 0; i < Length; i++)
        Disk_Pointer << to_string(IntArr[i]); // ��ǰλ��д�������е�����(0 or 1)
    Disk_Pointer << '\n';
}
void InputChar(fstream &Disk_Pointer, char Char)
{ // ��Char����ַ���8λ�����Ʊ�ʾд�뵱ǰλ��(д������ַ�)
    int *InputInt = new int[8];
    int IntNum = int(Char);       // ���ַ�ת��ΪASCII��
    for (int i = 0; i < 8; i++) { // �ó�nȡ�෨ת��Ϊ��������
        InputInt[7 - i] = IntNum % 2;
        IntNum /= 2;
    }
    cout << "��������ַ���Ӧ������:" << endl;
    for (int i = 0; i <= 7; i++)
        cout << "��" << i << "������" << InputInt[i] << " ";
    cout << endl;
    InputIntArr(Disk_Pointer, InputInt, 8);
}
void InputCharArr(fstream &Disk_Pointer, char *CharArr)
{ // ��ÿ���ַ����ݵ�8λ�����Ʊ�ʾд�뵱ǰλ��(д������ַ�����)
    for (int i = 0; i < strlen(CharArr); i++) {
        cout << "Ҫд������ַ�:" << CharArr[i] << endl;
        InputChar(Disk_Pointer, CharArr[i]);
    }
    cout << "д����!" << endl;
}
int main()
{
    std::fstream Disk_Pointer;
    Disk_Pointer.open("E:\\Desktop\\os_filesystem\\test_for_os\\test.txt", std::ios::in | std::ios::out);
    
    if (Disk_Pointer.is_open()) {
        Disk_Pointer.seekg(0, std::ios::beg);
        std::cout << "����ָ��λ��: " << Disk_Pointer.tellg() << std::endl;
    } else {
        std::cout << "�޷����ļ�����Ȩ�޷��ʡ�" << std::endl;
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
