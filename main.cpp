#include "bits/stdc++.h"
#include <pthread.h>
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

using namespace std;

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
//#define TEST

const int lowDepth=3;
const int highDepth=7;
const int threadNum=12;

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

//int inputs[560001];
int G[280000][50];    //G[u][v]
int GInv[280000][50]; //G[v][u]
string idCom[280000];
string idLF[280000];
//const char *idCom[280000];
//const char *idLF[280000];

int strSize[280000];
int totalLength;
//char resultData[300000000];

int outfd;                //输出文件的描述符
int perCnt;               //多线程写时每个线程负责perCnt×strLen个字节
int strLen=1024*1024;     //1M的文件大小
//int queue[50000];

bool visit[threadNum][280000];
bool reachable[threadNum][280000];
bool reachableInv[threadNum][280000];
//int out[threadNum][8];
//int reachables[threadNum][125000];  //3+4中前3层反序遍历到的结果
//int reachableInvs[threadNum][125000];

int res3[threadNum][3*200000];
int res4[threadNum][4*200000];
int res5[threadNum][5*400000];
int res6[threadNum][6*800000];
int res7[threadNum][7*1000000];
int *results[threadNum][8];

int resSize3[threadNum][280000];  //每个id开头的字节长度，用于计算偏移量
int resSize4[threadNum][280000];
int resSize5[threadNum][280000];
int resSize6[threadNum][280000];
int resSize7[threadNum][280000];
int *resultSize[threadNum][8];

int resOffSet3[threadNum][280000];  //memcpy拷贝时的resultData偏移量
int resOffSet4[threadNum][280000];
int resOffSet5[threadNum][280000];
int resOffSet6[threadNum][280000];
int resOffSet7[threadNum][280000];
int *resultOffSet[threadNum][8];

//int inDegrees[280000];
//int outDegrees[280000];
struct ThreadInfo{  //每个线程的序号
    //int l;
    //int r;
    int tc;
};

//int infoTmp[30];
ThreadInfo infos[threadNum];
pthread_t tids[threadNum];

//char str1[20];
//char str2[20];

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
    //int diff1=0,diff2=0;
    char *root=NULL;
    //int len1=0,len2=0;
#ifdef TEST
    cout<<"loading data ..."<<endl;
#endif
    char *p=mbuf;
    while(*p!='\0')
    {
        root=p;
        while(*p!=',') ++p;
        string tmp1(root,p-root);
        id1=stoi(tmp1);
        root=++p;

        while(*p!=',') ++p;
        string tmp2(root,p-root);
        id2=stoi(tmp2);
        ++p;
        while(*p!='\n') ++p;
        ++p;
        if(id1>50000||id2>50000) continue;

        if(maxId<id1) maxId=id1;
        if(idCom[id1].empty()){
            //++nodeCnt;
            idCom[id1]=tmp1+',';
            idLF[id1]=tmp1+'\n';
            strSize[id1]=tmp1.size()+1;
        }
        if(maxId<id2) maxId=id2;
        if(idCom[id2].empty()){
            //++nodeCnt;
            idCom[id2]=tmp2+',';
            idLF[id2]=tmp2+'\n';
            strSize[id2]=tmp2.size()+1;
        }
        G[id1][++G[id1][0]]=id2;
        GInv[id2][++GInv[id2][0]]=id1;
/*
            if(flag&&*p==','){
                memset(str,0,sizeof(str));
                diff=p-head;
                memcpy(str,head,diff);
                id1=atoi(str);
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
                //线上提交
                //if(id1>50000||id2>50000) continue;
                G[id1][++G[id1][0]]=id2;
                GInv[id2][++GInv[id2][0]]=id1;
            }
            ++p;
        }
        ++p;
*/
    }
    munmap(mbuf,len);

    for(int tc=0;tc<threadNum;tc++){
        results[tc][3]=res3[tc];
        results[tc][4]=res4[tc];
        results[tc][5]=res5[tc];
        results[tc][6]=res6[tc];
        results[tc][7]=res7[tc];

        resultSize[tc][3]=resSize3[tc];
        resultSize[tc][4]=resSize4[tc];
        resultSize[tc][5]=resSize5[tc];
        resultSize[tc][6]=resSize6[tc];
        resultSize[tc][7]=resSize7[tc];

        resultOffSet[tc][3]=resOffSet3[tc];
        resultOffSet[tc][4]=resOffSet4[tc];
        resultOffSet[tc][5]=resOffSet5[tc];
        resultOffSet[tc][6]=resOffSet6[tc];
        resultOffSet[tc][7]=resOffSet7[tc];
    }

#ifdef TEST
    cout<<"done!"<<endl;
    //cout<<"total node: "<<nodeCnt<<endl;
#endif
}

void simplifyAndSort()
{
#ifdef TEST
    cout<<"sorting..."<<endl;
#endif
    for(int i=0;i<=maxId;i++){
        if(G[i][0]&&GInv[i][0]){
            sort(G[i]+1,G[i]+G[i][0]+1,greater<int>());
            sort(GInv[i]+1,GInv[i]+GInv[i][0]+1);
            continue;
        }
        if(G[i][0]) G[i][0]=0;
        if(GInv[i][0]) GInv[i][0]=0;
    }
#ifdef TEST
    cout<<"done!"<<endl;
#endif
}

