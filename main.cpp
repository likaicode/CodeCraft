#include "bits/stdc++.h"
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

using namespace std;

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
#define TEST

#define DEPTH_LOW 3
#define DEPTH_HIGH 7
#define THREAD_NUM 8


//类名：Solution
//作用：评估金融账号是否存在循环转账
//数据特征：28w条记录，按平均转账10次算大概2.8w账户，输出大概300w的环，会有大量的重复边

string testFile;
string resultFile;
//vector<vector<int>> results[8];  //按环的长度从3到7储存结果
//int nodeCnt=0;   //记录节点数
int maxId=0;      //记录节点最大序号
int ringCnt=0;   //记录总环数

//vector<vector<int>> G;          //邻接矩阵
//unordered_map<int,int> idHash;  //sorted id to 0...n
//vector<string> idsCom; //0...n to sorted id
//vector<string> idsLF; //0...n to sorted id
//vector<int> inputs; //u-v pairs
//vector<bool> visit;
//vector<bool> reachable;

//vector<unordered_map<int,vector<int>>> R;

int inputs[560001];
int G[280000][50];    //G[u][v]
int GInv[280000][50]; //G[v][u]
string idCom[280000];
string idLF[280000];
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

// void loadData();

// void constructGraph();
// void quickSort(int arr[],int low,int high);
// void simplifyAndSort(int deg[],bool sorting);
// void constructReverseIndex();

// void solve();
// int binarySearch(int arr[], int target);
// void solveDFS(int head,int cur,int depth,int out[]);
// void saveData();

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
#ifdef TEST
    cout<<"loading data ..."<<endl;
#endif
    for(char *p=mbuf;p-mbuf<len;p++)
    {
        while(*p!=',')
        {
            id1=id1*10+(*p-'0');
            p++;
        }
        while(*(++p)!=',') id2=id2*10+(*p-'0');
        if(maxId<id1) maxId=id1;
        if(maxId<id2) maxId=id2;
        //maxId=max(maxId,max(id1,id2));
        G[id1][++G[id1][0]]=id2;
        GInv[id2][++GInv[id2][0]]=id1;
        if(idCom[id1].empty()){
            idCom[id1]=(to_string(id1)+',');
            idLF[id1]=(to_string(id1)+'\n');
        }
        if(idCom[id2].empty()){
            idCom[id2]=(to_string(id2)+',');
            idLF[id2]=(to_string(id2)+'\n');
        }
        //++inDegrees[id2];
        //++outDegrees[id1];
        //inputs[++inputs[0]]=id1;
        //inputs[++inputs[0]]=id2;
        //cout<<id1<<" ";
        id1=0;id2=0;
        //p=(char*)memchr(p, '\n', 1000);
        while(*(++p)!='\n');
    }
    munmap(mbuf,len);
#ifdef TEST
    cout<<"done!"<<endl;
#endif
}

/*
void constructGraph()
{
        int n=inputs[0];
        //int *tmp=new int[n];
        //memcpy(tmp,inputs+1,n*sizeof(int));
        //sort(tmp,tmp+n);
        //unique去重，返回一个迭代器，指向不重复序列的最后一个元素的下一个元素
        //nodeCnt=unique(tmp,tmp+n)-tmp;
        //idCom=new string[nodeCnt];
        //idLF=new string[nodeCnt];
        // visit=new bool[nodeCnt];
        // reachable=new bool[nodeCnt];
        // inDegrees=new int[nodeCnt];
        // outDegrees=new int[nodeCnt];

        for(int i=0;i<nodeCnt;i++){
            idCom[i]=(to_string(tmp[i])+',');
            idLF[i]=(to_string(tmp[i])+'\n');
            idHash[tmp[i]]=i;
        }
        //int n=inputs.size();
        for(int i=1;i<=n;i+=2){
            int u=idHash[inputs[i]],v=idHash[inputs[i+1]];
            G[u][++G[u][0]]=v;
            ++inDegrees[v];
            ++outDegrees[u];
        }
}
*/

/*
void quickSort(int arr[],int low,int high){
    int start=low;
    int end=high;
    int key=arr[low];
    while(start<end){
        //从后往前开始比较
        while(end>start && arr[end]>=key)//如果没有比关键值小的，比较下一个，直到有比关键值小的交换位置，然后又从前往后比较
            end--;
        if(arr[end]<=key){
            int temp=arr[end];
            arr[end]=arr[start];
            arr[start]=temp;
        }
        //从前往后比较
        while(end>start && arr[start]<=key)//如果没有比关键值大的，比较下一个，直到有比关键值大的交换位置，然后又从后往前比较
            start++;
        if(arr[start]>=key){
            int temp=arr[start];
            arr[start]=arr[end];
            arr[end]=temp;	
        }
    //此时第一次循环比较结束，关键值的位置已经确定了。左边的值都比关键值小，右边的值都比关键值大，但是两边的顺序还有可能是不一样的，进行下面的递归调用
    }
    //递归
    if(start>low) 
        quickSort(arr,low,start-1);
    if(start<high) 
        quickSort(arr,end+1,high);
            
}
*/

void simplifyAndSort()
{
    //q[0]=2,q[1]=2;
    //queue<int> q;
    for(int i=0;i<=maxId;i++){
        if(G[i][0]&&GInv[i][0]){
            sort(G[i]+1,G[i]+G[i][0]+1);
            continue;
        }
        if(G[i][0]) G[i][0]=0;
        if(GInv[i][0]) GInv[i][0]=0;
            //q[q[1]++]=i;
            //q.push(i);
    }
}
    // while(q[0]<q[1]){
    //     //int u=q.front(); q.pop();
    //     int u=q[q[0]++];
    //     int n=G[u][0];
    //     for(int i=1;i<=n;i++) {
    //         if(--deg[G[u][i]]==0)  //遍历入度为0的节点的邻接表并删除入度为1的节点的邻接表
    //             q[q[1]++]=G[u][i];
    //             //q.push(G[u][i]);
    //     }
    // }

