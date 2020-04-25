#include "bits/stdc++.h"
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

using namespace std;

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
#define TEST

const int lowDepth=3;
const int highDepth=7;
const int threadNum=4;

//类名：Solution
//作用：评估金融账号是否存在循环转账
//数据特征：28w条记录，按平均转账10次算大概2.8w账户，输出大概300w的环，会有大量的重复边

string testFile;
string resultFile;
//vector<vector<int>> results[8];  //按环的长度从3到7储存结果
//int nodeCnt=0;   //记录节点数
int maxId=0;      //记录节点最大序号
int ringCnt=0;   //记录总环数

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
bool visit[280000];
bool reachable[280000];
int out[8];
int current[125000];  //3+4中前3层反序遍历到的结果

int res3[3*500000];
int res4[4*500000];
int res5[5*1000000];
int res6[6*2000000];
int res7[7*3000000];
int *results[8]={0,0,0,res3,res4,res5,res6,res7};

//int inDegrees[280000];
//int outDegrees[280000];

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
    //int len1=0,len2=0;
#ifdef TEST
    cout<<"loading data ..."<<endl;
#endif
    for(char *p=mbuf;p-mbuf<len;p++)
    {
        while(*p!=',')
        {
            id1=id1*10+(*p-'0');
            p++;
            //len1++;
        }
        while(*(++p)!=','){
            id2=id2*10+(*p-'0');
            //len2++;
        }
        if(maxId<id1) maxId=id1;
        if(maxId<id2) maxId=id2;
        //maxId=max(maxId,max(id1,id2));
        G[id1][++G[id1][0]]=id2;
        GInv[id2][++GInv[id2][0]]=id1;
        if(idCom[id1].empty()){
            idCom[id1]=(to_string(id1)+',');
            idLF[id1]=(to_string(id1)+'\n');
            strSize[id1]=idCom[id1].size();
        }
        if(idCom[id2].empty()){
            idCom[id2]=(to_string(id2)+',');
            idLF[id2]=(to_string(id2)+'\n');
            strSize[id2]=idCom[id2].size();
        }
        //++inDegrees[id2];
        //++outDegrees[id1];
        //inputs[++inputs[0]]=id1;
        //inputs[++inputs[0]]=id2;
        //cout<<id1<<" ";
        id1=0;id2=0;
        //len1=0;len2=0;
        //p=(char*)memchr(p, '\n', 1000);
        while(*(++p)!='\n');
    }
    munmap(mbuf,len);
#ifdef TEST
    cout<<"done!"<<endl;
#endif
}

void simplifyAndSort()
{
    //q[0]=2,q[1]=2;
    //queue<int> q;
    for(int i=0;i<=maxId;i++){
        if(G[i][0]&&GInv[i][0]){
            sort(G[i]+1,G[i]+G[i][0]+1,greater<int>());
            sort(GInv[i]+1,GInv[i]+GInv[i][0]+1);
            continue;
        }
        if(G[i][0]) G[i][0]=0;
        if(GInv[i][0]) GInv[i][0]=0;
            //q[q[1]++]=i;
            //q.push(i);
    }
}

/*
void constructReverseIndex()
{
    //vector<unordered_map<int,vector<int>>> R;
    //使用R[i][j][k]来表示结点j到达结点i，中间经过结点k的路径详情，如果k不在我们已经搜索过的结点列表中，并且i是起点，那么j-k-i就是符合要求的路径的一部分。
    //具体来说，就是提前做深度为2的搜索（保存最后一层结点的入边），在第6层时直接根据现有结果进行判断，不进入第七层。
    R.resize(nodeCnt);
    for(int j=0;j<nodeCnt;j++){
        auto &arr=G[j];
        int arrSize=arr[0];
        for(int k=1;k<=arrSize;k++){//对i的所有邻接点k进行遍历 
            int &arrValue=arr[k];
            auto &arrk=G[arrValue];
            int arrkSize=arrk[0];
            for(int i=1;i<=arrkSize;i++){//对k的所有邻接点j进行遍历
                int &arrkValue=arrk[i];
                if(j>arrkValue&&arrValue>arrkValue) R[arrkValue][j].push_back(arrValue);//节点j能够通过节点k访问到节点i i!=j
            }
        }
    }
    for(int i=0;i<nodeCnt;i++){
        for(auto &x:R[i]){
            auto &xVec=x.second;
            if(xVec.size()>1){
                sort(xVec.begin(),xVec.end());
            }
        }
    }
}
*/

