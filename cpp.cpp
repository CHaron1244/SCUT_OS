#include <iostream>
#include <queue>
#include<string.h>
using namespace std;
int main()
{
    string DocName = "123asd";
    char *CharDocName = new char[10]; // 申请一个临时数组存放文件名(写入的参数只能是char*)
    strcpy_s(CharDocName, 10 + 1, DocName.c_str());
    cout << CharDocName << endl;
    // for(int i=0;i<10;i++)
    //     cout << CharDocName[i] << "-";
    cout << endl;
    cout << "hello" << endl;
    for(int i=0;i<10;i++)
        cout << (int)CharDocName[i]<<" ";
    char dest[20];
    cout << sizeof(dest) << endl;
    return 0;
}
