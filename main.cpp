#include "bits/stdc++.h"
#include <sys/mman.h>  // mmap()
#include <fcntl.h>     // open()
#include <unistd.h>    // lseek()

using namespace std;

//在官网上提交作品时一定一定要把下面这句注释掉，即转换文件读取路径
#define TEST

#define DEPTH_LOW 3
#define DEPTH_HIGH 7


//类名：Solution
//作用：评估金融账号是否存在循环转账
//数据特征：28w条记录，按平均转账10次算大概2.8w账户，输出大概300w的环，会有大量的重复边

class Solution{
public:
    Solution(string testFile, string resultFile);
    void loadData();
    void constructGraph();
    void simplifyAndSort(vector<int> &deg,bool sorting);
    void constructReverseIndex();
    //void dfs();                     //迭代形式的dfs
    void solve();
    void solveDFS(int head,int cur,int depth,vector<int> &out);
    void sortResult();
    void saveData();


private:
    string testFile;
    string resultFile;
    //unordered_map<int, vector<int>> dataMap;  //邻接表,存储每个账户所有的转出账户，隐藏属性：出度为0的节点没有计入，入度为0的点其对应的vector的size()为0
    //unordered_map<int, bool> visitMap;         //已访问矩阵，用于dfs的访问记录
    vector<vector<int>> results[8];  //按环的长度从3到7储存结果
    int nodeCnt=0;   //记录节点数
    int ringCnt=0;   //记录总环数

    vector<vector<int>> G;          //邻接矩阵
    unordered_map<int,int> idHash;  //sorted id to 0...n
    vector<string> idsCom; //0...n to sorted id
    vector<string> idsLF; //0...n to sorted id
    vector<int> inputs; //u-v pairs
    vector<bool> visit;
    vector<int> reachable;

    vector<unordered_map<int,vector<int>>> R;

public:
    vector<int> inDegrees;
    vector<int> outDegrees;

};

Solution::Solution(string testF, string resultF)
{
    testFile=testF;
    resultFile=resultF;
}

void Solution::loadData()
{
    int fd = open(testFile.c_str(), O_RDONLY);
    //可以根据文件大小设置是否启动多线程，1个线程/4个以上线程
    int len = lseek(fd,0,SEEK_END);
    //cout<<"file length: "<<len<<endl;
    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    close(fd);
    //cout<<mbuf[10]<<"."<<mbuf[11]<<mbuf[12]<<"."<<mbuf[13]<<endl;  //第11个是'\r',第12个是'\n'
    int id1=0,id2=0;
    cout<<"loading data ..."<<endl;
    for(char *p=mbuf;*p&&p-mbuf<len;p++)
    {
        while(*p!=',')
        {
            id1=id1*10+(*p-'0');
            p++;
        }
        while(*(++p)!=',') id2=id2*10+(*p-'0');
        //dataMap[id1].push_back(id2);
        //visitMap[id1]=false;
        inputs.push_back(id1);
        inputs.push_back(id2);
        //cout<<id1<<" ";
        id1=0;id2=0;
        while(*(++p)!='\n');
        //p++;
    }
    munmap(mbuf,len);
    cout<<"done!"<<endl;
}

void Solution::constructGraph()
{
        auto tmp=inputs;
        sort(tmp.begin(),tmp.end());
        tmp.erase(unique(tmp.begin(),tmp.end()),tmp.end());
        nodeCnt=tmp.size();
        idsCom.reserve(nodeCnt);
        idsLF.reserve(nodeCnt);
        nodeCnt=0;
        nodeCnt=0;
        for(int &x:tmp){
            idsCom.push_back(to_string(x)+',');
            idsLF.push_back(to_string(x)+'\n');
            idHash[x]=nodeCnt++;
        }

        int sz=inputs.size();
        G=vector<vector<int>>(nodeCnt);
        inDegrees=vector<int>(nodeCnt,0);
        outDegrees=vector<int>(nodeCnt,0);
        for(int i=0;i<sz;i+=2){
            int u=idHash[inputs[i]],v=idHash[inputs[i+1]];
            G[u].push_back(v);
            ++inDegrees[v];
            ++outDegrees[u];
        }
}

void Solution::simplifyAndSort(vector<int> &deg,bool sorting)
{
    queue<int> q;
    for(int i=0;i<nodeCnt;i++){
        if(deg[i]==0)
            q.push(i);
    }
    while(!q.empty()){
        int u=q.front(); q.pop();
        for(int &v:G[u]) {
            if(--deg[v]==0)  //遍历入度为0的节点的邻接表并删除入度为1的节点的邻接表
                q.push(v);
        }
    }

    int cnt=0;
    for(int i=0;i<nodeCnt;i++){
        if(deg[i]==0){
            G[i].clear();
            cnt++;
        }else if(sorting){
            sort(G[i].begin(),G[i].end());
        }
    }
#ifdef TEST
        cout<<cnt<<" nodes eliminated"<<endl;
#endif
}