/*
void solveDFS(int head,int cur,int depth,int out[])
{
    //递归形式的dfs
    visit[cur]=true;
    out[++out[0]]=cur;
    auto &vCur=G[cur];
    int *beg=vCur+1, *end=vCur+vCur[0]+1;
    auto it=lower_bound(beg,end,head);
    //int it=binarySearch(vCur,head);//查找第一个大于等于head的位置
    //if(head==0) cout<<it<<" "<<endl;
    //auto it=lower_bound(vCur.begin(),vCur.end(),head);
    //handle [3,6]
    if(it!=end && *it==head && depth>=DEPTH_LOW && depth<DEPTH_HIGH) {
        results[depth][0]+=depth;
        int n=results[depth][0];
        for(int i=1;i<=out[0];i++){
            results[depth][n++]=out[i];
        }
        ++ringCnt;
    }

    if(depth<DEPTH_HIGH-1){
        for(;it!=end;++it){
            if(!visit[*it]){
                solveDFS(head,*it,depth+1,out);
            }
        }
    }else if(reachable[cur] && depth==DEPTH_HIGH-1){ //handle [7]
        auto &ks=R[head][cur];
        int sz=ks.size();
        for(int idx=0;idx<sz;++idx){
            int k=ks[idx];
            if(visit[k]) continue;
            out[++out[0]]=k;
            depth+=1;
            results[depth][0]+=depth;
            int n=results[depth][0];
            for(int i=1;i<=out[0];i++){
                results[depth][n++]=out[i];
            }
            depth-=1;
            --out[0];
            //results[depth+1].emplace_back(tmp);
            ++ringCnt;
        }
    }
    visit[cur]=false;
    --out[0];

}
*/


