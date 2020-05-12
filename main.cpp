#include "bits/stdc++.h"
#include <pthread.h>
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

using namespace std;

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
#define TEST

typedef unsigned long long ull;
typedef unsigned int uint;

const int lowDepth=3;
const int highDepth=7;
const int threadNum=10;

string testFile;
string resultFile;

vector<vector<int>> G;
vector<vector<int>> GInv;
vector<string> idCom;
vector<string> idLF;
vector<int> strSize;
unordered_map<uint, int> idHash; //sorted id to 0...n
unordered_map<ull, int> idCash;
vector<uint> ids; //0...n to sorted id
vector<uint> inputs; //u-v pairs
//vector<int> inDegrees;
int nodeCnt;
int ringCnt[threadNum];
vector<vector<uint>> results[threadNum][8];
vector<int> resultSize[threadNum][8];   //每个id开头的字节长度，用于计算偏移量
vector<int> resultOffSet[threadNum][8];  //memcpy拷贝时的resultData偏移量

vector<bool> visit[threadNum];
vector<bool> reachable[threadNum];
vector<bool> reachableInv[threadNum];

struct ThreadInfo{  //每个线程的序号
    //int l;
    //int r;
    int tc;
};
ThreadInfo infos[threadNum];
pthread_t tids[threadNum];

inline bool check(int x, int y) {
    if(x==0 || y==0) return false;
    return x <= 5ll * y && y <= 3ll * x;
}

void loadData()
{
    //mmap读取数据
    int fd = open(testFile.c_str(), O_RDONLY);
    int len = lseek(fd,0,SEEK_END);
    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    close(fd);
    inputs.reserve(2000000);
    uint id1=0,id2=0,cash=0;
    int lf=0;
    char *root=NULL;
#ifdef TEST
    cout<<"loading data ..."<<endl;
#endif
    char *p=mbuf;
    //int cnt=0;
    while(*p!='\0')
    {
        root=p;
        while(*p!=',') ++p;
        string tmp1(root,p-root);
        id1=atoi(tmp1.c_str());
        root=++p;

        while(*p!=',') ++p;
        string tmp2(root,p-root);
        id2=atoi(tmp2.c_str());
        root=++p;

        while(*p!='\n') ++p;
        if((!lf) && (*(p-1)=='\r')) lf=1;
        string temp3(root,p-root-lf);
        cash=atoi(temp3.c_str());
        ++p;

        inputs.push_back(id1);
        inputs.push_back(id2);
        idCash[(ull) id1 << 32 | id2] = cash;

        //++cnt;
    }
    munmap(mbuf,len);
#ifdef TEST
    cout<<"done!"<<endl;
    //cout<<"records: "<<cnt<<endl;
#endif
}


void constructGraph()
{
    //建图
    auto tmp = inputs;
    sort(tmp.begin(), tmp.end());
    tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
    nodeCnt = tmp.size();
    ids = tmp;
    nodeCnt = 0;
    for (uint &x:tmp) {
        idHash[x] = nodeCnt++;
    }
#ifdef TEST
    cout<<"total nodes: "<<nodeCnt<<endl;
#endif
    int sz=inputs.size();
    G.resize(nodeCnt+threadNum);
    GInv.resize(nodeCnt+threadNum);
    //inDegrees=vector<int>(nodeCnt,0);
    idCom.resize(nodeCnt);
    idLF.resize(nodeCnt);
    strSize.resize(nodeCnt);
    for (int i=0;i<sz;i+=2){
        int u=idHash[inputs[i]], v=idHash[inputs[i+1]];
        G[u].push_back(v);
        GInv[v].push_back(u);
        //++inDegrees[v];
    }
    for(int i=0;i<nodeCnt;i++){
        idCom[i]=to_string(ids[i])+',';
        idLF[i]=to_string(ids[i])+'\n';
        strSize[i]=idCom[i].size()+1;
        sort(G[i].begin(),G[i].end(),greater<int>());
        sort(GInv[i].begin(),GInv[i].end(),greater<int>());
    }
}

bool checkAns(vector<uint> tmp, int depth){
    for (int i=0;i<depth;i++){
        int l=tmp[(i+depth-1)%depth], m=tmp[i], r=tmp[(i+1)%depth];
        if (!check(idCash[(ull)l<<32 | m], idCash[(ull)m<<32 | r])) return false;
    }
    return true;
}