void Solution::constructReverseIndex()
{
    //vector<unordered_map<int,vector<int>>> R;
    //使用R[j][i][k]来表示结点i到达结点j，中间经过结点k的路径详情，如果k不在我们已经搜索过的结点列表中，并且i是起点，那么j-k-i就是符合要求的路径的一部分。
    //具体来说，就是提前做深度为2的搜索（保存最后一层结点的入边），在第6层时直接根据现有结果进行判断，不进入第七层。
    R.resize(nodeCnt);
    for(int i=0;i<nodeCnt;i++){
            auto &vec=G[i];
            for(int &k:vec){//对i的所有邻接点k进行遍历 
                auto &veck=G[k];
                for(int &j:veck){//对k的所有邻接点j进行遍历
                    if(j!=i) R[j][i].push_back(k);//节点i能够通过节点k访问到节点j
                }
            }
        }
    for(int i=0;i<nodeCnt;i++){
        for(auto &x:R[i]){
            if(x.second.size()>1){
                sort(x.second.begin(),x.second.end());
            }
        }
    }
}

void Solution::solveDFS(int head,int cur,int depth,vector<int> &out)
{
    //递归形式的dfs
    visit[cur]=true;
    out.push_back(cur);
    auto &vCur=G[cur];
    auto it=lower_bound(vCur.begin(),vCur.end(),head);
    //handle [3,6]
    if(it!=vCur.end() && *it==head && depth>=DEPTH_LOW && depth<DEPTH_HIGH) {
        results[depth].emplace_back(out);
        ++ringCnt;
    }

    if(depth<DEPTH_HIGH-1){
        for(;it!=vCur.end();++it){
            if(!visit[*it]){
                solveDFS(head,*it,depth+1,out);
            }
        }
    }else if(reachable[cur]>-1 && depth==DEPTH_HIGH-1){ //handle [7]
        auto ks=R[head][cur];
        int sz=ks.size();
        for(int idx=reachable[cur];idx<sz;++idx){
            int k=ks[idx];
            if(visit[k]) continue;
            auto tmp=out;
            tmp.push_back(k);
            results[depth+1].emplace_back(tmp);
            ++ringCnt;
        }
    }
    visit[cur]=false;
    out.pop_back();

    /*
    visit[cur]=true;
    out.push_back(cur);
    for(int &v:G[cur]){ //depth=6时，cur是第6层的一个点，判断这个节点的邻接表中的每个元素v是否有等于head的节点，相等则放入结果中，否则对该节点遍历下一层
        if(v==head && depth>=3 && depth<=7){  //此条件在cur的邻接表中最多触发一次
            vector<int> tmp;
            for(int &x:out)
                tmp.push_back(ids[x]);
            results[depth].emplace_back(tmp);
        }
        if(depth<7 && !visit[v] && v>head){
            solveDFS(head,v,depth+1,out);
        }
    }
    //反向索引直接得出第7层结果，跳过第7次dfs
    //优化效果不太好，若注释的话记得将上面的6改为7
    if(depth==6 && !R[cur][head].empty()){
        vector<int> tmp;
        for(int &x:out)
            tmp.push_back(ids[x]);
        for(int& k:R[cur][head]){
            //cout<<"第7层";
            if(visit[k] || k<head) continue;
            tmp.push_back(ids[k]);
            results[depth+1].emplace_back(tmp);
            tmp.pop_back();
        }
    }
    
    visit[cur]=false;
    out.pop_back();
    */
/*
    visitMap[cur]=true;
    out.push_back(cur);
    for(int& v:dataMap[cur]){
        if(v==head && depth>=3 && depth<=7){
            results[depth].emplace_back(out);
        }
        if(depth<7 && !visitMap[v] && v>head){
            solveDFS(head,v,depth+1,out);
        }
    }
    visitMap[cur]=false;
    out.pop_back();
*/
}


void Solution::solve()
{
    //solveDFS的外循环
    ringCnt=0;
    visit=vector<bool>(nodeCnt,false);
    vector<int> out;
    reachable=vector<int>(nodeCnt,-1);
    vector<int> currentJs(nodeCnt);
    for(int i=0;i<nodeCnt;++i){
#ifdef TEST
        if(i%100==0) cout<<i<<"/"<<nodeCnt<<endl;
#endif
        if(!G[i].empty()){
            //可以通过大于head的id返回的
            for(auto &js:R[i]){
                int j=js.first;
                if(j>i){
                    auto &val=js.second;
                    int sz=val.size();
                    int lb=lower_bound(val.begin(),val.end(),i)-val.begin();
                    if(lb<val.size()) reachable[j]=lb;
                    currentJs.push_back(j);  //存储大于i的j的数值
                }
            }
            solveDFS(i,i,1,out);
            for(int &x:currentJs)
                reachable[x]=-1;
            currentJs.clear();
        }
    }
#ifdef TEST
    cout<<"total rings: "<<ringCnt<<endl;
#endif
/*
    vector<int> out;
    int cnt=0;  //dfs进度
    for(auto& elem:dataMap){
        cnt++;
        if(cnt%100==0) cout<<cnt<<"/"<<nodeCnt<<endl;
        solveDFS(elem.first,elem.first,1,out);
    }
*/
}

