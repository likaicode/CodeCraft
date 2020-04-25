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
const int threadNum=10;

//类名：Solution
//作用：评估金融账号是否存在循环转账
//数据特征：28w条记录，按平均转账10次算大概2.8w账户，输出大概300w的环，会有大量的重复边

string testFile;
string resultFile;
//vector<vector<int>> results[8];  //按环的长度从3到7储存结果
//int nodeCnt=0;   //记录节点数
int maxId=0;      //记录节点最大序号
int nodeCnt=0;
//int ringCnt=0;   //记录总环数
int ringCnt[threadNum];

//vector<unordered_map<int,vector<int>>> R;

//int inputs[560001];
int G[280000][50];    //G[u][v]
int GInv[280000][50]; //G[v][u]
string idCom[280000];
string idLF[280000];
//const char *idCom[280000];
//const char *idLF[280000];
int strSize[280000];
char resultData[300000000];
//int queue[50000];
bool visit[threadNum][280000];
bool reachable[threadNum][280000];
bool reachableInv[threadNum][280000];
//int out[threadNum][8];
int reachables[threadNum][125000];  //3+4中前3层反序遍历到的结果
int reachableInvs[threadNum][125000];

int res3[threadNum][3*500000];
int res4[threadNum][4*500000];
int res5[threadNum][5*1000000];
int res6[threadNum][6*2000000];
int res7[threadNum][7*3000000];
int *results[threadNum][8];

//int inDegrees[280000];
//int outDegrees[280000];
struct ThreadInfo{  //每个线程的左右边界
    //int l;
    //int r;
    int threadCnt;
};

//int infoTmp[30];
ThreadInfo infos[threadNum];
pthread_t tids[threadNum];

char str[30];

void loadData()
{
    int fd = open(testFile.c_str(), O_RDONLY);
    //可以根据文件大小设置是否启动多线程，1个线程/4个以上线程
    int len = lseek(fd,0,SEEK_END);
    //cout<<"file length: "<<len<<endl;
    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    close(fd);
    //cout<<mbuf[10]<<"."<<mbuf[11]<<mbuf[12]<<"."<<mbuf[13]<<endl;  //第11个是'\r',第12个是'\n'
    int id1=0,id2=0;
    int diff=0;
    char *head=NULL;
    bool flag=true;
    //int len1=0,len2=0;
#ifdef TEST
    cout<<"loading data ..."<<endl;
#endif
    char *p=mbuf;
    while(*p!='\0')
    {
        head=p;
        while(*p!='\n'){
            if(flag&&*p==','){
                memset(str,0,sizeof(str));
                diff=p-head;
                memcpy(str,head,diff);
                id1=stoi(str);
                //cout<<id1<<" ";
                if(maxId<id1) maxId=id1;
                if(idCom[id1].empty()){
                    ++nodeCnt;
                    str[diff]=',';
                    idCom[id1]=str;
                    str[diff]='\n';
                    idLF[id1]=str;
                    strSize[id1]=diff+1;
                }
                head=++p;
                flag=false;
            }
            if((!flag)&&*p==','){
                memset(str,0,sizeof(str));
                diff=p-head;
                memcpy(str,head,diff);
                id2=stoi(str);
                //cout<<id2<<" ";
                if(maxId<id2) maxId=id2;
                if(idCom[id2].empty()){
                    ++nodeCnt;
                    str[diff]=',';
                    idCom[id2]=str;
                    str[diff]='\n';
                    idLF[id2]=str;
                    strSize[id2]=diff+1;
                }
                ++p;
                flag=true;
                G[id1][++G[id1][0]]=id2;
                GInv[id2][++GInv[id2][0]]=id1;
            }
            ++p;
        }
        ++p;

        //++inDegrees[id2];
        //++outDegrees[id1];
    }
    munmap(mbuf,len);

    for(int tc=0;tc<threadNum;tc++){
        results[tc][3]=res3[tc];
        results[tc][4]=res4[tc];
        results[tc][5]=res5[tc];
        results[tc][6]=res6[tc];
        results[tc][7]=res7[tc];
    }

#ifdef TEST
    cout<<"done!"<<endl;
    cout<<"total node: "<<nodeCnt<<endl;
#endif
}

void simplifyAndSort()
{
#ifdef TEST
    cout<<"sorting..."<<endl;
#endif
    //对id号进行分块，4线程为例，总结点数除以20(4*5)得到perCnt，第0个线程从0到perCnt倒序遍历，第1个线程从perCnt到2×perCnt，第2个从2×perCnt到4×perCnt，第3个负责剩下的
    //int perCnt=nodeCnt/(threadNum*(threadNum+1));
    //int perCnt=nodeCnt/threadNum;
    //int nodetmp=0;
    //int tc=0;
    for(int i=0;i<=maxId;i++){
        if(G[i][0]&&GInv[i][0]){
            // ++nodetmp;
            // if(nodetmp==1){
            //     infos[0].l=i-1;
            //     ++tc;
            // }
            // else if(nodetmp==tc*perCnt){
            //     infos[tc-1].r=i; //左开右闭
            //     infos[tc].l=i;
            //     ++tc;
            // }

            sort(G[i]+1,G[i]+G[i][0]+1,greater<int>());
            sort(GInv[i]+1,GInv[i]+GInv[i][0]+1);
            continue;
        }
        if(G[i][0]) G[i][0]=0;
        if(GInv[i][0]) GInv[i][0]=0;
            //q[q[1]++]=i;
            //q.push(i);
    }
    //infos[tc-1].r=maxId;
#ifdef TEST
    cout<<"done!"<<endl;
#endif
}

