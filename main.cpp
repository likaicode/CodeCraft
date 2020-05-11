#include "bits/stdc++.h"
#include <pthread.h>
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

using namespace std;

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
#define TEST

const int lowDepth=3;
const int highDepth=7;
const int threadNum=4;

string testFile;
string resultFile;


void loadData()
{
    //mmap读取数据
}





void solve()
{
    //多线程dfs
}





void saveData()
{
    //多线程mmap分块写数据
}

int main(int argc, char *argv[])
{
#ifdef TEST
    testFile="./data/test_data.txt";
    resultFile="./projects/student/result.txt";
#else
    testFile="/data/test_data.txt";
    resultFile="/projects/student/result.txt";
#endif

    loadData();
    //simplifyAndSort();
    solve();
    saveData();

    return 0;
}