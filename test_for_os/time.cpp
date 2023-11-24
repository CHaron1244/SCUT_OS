#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

int main() {
    // 打开文件进行写入
    ofstream file("test.txt");

    // 开始计时
    auto start = high_resolution_clock::now();

    // 写入十万个0
    for (int i = 0; i < 100*1024*1024; ++i) {
        file << "0";
    }

    // 结束计时
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // 输出耗时
    cout << "Time taken: " << duration.count() << " ms" << endl;

    // 关闭文件
    file.close();

    return 0;
}