void *run(void *threadInfo)
{
    //solveDFS的外循环，可以用多线程来并行优化
    //if(nodeCnt>20000) openThread=true;    //当图的节点比较多时，才会采用多线程dfs
    //多线程dfs方法：将nodeCnt个节点分为threadNum块，开启threadNum个线程并在每个线程中搜索指定范围内的节点是否成环，G和R是线程共享的
    //每个线程访问自己的reachable和currentJs

    ThreadInfo* info = (ThreadInfo*)threadInfo;
    //int l=info->l, r=info->r;
    int tc=info->threadCnt;
    int out[8];
    out[0]=0;
#ifdef TEST
    //cout<<"thread"<<tc<<"in"<<endl;
#endif
    for(int i=maxId-maxId%threadNum+tc;i>=0;i-=threadNum){
        if(G[i][0]){
            for(int i1=GInv[i][0];i1>0;i1--){  //反序遍历第1层
                int &v1=GInv[i][i1];
                if(v1<=i) break;
                reachable[tc][v1]=true;  //正向一层可到达
                reachableInv[tc][v1]=true;
                reachables[tc][++reachables[tc][0]]=v1;
                reachableInvs[tc][++reachableInvs[tc][0]]=v1;
                for(int i2=GInv[v1][0];i2>0;i2--){  //反序遍历第2层
                    int &v2=GInv[v1][i2];
                    if(v2<=i) break;
                    reachableInv[tc][v2]=true;
                    reachableInvs[tc][++reachableInvs[tc][0]]=v2;
                    for(int i3=GInv[v2][0];i3>0;i3--){  //反序遍历第3层
                        int &v3=GInv[v2][i3];
                        if(v3<=i) break;
                        reachableInv[tc][v3]=true;
                        reachableInvs[tc][++reachableInvs[tc][0]]=v3;
                    }
                }
            }
            //正向4层dfs遍历
            out[++out[0]]=i;
            visit[tc][i]=true;
            for(int i1=1;i1<=G[i][0];i1++)  //查找，out的第0个元素为环的长度，第1个元素为始节点，从第2个开始的节点倒序存储
            {  
                int &u2=G[i][i1];  //环中的第二个点
                if(u2<=i) break;
                out[++out[0]]=u2;
                visit[tc][u2]=true;
                for(int i2=1;i2<=G[u2][0];i2++)
                {
                    int &u3=G[u2][i2];
                    if(u3<=i) break;
                    out[++out[0]]=u3;
                    visit[tc][u3]=true;
                    if(reachable[u3])
                    {
                        results[tc][3][0]+=3;  //检测到长度为3的环
                        int n=results[tc][3][0];
                        for(int k=1;k<=3;k++)
                        {
                            results[tc][3][n++]=out[k];
                        }
                        ++ringCnt[tc];
                    }
                    for(int i3=1;i3<=G[u3][0];i3++)
                    {
                        int &u4=G[u3][i3];
                        if(u4<i) break;
                        if(visit[tc][u4]) continue;
                        out[++out[0]]=u4;
                        visit[tc][u4]=true;
                        if(reachable[u4])
                        {
                            results[tc][4][0]+=4;  //检测到长度为4的环
                            int n=results[tc][4][0];
                            for(int k=1;k<=4;k++)
                            {
                                results[tc][4][n++]=out[k];
                            }
                            ++ringCnt[tc];
                        }
                        for(int i4=1;i4<=G[u4][0];i4++)
                        {
                            int &u5=G[u4][i4];
                            if(u5<i) break;
                            if(visit[tc][u5]||(!reachableInv[tc][u5])) continue;
                            out[++out[0]]=u5;
                            visit[tc][u5]=true;
                            if(reachable[u5])
                            {
                                results[tc][5][0]+=5;  //检测到长度为5的环
                                int n=results[tc][5][0];
                                for(int k=1;k<=5;k++)
                                {
                                    results[tc][5][n++]=out[k];
                                }
                                ++ringCnt[tc];
                            }
                            for(int i5=1;i5<=G[u5][0];i5++)
                            {
                                int &u6=G[u5][i5];
                                if(u6<i) break;
                                if(visit[tc][u6]||(!reachableInv[tc][u6])) continue;
                                out[++out[0]]=u6;
                                visit[tc][u6]=true;
                                if(reachable[u6])
                                {
                                    results[tc][6][0]+=6;  //检测到长度为6的环
                                    int n=results[tc][6][0];
                                    for(int k=1;k<=6;k++)
                                    {
                                        results[tc][6][n++]=out[k];
                                    }
                                    ++ringCnt[tc];
                                }
                                for(int i6=1;i6<=G[u6][0];i6++)
                                {
                                    int &u7=G[u6][i6];
                                    if(u7<i) break;
                                    if(visit[tc][u7]||(!reachableInv[tc][u7])) continue;
                                    out[++out[0]]=u7;
                                    visit[tc][u7]=true;
                                    if(reachable[u7])
                                    {
                                        results[tc][7][0]+=7;  //检测到长度为6的环
                                        int n=results[tc][7][0];
                                        for(int k=1;k<=7;k++)
                                        {
                                            results[tc][7][n++]=out[k];
                                        }
                                        ++ringCnt[tc];
                                    }
                                    --out[0];
                                    visit[tc][u7]=false;
                                }
                                --out[0];
                                visit[tc][u6]=false;
                            }
                            --out[0];
                            visit[tc][u5]=false;
                        }
                        --out[0];
                        visit[tc][u4]=false;
                    }
                    --out[0];
                    visit[tc][u3]=false;
                }
                --out[0];
                visit[tc][u2]=false;
            }
            --out[0];
            visit[tc][i]=false;
        }


        //将反序表遍历的标记清除
        int m=reachables[tc][0];
        for(int i=1;i<=m;i++){
            reachable[tc][reachables[tc][i]]=false;
        }
        reachables[tc][0]=0;
        int m=reachableInvs[tc][0];
        for(int i=1;i<=m;i++){
            reachableInv[tc][reachableInvs[tc][i]]=false;
        }
        reachableInvs[tc][0]=0;
    }

#ifdef TEST
    cout<<"thread"<<tc<<" rings: "<<ringCnt[tc]<<endl;
    //cout<<"thread"<<tc<<"out"<<endl;
#endif
}



