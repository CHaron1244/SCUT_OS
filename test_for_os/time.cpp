#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

int main() {
    // ���ļ�����д��
    ofstream file("test.txt");

    // ��ʼ��ʱ
    auto start = high_resolution_clock::now();

    // д��ʮ���0
    for (int i = 0; i < 100*1024*1024; ++i) {
        file << "0";
    }

    // ������ʱ
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // �����ʱ
    cout << "Time taken: " << duration.count() << " ms" << endl;

    // �ر��ļ�
    file.close();

    return 0;
}
