#include "bits/stdc++.h"
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
#define TEST

typedef unsigned int uint;

using namespace std;


//类名：Solution
//作用：评估金融账号是否存在循环转账
//数据特征：28w条记录，按平均转账10次算大概2.8w账户，输出大概500w的环，会有大量的重复边

class Solution{
public:
    Solution(string testFile, string resultFile);
    void loadData();
    void dfs();
    void sortResult();
    void saveData();


private:
    string testFile;
    string resultFile;
    unordered_map<uint, vector<uint>> dataMap;  //邻接表
    unordered_map<uint, bool> visitMap;         //已访问矩阵，用于dfs的访问记录
    vector<vector<vector<uint>>> results;  //按环的长度从3到7储存结果
    uint cnt=0;   //记录总环数

};

Solution::Solution(string testF, string resultF)
{
    testFile=testF;
    resultFile=resultF;
    results.resize(8);
}

void Solution::loadData()
{
    int fd = open(testFile.c_str(), O_RDONLY);
    int len = lseek(fd,0,SEEK_END);
    //cout<<"file length: "<<len<<endl;
    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    close(fd);
    //cout<<mbuf[10]<<"."<<mbuf[11]<<mbuf[12]<<"."<<mbuf[13]<<endl;  //第11个是'\r',第12个是'\n'
    uint id1=0,id2=0;
    cout<<"loading data ..."<<endl;
    for(char *p=mbuf;*p&&p-mbuf<len;p++)
    {
        while(*p!=',')
        {
            id1=id1*10+(*p-'0');
            p++;
        }
        while(*(++p)!=',') id2=id2*10+(*p-'0');
        dataMap[id1].emplace_back(id2);
        visitMap[id1]=false;
        //cout<<id1<<" ";
        id1=0;id2=0;
        while(*(++p)!='\n');
        //p++;
    }
    munmap(mbuf,len);
    cout<<"done!"<<endl;
}

//禁忌的七重舞法天女
void Solution::dfs()
{
    cout<<"search ..."<<endl;
    for(auto &elem:dataMap)
    {
        vector<uint> out;
        uint firstElem=elem.first;
        out.emplace_back(firstElem);
        visitMap[firstElem]=true;
        for(int i2=0;i2<elem.second.size();i2++)
        {
            uint temp2=elem.second[i2];
            if(temp2<=firstElem) continue;  //判断要遍历的账户id是否小于最开始的id，保证结果的首元素值最小
            out.emplace_back(temp2);
            visitMap[temp2]=true;
            for(int i3=0;i3<dataMap[temp2].size();i3++)
            {
                uint temp3=dataMap[temp2][i3];
                if(temp3<=firstElem||visitMap[temp3]) continue;  //确保要遍历的第3层元素值大于首元素且之前未被访问过
                out.emplace_back(temp3);
                visitMap[temp3]=true;
                for(int i4=0;i4<dataMap[temp3].size();i4++)  //当长度为4时（包括首元素），需要检查第4个元素是否为首元素，以检测环的存在
                {
                    uint temp4=dataMap[temp3][i4];
                    if(temp4==firstElem)
                    {
                        results[3].emplace_back(out);
                        continue;
                    }
                    if(temp4<firstElem||visitMap[temp4]) continue;
                    out.emplace_back(temp4);
                    visitMap[temp4]=true;
                    //第5层循环遍历
                    for(int i5=0;i5<dataMap[temp4].size();i5++)
                    {
                        uint temp5=dataMap[temp4][i5];
                        if(temp5==firstElem)
                        {
                            results[4].emplace_back(out);  //因为第5个元素等于首元素，所以成环，环的长度为4
                            continue;
                        }
                        if(temp5<firstElem||visitMap[temp5]) continue;
                        out.emplace_back(temp5);
                        visitMap[temp5]=true;
                        //第6层循环
                        for(int i6=0;i6<dataMap[temp5].size();i6++)
                        {
                            uint temp6=dataMap[temp5][i6];
                            if(temp6==firstElem)
                            {
                                results[5].emplace_back(out);  //因为第5个元素等于首元素，所以成环，环的长度为4
                                continue;
                            }
                            if(temp6<firstElem||visitMap[temp6]) continue;
                            out.emplace_back(temp6);
                            visitMap[temp6]=true;
                            //第7层循环
                            for(int i7=0;i7<dataMap[temp6].size();i7++)
                            {
                                uint temp7=dataMap[temp6][i7];
                                if(temp7==firstElem)
                                {
                                    results[6].emplace_back(out);  //因为第5个元素等于首元素，所以成环，环的长度为4
                                    continue;
                                }
                                if(temp7<firstElem||visitMap[temp7]) continue;
                                out.emplace_back(temp7);
                                visitMap[temp7]=true;
                                //第8层循环
                                for(int i8=0;i8<dataMap[temp7].size();i8++)
                                {
                                    uint temp8=dataMap[temp7][i8];
                                    if(temp8==firstElem)
                                    {
                                        results[7].emplace_back(out);  //如果第8个元素等于首元素就加入out数组组成环，否则直接遍历下一个元素
                                        //cout<<firstElem<<" ";
                                    }
                                }
                                out.pop_back();  //弹出尾部元素temp7
                                visitMap[temp7]=false;
                            }
                            out.pop_back();
                            visitMap[temp6]=false;
                        }
                        out.pop_back();
                        visitMap[temp5]=false;
                    }
                    out.pop_back();
                    visitMap[temp4]=false;
                }
                out.pop_back();
                visitMap[temp3]=false;
            }
            out.pop_back();
            visitMap[temp2]=false;
        }
        out.pop_back();
        visitMap[firstElem]=false;
    }
    cout<<"done!"<<endl;
}

void Solution::sortResult()
{
    //继续优化：多线程分别运行sort()，或者直接用set存储results[3]
    cout<<"sorting result ..."<<endl;
    for(int l=3;l<=7;l++)
    {
        cnt+=results[l].size();
        sort(results[l].begin(),results[l].end());
    }
    cout<<"done!"<<endl;
    cout<<"总环数： "<<cnt<<endl;
    //for(auto& result:results[3])
    //    cout<<result[0]<<" ";
}

void Solution::saveData()
{
    cout<<"saving data ..."<<endl;
    ofstream output(resultFile);
    output<<cnt<<endl;
    for(int l=3;l<=7;l++)
    {
        auto& result=results[l];    //相同长度的所有环的记录
        for(int i=0;i<result.size();i++)
        {
            output<<result[i][0];
            for(int j=1;j<result[i].size();j++)
                output<<","<<result[i][j];
            output<<endl;
        }
    }
    cout<<"done!"<<endl;
}



int main(int argc, char *argv[])
{
#ifdef TEST
    //string testFile="./data/2020HuaweiCodecraft-TestData-master/38252/test_data.txt";
    string testFile="./data/test_data.txt";
    string resultFile="./projects/student/result.txt";
#else
    string testFile="/data/test_data.txt";
    string resultFile="/projects/student/result.txt";
#endif

    Solution solution(testFile,resultFile);
    solution.loadData();
    solution.dfs();
    solution.sortResult();
    solution.saveData();

    return 0;
}