//多线程3+4迭代式dfs
void solve()
{
#ifdef TEST
    cout<<"dfs..."<<endl;
#endif

    for(int i=0;i<threadNum;i++){
        infos[i].threadCnt=i;
        pthread_create(&tids[i], NULL, run, (void *)&(infos[i]));
    }
    //主线程充当第0个线程来计算

    for(int i=0;i<threadNum;i++){
        pthread_join(tids[i], NULL);
    }

#ifdef TEST
    cout<<"done!"<<endl;
#endif
}


void saveData()
{
    //mmap写
#ifdef TEST
    cout<<"saving data ..."<<endl;
#endif
    //结果转成字符串
    int totalRings=0;
    for(int tc=0;tc<threadNum;tc++){
        totalRings+=ringCnt[tc];
    }
#ifdef TEST
    cout<<"total rings: "<<totalRings<<endl;
#endif
    string tmp=(to_string(totalRings)+'\n').c_str();
    int len=tmp.size();
    memcpy(resultData,tmp.c_str(),len);
    for(int l=lowDepth;l<=highDepth;l++)
    {
        for(int tc=0;tc<threadNum;tc++){
            int n=results[tc][l][0];
            for(int i=n;i>=l;i-=l){
                for(int j=1;j<l;j++){
                    int &sz=strSize[results[tc][l][i]];
                    memcpy(resultData+len,idCom[results[tc][l][i]].c_str(),sz);
                    len+=sz;
                    i++;
                }
                int &sz=strSize[results[tc][l][i]];
                memcpy(resultData+len,idLF[results[tc][l][i]].c_str(),sz);
                len+=sz;
                i-=l-1;
                //resultStr+=idLF[results[l][i]];
            }
        }
    }
    
    //mmap写到文件
    int fd = open(resultFile.c_str(),O_RDWR|O_CREAT,0666);
    //int len=resultStr.length();
    ftruncate(fd,len);
    int strLen=1024*1024;
    char* pbuf=NULL;
    int cnt=len/strLen;
    int mod=len%strLen;
#ifdef TEST
    cout<<cnt<<"MB+"<<mod<<endl;
#endif
    for(int i=0;i<cnt;i++){
        pbuf=(char *)mmap(NULL,strLen,PROT_READ|PROT_WRITE,MAP_SHARED,fd,i*strLen);
        memcpy(pbuf,resultData+i*strLen,strLen);
        munmap(pbuf,strLen);
    }
    pbuf=(char*) mmap(NULL,mod,PROT_READ|PROT_WRITE,MAP_SHARED,fd,cnt*strLen);
    memcpy(pbuf,resultData+cnt*strLen,mod);
    munmap(pbuf,mod);

    close(fd);
#ifdef TEST
    cout<<"done!"<<endl;
#endif
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
    //Solution solution(testFile,resultFile);
    loadData();
    //constructGraph();
    simplifyAndSort();
    solve();
    saveData();

    return 0;
}
