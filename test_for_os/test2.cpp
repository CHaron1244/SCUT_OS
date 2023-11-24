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
int main()
{
    fstream f("test2.txt", std::ios::in | std::ios::out | std::ios::binary);
    cout<<f.tellg()<<endl;
    for (int i = 0; i < 1024; i++)
        f << 0;
    
    return 0;
}