// void dfs(int head, int cur, int depth, int *out){
//     visit[cur] = true;
//     out[++out[0]]=cur;
//     //path.push_back(cur);
//     for (int &v:G[cur]){
//         if(v<head) continue;
//         if(v==head && depth>=lowDepth && depth<=highDepth){
//             vector<uint> tmp(out[0]);
//             for(int i=1;i<=out[0];i++)
//                 tmp[i-1]=ids[out[i]];
//             if(checkAns(tmp, depth)){        //check需要实际的id
//                 results[depth].emplace_back(tmp);
//                 ++totalRings;
//             }
//         }
//         if(depth < 7 && !visit[v] && v > head){
//             dfs(head, v, depth + 1, out);
//         }
//     }
//     visit[cur] = false;
//     --out[0];
// }

void *run(void *threadInfo)
{
    ThreadInfo* info = (ThreadInfo*)threadInfo;
    int tc=info->tc;
    visit[tc].resize(nodeCnt);
    reachable[tc].resize(nodeCnt);
    reachableInv[tc].resize(nodeCnt);
    for(int l=lowDepth;l<=highDepth;l++){
        resultSize[tc][l].resize(nodeCnt+threadNum);
        resultOffSet[tc][l].resize(nodeCnt+threadNum);
    }
    int out[8];
    out[0]=0;
    for(int i=nodeCnt-nodeCnt%threadNum+tc;i>=0;i-=threadNum)
    {
        if(!G[i].empty()&&!GInv[i].empty())
        {
            for(int i1=0;i1<GInv[i].size();i1++){
                int &v1=GInv[i][i1];
                if(v1<=i) break;
                reachable[tc][v1]=true;  //正向一层可到达
                reachableInv[tc][v1]=true;
                for(int i2=0;i2<GInv[v1].size();i2++){  //反序遍历第2层
                    int &v2=GInv[v1][i2];
                    if(v2<=i) break;
                    reachableInv[tc][v2]=true;
                    for(int i3=0;i3<GInv[v2].size();i3++){  //反序遍历第3层
                        int &v3=GInv[v2][i3];
                        if(v3<=i) break;
                        reachableInv[tc][v3]=true;
                    }
                }
            }
            //正向4层dfs遍历
            out[++out[0]]=i;
            visit[tc][i]=true;
            for(int i1=0;i1<G[i].size();i1++){  //查找，out的第0个元素为环的长度，第1个元素为始节点，从第2个开始的节点倒序存储  
                int &u2=G[i][i1];  //环中的第2个点
                if(u2<i) break;
                out[++out[0]]=u2;
                visit[tc][u2]=true;
                for(int i2=0;i2<G[u2].size();i2++){
                    int &u3=G[u2][i2];  //环中的第3个点
                    if(u3<=i) break;
                    out[++out[0]]=u3;
                    visit[tc][u3]=true;
                    if(reachable[tc][u3]){  //检测到长度为3的环
                        vector<uint> tmp(out[0]);
                        for(int i=1;i<=out[0];i++)
                            tmp[i-1]=ids[out[i]];
                        if(checkAns(tmp,out[0])){    //check需要实际的id
                            results[tc][out[0]].emplace_back(tmp);
                            ++ringCnt[tc];
                        }
                    }
                    for(int i3=0;i3<G[u3].size();i3++){
                        int &u4=G[u3][i3];  //环中的第4个点
                        if(u4<i) break;
                        if(visit[tc][u4]) continue;
                        out[++out[0]]=u4;
                        visit[tc][u4]=true;
                        if(reachable[tc][u4]){  //检测到长度为4的环
                            vector<uint> tmp(out[0]);
                            for(int i=1;i<=out[0];i++)
                                tmp[i-1]=ids[out[i]];
                            if(checkAns(tmp,out[0])){
                                results[tc][out[0]].emplace_back(tmp);
                                ++ringCnt[tc];
                            }
                        }
                        for(int i4=0;i4<G[u4].size();i4++){
                            int &u5=G[u4][i4];  //环中的第5个点
                            if(u5<i) break;
                            if(visit[tc][u5]||(!reachableInv[tc][u5])) continue;
                            out[++out[0]]=u5;
                            visit[tc][u5]=true;
                            if(reachable[tc][u5]){  //检测到长度为5的环
                                vector<uint> tmp(out[0]);
                                for(int i=1;i<=out[0];i++)
                                    tmp[i-1]=ids[out[i]];
                                if(checkAns(tmp,out[0])){
                                    results[tc][out[0]].emplace_back(tmp);
                                    ++ringCnt[tc];
                                }
                            }
                            for(int i5=0;i5<G[u5].size();i5++){
                                int &u6=G[u5][i5];  //环中的第6个点
                                if(u6<i) break;
                                if(visit[tc][u6]||(!reachableInv[tc][u6])) continue;
                                out[++out[0]]=u6;
                                visit[tc][u6]=true;
                                if(reachable[tc][u6]){  //检测到长度为6的环
                                    vector<uint> tmp(out[0]);
                                    for(int i=1;i<=out[0];i++)
                                        tmp[i-1]=ids[out[i]];
                                    if(checkAns(tmp,out[0])){
                                        results[tc][out[0]].emplace_back(tmp);
                                        ++ringCnt[tc];
                                    }
                                }
                                for(int i6=0;i6<G[u6].size();i6++){
                                    int &u7=G[u6][i6];  //环中的第7个点
                                    if(u7<i) break;
                                    if(visit[tc][u7]||(!reachableInv[tc][u7])) continue;
                                    out[++out[0]]=u7;
                                    visit[tc][u7]=true;
                                    if(reachable[tc][u7]){  //检测到长度为7的环
                                        vector<uint> tmp(out[0]);
                                        for(int i=1;i<=out[0];i++)
                                            tmp[i-1]=ids[out[i]];
                                        if(checkAns(tmp,out[0])){
                                            results[tc][out[0]].emplace_back(tmp);
                                            ++ringCnt[tc];
                                        }
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

            //将反序表遍历的标记清除
            for(int i1=0;i1<GInv[i].size();i1++){
                int &v1=GInv[i][i1];
                if(v1<=i) break;
                reachable[tc][v1]=false;  //正向一层可到达
                reachableInv[tc][v1]=false;
                for(int i2=0;i2<GInv[v1].size();i2++){  //反序遍历第2层
                    int &v2=GInv[v1][i2];
                    if(v2<=i) break;
                    reachableInv[tc][v2]=false;
                    for(int i3=0;i3<GInv[v2].size();i3++){  //反序遍历第3层
                        int &v3=GInv[v2][i3];
                        if(v3<=i) break;
                        reachableInv[tc][v3]=false;
                    }
                }
            }
        }
    }
    //resultSize[tc][l][i] 获取每个长度下每个id开头的所有相邻环的字节数
    int len=0;
    for(int l=lowDepth;l<=highDepth;l++)
    {
        int index=results[tc][l].size()-1;
        for(int i=tc;i<=nodeCnt-nodeCnt%threadNum+tc;i+=threadNum)
        {
            len=0;
            while(index>=0&&results[tc][l][index][0]==i)
            {
                for(int n=0;n<l;n++)
                {
                    len+=strSize[results[tc][l][index][n]];
                }
                --index;
            }
            resultSize[tc][l][i]=len;
        }
    }
#ifdef TEST
    cout<<"thread"<<tc<<" "<<ringCnt[tc]<<endl;
#endif
}

void solve()
{
    //多线程dfs
#ifdef TEST
    cout<<"dfs..."<<endl;
#endif

    for(int i=0;i<threadNum;i++){
        infos[i].tc=i;
        pthread_create(&tids[i], NULL, run, (void *)&(infos[i]));
    }
    for(int i=0;i<threadNum;i++){
        pthread_join(tids[i], NULL);
    }

#ifdef TEST
    cout<<"done!"<<endl;
    //cout<<"total rings: "<<totalRings<<endl;
#endif
}

void saveData()
{
    //多线程mmap分块写数据
    int totalRings=0;
    for(int tc=0;tc<threadNum;tc++){
        totalRings+=ringCnt[tc];
    }
#ifdef TEST
    cout<<"saving data..."<<endl;
    cout<<"total rings: "<<totalRings<<endl;
#endif
    // FILE *fp = fopen(resultFile.c_str(), "wb");
    // char buf[1024];
    // int idx=sprintf(buf,"%d\n",totalRings);
    // buf[idx]='\0';
    // fwrite(buf, idx , sizeof(char), fp );
    // for(int i=lowDepth;i<=highDepth;i++){
    //     //cout<<i;
    //     for (auto &result:results[i]) {
    //         int sz=result.size();
    //         for(int j=0;j<sz-1;j++){
    //             auto &res=idCom[idHash[result[j]]];
    //             fwrite(res.c_str(),res.size(),sizeof(char),fp);
    //         }
    //         auto &res=idLF[idHash[result[sz-1]]];
    //         fwrite(res.c_str(),res.size(),sizeof(char),fp);
    //     }
    // }
    // fclose(fp);

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
    //simplifyAndSort();
    constructGraph();
    solve();
    saveData();

    return 0;
}