void solve()
{
    //solveDFS的外循环，可以用多线程来并行优化
    //if(nodeCnt>20000) openThread=true;    //当图的节点比较多时，才会采用多线程dfs
    //多线程dfs方法：将nodeCnt个节点分为threadNum块，开启threadNum个线程并在每个线程中搜索指定范围内的节点是否成环，G和R是线程共享的
    //每个线程访问自己的reachable和currentJs

    for(int i=maxId;i>=0;i--){
        if(G[i][0]){
            int &m1=GInv[i][0];
            for(int i1=m1;i1>0;i1--){  //反序遍历第1层
                int &v1=GInv[i][i1];
                if(v1<=i) break;
                reachable[v1]=true;
                current[++current[0]]=v1;
                int &m2=GInv[v1][0];
                for(int i2=m2;i2>0;i2--){  //反序遍历第2层
                    int &v2=GInv[v1][i2];
                    if(v2<=i) break;
                    reachable[v2]=true;
                    current[++current[0]]=v2;
                    int &m3=GInv[v2][0];
                    for(int i3=m3;i3>0;i3--){  //反序遍历第3层
                        int &v3=GInv[v2][i3];
                        if(v3<=i) break;
                        reachable[v3]=true;
                        current[++current[0]]=v3;
                    }
                }
            }
            //正向4层dfs遍历
            out[++out[0]]=i;
            visit[i]=true;
            int &n1=G[i][0];
            //int i1=lower_bound(G[i]+1,G[i]+n1+1,i)-(G[i]+1);
            for(int i1=1;i1<=n1;i1++){  //查找，out的第0个元素为环的长度，第1个元素为始节点，从第2个开始的节点倒序存储
                int &u1=G[i][i1];
                if(u1<=i) break;
                out[++out[0]]=u1;
                visit[u1]=true;
                int &n2=G[u1][0];
                //int i2=lower_bound(G[u1]+1,G[u1]+n2+1,i)-(G[u1]+1);
                for(int i2=1;i2<=n2;i2++){
                    int &u2=G[u1][i2];
                    if(u2<=i) break;
                    out[++out[0]]=u2;
                    visit[u2]=true;
                    int &n3=G[u2][0];
                    //int i3=lower_bound(G[u2]+1,G[u2]+n3+1,i)-(G[u2]+1);
                    for(int i3=1;i3<=n3;i3++){
                        int &u3=G[u2][i3];
                        if(u3<i) break;
                        if(u3==i){
                            results[3][0]+=3;  //检测到长度为3的环
                            int n=results[3][0];
                            for(int k=1;k<=3;k++){
                                results[3][n++]=out[k];
                            }
                            ++ringCnt;
                            break;
                        }
                        if(visit[u3]) continue;
                        out[++out[0]]=u3;
                        visit[u3]=true;
                        int &n4=G[u3][0];
                        //int i4=lower_bound(G[u3]+1,G[u3]+n4+1,i)-(G[u3]+1);
                        for(int i4=1;i4<=n4;i4++){
                            int &u4=G[u3][i4];
                            if(u4<i) break;
                            if(u4==i){
                                results[4][0]+=4;  //检测到长度为4的环
                                int n=results[4][0];
                                for(int k=1;k<=4;k++){
                                    results[4][n++]=out[k];
                                }
                                ++ringCnt;
                                break;
                            }
                            if(visit[u4]||(!reachable[u4])) continue;
                            out[++out[0]]=u4;
                            visit[u4]=true;
                            //只有在3层内可到达i的节点才能继续遍历
                            int &n5=G[u4][0];
                            //int i5=lower_bound(G[u4]+1,G[u4]+n5+1,i)-(G[u4]+1);
                            for(int i5=1;i5<=n5;i5++){
                                int &u5=G[u4][i5];
                                if(u5<i) break;
                                if(u5==i){
                                    results[5][0]+=5;  //检测到长度为5的环
                                    int n=results[5][0];
                                    for(int k=1;k<=5;k++){
                                        results[5][n++]=out[k];
                                    }
                                    ++ringCnt;
                                    break;
                                }
                                if(visit[u5]||(!reachable[u5])) continue;
                                out[++out[0]]=u5;
                                visit[u5]=true;
                                int &n6=G[u5][0];
                                //int i6=lower_bound(G[u5]+1,G[u5]+n6+1,i)-(G[u5]+1);
                                for(int i6=1;i6<=n6;i6++){
                                    int &u6=G[u5][i6];
                                    if(u6<i) break;
                                    if(u6==i){
                                        results[6][0]+=6;  //检测到长度为6的环
                                        int n=results[6][0];
                                        for(int k=1;k<=6;k++){
                                            results[6][n++]=out[k];
                                        }
                                        ++ringCnt;
                                        break;
                                    }
                                    if(visit[u6]||(!reachable[u6])) continue;
                                    out[++out[0]]=u6;
                                    visit[u6]=true;
                                    int &n7=G[u6][0];
                                    //int i7=lower_bound(G[u6]+1,G[u6]+n7+1,i)-(G[u6]+1);
                                    for(int i7=1;i7<=n7;i7++){
                                        int &u7=G[u6][i7];
                                        if(u7<i) break;
                                        if(u7==i){
                                            results[7][0]+=7;  //检测到长度为7的环
                                            int n=results[7][0];
                                            for(int k=1;k<=7;k++){
                                                results[7][n++]=out[k];
                                            }
                                            ++ringCnt;
                                            break;
                                        }
                                    }
                                    --out[0];
                                    visit[u6]=false;
                                }
                                --out[0];
                                visit[u5]=false;
                            }
                            --out[0];
                            visit[u4]=false;
                        }
                        --out[0];
                        visit[u3]=false;
                    }
                    --out[0];
                    visit[u2]=false;
                }
                --out[0];
                visit[u1]=false;
            }
            --out[0];
            visit[i]=false;
        }


        //将反序表遍历的标记清除
        int m=current[0];
        for(int i=1;i<=m;i++){
            reachable[current[i]]=false;
        }
        current[0]=0;
    }

#ifdef TEST
    cout<<"total rings: "<<ringCnt<<endl;
#endif
}

void saveData()
{
    //fwrite或者mmap写
#ifdef TEST
    cout<<"saving data ..."<<endl;
#endif
    //结果转成字符串
    //string resultStr(to_string(ringCnt)+'\n');
    //resultStr.reserve(100000);
    //resultStr+=to_string(ringCnt);
    //resultStr+="\n";
    string tmp=(to_string(ringCnt)+'\n').c_str();
    int len=tmp.size();
    memcpy(resultData,tmp.c_str(),len);
    for(int l=lowDepth;l<=highDepth;l++)
    {
        //sort(results[l].begin(),results[l].end());
        //results[l][0]+=l;
        int n=results[l][0];
        for(int i=n;i>=l;i-=l){
            for(int j=1;j<l;j++){
                int &sz=strSize[results[l][i]];
                memcpy(resultData+len,idCom[results[l][i]].c_str(),sz);
                len+=sz;
                i++;
            }
            int &sz=strSize[results[l][i]];
            memcpy(resultData+len,idLF[results[l][i]].c_str(),sz);
            len+=sz;
            i-=l-1;
            //resultStr+=idLF[results[l][i]];
        }
    }
    // FILE *fp=fopen(resultFile.c_str(),"w+");
    // fwrite(resultData,len,1,fp);
    // fclose(fp);
    
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