// #ifdef TEST
//     int cnt=0;
// #endif
//     for(int i=0;i<nodeCnt;i++){
//         if(deg[i]==0){
//             G[i][0]=0;
// #ifdef TEST
//             cnt++;
// #endif
//         }else if(sorting){
//             sort(G[i]+1,G[i]+G[i][0]+1);
//             //quickSort(G[i],1,G[i][0]);
//         }
//     }
// #ifdef TEST
//         cout<<cnt<<" nodes eliminated"<<endl;
// #endif

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
int binarySearch(int arr[], int target)
{
    int start = 1, end = arr[0];
    if(arr[end]<target) return 0;
	while (start <= end) {
		int mid = (start + end) / 2;
		if (arr[mid] < target)
			start = mid + 1;
		else if (arr[mid] >= target)
			end = mid - 1;
	}
    return end + 1;
}
*/


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


void solve()
{
    //solveDFS的外循环，可以用多线程来并行优化
    //if(nodeCnt>20000) openThread=true;    //当图的节点比较多时，才会采用多线程dfs
    //多线程dfs方法：将nodeCnt个节点分为threadNum块，开启threadNum个线程并在每个线程中搜索指定范围内的节点是否成环，G和R是线程共享的
    //每个线程访问自己的reachable和currentJs

    for(int i=0;i<=maxId;i++){
        if(G[i][0]){
            int &m1=GInv[i][0];
            for(int i1=1;i1<=m1;i1++){  //反序遍历第1层
                int &v1=GInv[i][i1];
                reachable[v1]=true;
                current[++current[0]]=v1;
                int &m2=GInv[v1][0];
                for(int i2=1;i2<=m2;i2++){  //反序遍历第2层
                    int &v2=GInv[v1][i2];
                    reachable[v2]=true;
                    current[++current[0]]=v2;
                    int &m3=GInv[v2][0];
                    for(int i3=1;i3<=m3;i3++){  //反序遍历第3层
                        int &v3=GInv[v2][i3];
                        reachable[v3]=true;
                        current[++current[0]]=v3;
                    }
                }
            }
            //正向4层dfs遍历
            out[++out[0]]=i;
            visit[i]=true;
            int &n1=G[i][0];
            for(int i1=1;i1<=n1;i1++){
                int &u1=G[i][i1];
                
            }


            //将反序表遍历的标记清除
            int m=current[0];
            for(int i=1;i<=m;i++){
                reachable[current[i]]=false;
            }
            current[0]=0;
        }
    }



/*
    for(int i=0;i<nodeCnt;++i){
#ifdef TEST
        if(i%100==0) cout<<i<<"/"<<nodeCnt<<endl;
#endif
        if(G[i][0]){
            //可以通过大于head的id返回的
            for(auto &js:R[i]){
                int j=js.first;
                reachable[j]=true;
                currentJs[++currentJs[0]]=j;
                // int j=js.first;
                // if(j>i){
                //     auto &val=js.second;
                //     int sz=val.size();
                //     int lb=lower_bound(val.begin(),val.end(),i)-val.begin();
                //     if(lb<val.size()) reachable[j]=lb;
                //     currentJs.push_back(j);  //存储大于i的j的数值
                // }
            }
            solveDFS(i,i,1,out);
            int currentJsSize=currentJs[0];
            for(int j=1;j<=currentJsSize;j++)
                reachable[currentJs[j]]=false;
            currentJs[0]=0;
        }
    }
*/
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
    string resultStr(to_string(ringCnt)+'\n');
    resultStr.reserve(100000);
    //resultStr+=to_string(ringCnt);
    //resultStr+="\n";
    for(int l=DEPTH_LOW;l<=DEPTH_HIGH;l++)
    {
        //sort(results[l].begin(),results[l].end());
        results[l][0]+=l;
        int n=results[l][0];
        for(int i=l;i<n;i++){
            for(int j=1;j<l;j++){
                resultStr+=idCom[results[l][i]];
                i++;
            }
            resultStr+=idLF[results[l][i]];
        }
    }
    //mmap写到文件
    int fd = open(resultFile.c_str(),O_RDWR|O_CREAT,0666);
    int len=resultStr.length();
    lseek(fd, len - 1, SEEK_SET);
    write(fd, " ", 1);
    //ftruncate(fd,len);
    int strLen=1024*1024;
    char* pbuf=NULL;
    int cnt=len/strLen;
    int mod=len%strLen;
#ifdef TEST
    cout<<cnt<<"MB+"<<mod<<endl;
#endif
    for(int i=0;i<cnt;i++){
        pbuf=(char *)mmap(NULL,strLen,PROT_READ|PROT_WRITE,MAP_SHARED,fd,i*strLen);
        memcpy(pbuf,resultStr.substr(i*strLen,strLen).c_str(),strLen);
        munmap(pbuf,strLen);
    }
    pbuf=(char*) mmap(NULL,mod,PROT_READ|PROT_WRITE,MAP_SHARED,fd,cnt*strLen);
    memcpy(pbuf,resultStr.substr(cnt*strLen,mod).c_str(),mod);
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
    testFile="./data/test_data.txt";
    resultFile="./projects/student/result.txt";
#endif
    //Solution solution(testFile,resultFile);
    loadData();

    constructGraph();
    simplifyAndSort();
    //simplifyAndSort(inDegrees,false);
    //simplifyAndSort(outDegrees,true);

    //constructReverseIndex();

    solve();

    saveData();

    return 0;
}