/*
//禁忌的七重舞法天女
void Solution::dfs()
{
    cout<<"search ..."<<endl;
    for(auto &elem:dataMap)
    {
        vector<int> out;
        int firstElem=elem.first;
        out.emplace_back(firstElem);
        visitMap[firstElem]=true;
        for(int i2=0;i2<elem.second.size();i2++)
        {
            int temp2=elem.second[i2];
            if(temp2<=firstElem) continue;  //判断要遍历的账户id是否小于最开始的id，保证结果的首元素值最小
            out.emplace_back(temp2);
            visitMap[temp2]=true;
            for(int i3=0;i3<dataMap[temp2].size();i3++)
            {
                int temp3=dataMap[temp2][i3];
                if(temp3<=firstElem||visitMap[temp3]) continue;  //确保要遍历的第3层元素值大于首元素且之前未被访问过
                out.emplace_back(temp3);
                visitMap[temp3]=true;
                for(int i4=0;i4<dataMap[temp3].size();i4++)  //当长度为4时（包括首元素），需要检查第4个元素是否为首元素，以检测环的存在
                {
                    int temp4=dataMap[temp3][i4];
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
                        int temp5=dataMap[temp4][i5];
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
                            int temp6=dataMap[temp5][i6];
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
                                int temp7=dataMap[temp6][i7];
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
                                    int temp8=dataMap[temp7][i8];
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
*/

void Solution::sortResult()
{
    //继续优化：多线程分别运行sort()，或者直接用set存储results[3]
    cout<<"sorting result ..."<<endl;
    for(int l=3;l<=7;l++)
    {
        ringCnt+=results[l].size();
        sort(results[l].begin(),results[l].end());
    }
    cout<<"done!"<<endl;
    cout<<"总环数： "<<ringCnt<<endl;
    //for(auto& result:results[3])
    //    cout<<result[0]<<" ";
}

void Solution::saveData()
{
    //fwrite或者mmap写
    cout<<"saving data ..."<<endl;
    // for(int i=DEPTH_LOW_LIMIT;i<=DEPTH_HIGH_LIMIT;i++) {
    //         //sort(ans[i].begin(),ans[i].end());
    //         for (auto &x:ans[i]) {
    //             auto path = x.path;
    //             int sz = path.size();
    //             //idx=0;
    //             for(int j=0;j<sz-1;j++){
    //                 auto res=idsComma[path[j]];
    //                 fwrite(res.c_str(), res.size() , sizeof(char), fp );
    //             }
    //             auto res=idsLF[path[sz-1]];
    //             fwrite(res.c_str(), res.size() , sizeof(char), fp );
    //         }
    // }
    //结果转成字符串
    string resultStr(to_string(ringCnt)+"\n");
    //ofstream output(resultFile);
    //output<<ringCnt<<endl;
    for(int l=DEPTH_LOW;l<=DEPTH_HIGH;l++)
    {
        sort(results[l].begin(),results[l].end());
        for(auto& result:results[l]){
            int sz=result.size();
            for(int j=0;j<sz-1;j++){
                auto res=idsCom[result[j]];
                resultStr+=res;
            }
            auto res=idsLF[result[sz-1]];
            resultStr+=res;
        }
    }
    //mmap写到文件
    int fd = open(resultFile.c_str(),O_RDWR|O_CREAT,0666);
    int len=resultStr.length();
    ftruncate(fd,len);
    int strLen=1024*1024;
    char* pbuf=NULL;
    int cnt=len/strLen;
    int mod=len%strLen;
    //cout<<cnt<<" "<<mod<<endl;
    for(int i=0;i<cnt;i++){
        pbuf=(char *)mmap(NULL,strLen,PROT_READ|PROT_WRITE,MAP_SHARED,fd,i*strLen);
        memcpy(pbuf,resultStr.substr(i*strLen,strLen).c_str(),strLen);
        munmap(pbuf,strLen);
    }
    pbuf=(char*) mmap(NULL,mod,PROT_WRITE,MAP_SHARED,fd,cnt*strLen);
    memcpy(pbuf,resultStr.substr(cnt*strLen,mod).c_str(),mod);
    munmap(pbuf,mod);

    close(fd);

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
    solution.constructGraph();
    solution.simplifyAndSort(solution.inDegrees,false);
    solution.simplifyAndSort(solution.outDegrees,true);
    solution.constructReverseIndex();
    //solution.dfs();
    solution.solve();

    //solution.sortResult();
    solution.saveData();

    return 0;
}