void *run(void *threadInfo)
{
    //solveDFS的外循环，可以用多线程来并行优化
    //if(nodeCnt>20000) openThread=true;    //当图的节点比较多时，才会采用多线程dfs
    //多线程dfs方法：将nodeCnt个节点分为threadNum块，开启threadNum个线程并在每个线程中搜索指定范围内的节点是否成环，G和R是线程共享的
    ThreadInfo* info = (ThreadInfo*)threadInfo;
    //int l=info->l, r=info->r;
    int tc=info->tc;
    int out[8];
    out[0]=0;
#ifdef TEST
    //cout<<"thread"<<tc<<"in"<<endl;
#endif
    for(int i=maxId-maxId%threadNum+tc;i>=0;i-=threadNum)
    {
        if(G[i][0])
        {
            for(int i1=GInv[i][0];i1>0;i1--)
            {  //反序遍历第1层
                int &v1=GInv[i][i1];
                if(v1<=i) break;
                reachable[tc][v1]=true;  //正向一层可到达
                reachableInv[tc][v1]=true;
                //reachables[tc][++reachables[tc][0]]=v1;
                //reachableInvs[tc][++reachableInvs[tc][0]]=v1;
                for(int i2=GInv[v1][0];i2>0;i2--)
                {  //反序遍历第2层
                    int &v2=GInv[v1][i2];
                    if(v2<=i) break;
                    reachableInv[tc][v2]=true;
                    //reachableInvs[tc][++reachableInvs[tc][0]]=v2;
                    for(int i3=GInv[v2][0];i3>0;i3--)
                    {  //反序遍历第3层
                        int &v3=GInv[v2][i3];
                        if(v3<=i) break;
                        reachableInv[tc][v3]=true;
                        //reachableInvs[tc][++reachableInvs[tc][0]]=v3;
                    }
                }
            }
            //正向4层dfs遍历
            out[++out[0]]=i;
            visit[tc][i]=true;
            for(int i1=1;i1<=G[i][0];i1++)  //查找，out的第0个元素为环的长度，第1个元素为始节点，从第2个开始的节点倒序存储
            {  
                int &u2=G[i][i1];  //环中的第二个点
                if(u2<i) break;
                out[++out[0]]=u2;
                visit[tc][u2]=true;
                for(int i2=1;i2<=G[u2][0];i2++)
                {
                    int &u3=G[u2][i2];
                    if(u3<=i) break;
                    out[++out[0]]=u3;
                    visit[tc][u3]=true;
                    if(reachable[tc][u3])
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
                        if(reachable[tc][u4])
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
                            if(reachable[tc][u5])
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
                                if(reachable[tc][u6])
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
                                    if(reachable[tc][u7])
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
        for(int i1=GInv[i][0];i1>0;i1--)
        {  //反序遍历第1层
            int &v1=GInv[i][i1];
            if(v1<=i) break;
            reachable[tc][v1]=false;  //正向一层可到达
            reachableInv[tc][v1]=false;
            for(int i2=GInv[v1][0];i2>0;i2--)
            {  //反序遍历第2层
                int &v2=GInv[v1][i2];
                if(v2<=i) break;
                reachableInv[tc][v2]=false;
                for(int i3=GInv[v2][0];i3>0;i3--)
                {  //反序遍历第3层
                    int &v3=GInv[v2][i3];
                    if(v3<=i) break;
                    reachableInv[tc][v3]=false;
                }
            }
        }

    }

    //resultSize
    int len=0;
    for(int l=lowDepth;l<=highDepth;l++)
    {
        int index=results[tc][l][0];
        for(int i=tc;i<=maxId-maxId%threadNum+tc;i+=threadNum)
        {
            len=0;
            //resultSize[tc][l][i]
            while(index>=l&&results[tc][l][index]==i)
            {
                for(int n=0;n<l;n++)
                {
                    len+=strSize[results[tc][l][index+n]];
                }
                index-=l;
            }
            resultSize[tc][l][i]=len;
        }
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
        infos[i].tc=i;
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

void *runCpy(void *threadInfo)
{
    //在多线程中拼接字符串
    ThreadInfo* info = (ThreadInfo*)threadInfo;
    //int l=info->l, r=info->r;
    int tc=info->tc;
    char *pbuf=(char *)mmap(NULL,totalLength,PROT_READ|PROT_WRITE,MAP_SHARED,outfd,0);
    for(int l=lowDepth;l<=highDepth;l++)
    {
        //for(int k=0;k<=maxId;k++)
        for(int i=tc;i<=maxId-maxId%threadNum+tc;i+=threadNum)
        {
            int index=results[tc][l][0];
            int len=resultOffSet[tc][l][i];
            while(index>=l&&results[tc][l][index]==i)
            {
                for(int n=0;n<l-1;n++)
                {
                    int &sz=strSize[results[tc][l][index+n]];
                    memcpy(pbuf+len,idCom[results[tc][l][index+n]].c_str(),sz);
                    len+=sz;
                }
                int &sz=strSize[results[tc][l][index+l-1]];
                memcpy(pbuf+len,idLF[results[tc][l][index+l-1]].c_str(),sz);
                len+=sz;
                results[tc][l][0]-=l;
                index-=l;
            }
        }
    }
    munmap(pbuf,totalLength);
}

/*
void *runWrite(void *arg)
{
    //mmap
    int tc=(long)arg;
    //cout<<"thread"<<tc<<endl;
    char* pbuf=NULL;
    int start=tc*perCnt*strLen;
    for(int i=0;i<perCnt;i++){
        pbuf=(char *)mmap(NULL,strLen,PROT_READ|PROT_WRITE,MAP_SHARED,outfd,start+i*strLen);
        memcpy(pbuf,resultData+start+i*strLen,strLen);
        munmap(pbuf,strLen);
    }
    //cout<<"thread out"<<tc<<endl;
}
*/

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
    totalLength=tmp.size();
    //memcpy(resultData,tmp.c_str(),totalLength);
    //获取偏移量
    for(int l=lowDepth;l<=highDepth;l++)
    {
        for(int k=0;k<=maxId;k++)
        {
            int idx=k%threadNum;
            if(k==0&&l==3){
                resultOffSet[idx][l][k]=totalLength;
                continue;
            }
            if(k==0&&l!=3){
                resultOffSet[idx][l][k]=resultOffSet[maxId%threadNum][l-1][maxId]+resultSize[maxId%threadNum][l-1][maxId];
                continue;
            }
            if(idx){
                resultOffSet[idx][l][k]=resultOffSet[idx-1][l][k-1]+resultSize[idx-1][l][k-1];
            }
            else if(idx==0){
                resultOffSet[idx][l][k]=resultOffSet[threadNum-1][l][k-1]+resultSize[threadNum-1][l][k-1];
                //cout<<resultOffSet[idx][l][k]<<" ";
            }
        }
    }
    totalLength=resultOffSet[maxId%threadNum][7][maxId]+resultSize[maxId%threadNum][7][maxId];

    //扩充文件大小
    outfd = open(resultFile.c_str(),O_RDWR|O_CREAT,0666);
    int ret1=ftruncate(outfd,totalLength);
    int ret2=write(outfd,tmp.c_str(),tmp.size());
    int cnt=totalLength/strLen;
    int mod=totalLength%strLen;
#ifdef TEST
    cout<<cnt<<"MB+"<<mod<<endl;
#endif

    //多线程拼接字符串结果
    for(int i=0;i<threadNum;i++){
        infos[i].tc=i;
        pthread_create(&tids[i], NULL, runCpy, (void *)&(infos[i]));
    }
    for(int i=0;i<threadNum;i++){
        pthread_join(tids[i], NULL);
    }

/*
    int writeThreadNum=0;
    if(cnt>12){
        writeThreadNum=3;  //结果文件大于12M，开3个线程，第4个线程为主线程
    }
    //mmap多线程写到文件
    if(writeThreadNum==0)
    {
        char* pbuf=NULL;
        for(int i=0;i<cnt;i++){
            pbuf=(char *)mmap(NULL,strLen,PROT_READ|PROT_WRITE,MAP_SHARED,outfd,i*strLen);
            memcpy(pbuf,resultData+i*strLen,strLen);
            munmap(pbuf,strLen);
        }
        pbuf=(char*) mmap(NULL,mod,PROT_READ|PROT_WRITE,MAP_SHARED,outfd,cnt*strLen);
        memcpy(pbuf,resultData+cnt*strLen,mod);
        munmap(pbuf,mod);
    }
    else if(writeThreadNum==3)
    {
        //开threadN个mmap写线程
        perCnt=cnt/writeThreadNum;
        for(int i=0;i<writeThreadNum;i++){
            pthread_create(&tids[i], NULL, runWrite, (void *)(i));
        }
        //主线程中处理尾部部分的写文件
        char* pbuf=NULL;
        int start=writeThreadNum*perCnt*strLen;
        for(int i=0;i<cnt-3*perCnt;i++){
            pbuf=(char *)mmap(NULL,strLen,PROT_READ|PROT_WRITE,MAP_SHARED,outfd,start+i*strLen);
            memcpy(pbuf,resultData+start+i*strLen,strLen);
            munmap(pbuf,strLen);
        }
        pbuf=(char*) mmap(NULL,mod,PROT_READ|PROT_WRITE,MAP_SHARED,outfd,cnt*strLen);
        memcpy(pbuf,resultData+cnt*strLen,mod);
        munmap(pbuf,mod);

        //等待结果
        for(int i=0;i<writeThreadNum;i++){
            pthread_join(tids[i], NULL);
        }
    }
*/
    close(outfd);
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

    loadData();
    simplifyAndSort();
    solve();
    saveData();

    return 0;
}
