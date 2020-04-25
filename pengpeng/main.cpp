#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<pthread.h>
#include<fcntl.h>
#include<vector>
#include<queue>
#include<string>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<algorithm>
#include<numeric>
#include<time.h>

using namespace std;

//#define TEST
//#define TIME
#define MAX_DEPTH 7
#define MIN_DEPTH 3
#define MAX_VERTEX 560000
#define MAX_RECORD 280000
#define MAX_DEGREE 50
#define THREADNUM 10
#define NUM1 200000
#define NUM2 200000
#define NUM3 400000
#define NUM4 800000
#define NUM5 1000000
typedef unsigned int uint;

uint IdVec[MAX_VERTEX+1];
int Graph[MAX_RECORD][MAX_DEGREE];
int GraphInv[MAX_RECORD][MAX_DEGREE];
thread_local int Path[MAX_DEPTH];
thread_local int NodeVec[MAX_DEGREE*MAX_DEGREE];
int InDegree[MAX_VERTEX];
int OutDegree[MAX_VERTEX];
thread_local bool Visited[MAX_VERTEX];
thread_local bool Reach[MAX_VERTEX];
thread_local bool InvReach[MAX_VERTEX];
string IdWithComma[MAX_VERTEX];
string IdWithLF[MAX_VERTEX];
int ans3[THREADNUM * 3 * NUM1];
int ans4[THREADNUM * 4 * NUM2];
int ans5[THREADNUM * 5 * NUM3];
int ans6[THREADNUM * 6 * NUM4];
int ans7[THREADNUM * 7 * NUM5];
int circleNum[5]={NUM1,NUM2,NUM3,NUM4,NUM5};
int *ans[] = { ans3,ans4,ans5,ans6,ans7};
int ansSize[THREADNUM][5];
int CircleCnt;
int Maxid;
thread_local int AnsCnt;
thread_local int Top;
char pData[300000000];



struct PthreadData{
	int fd;
	int size;
	int start;
	int end;
};
typedef struct PthreadData PTD;


void ReadData(const string &inputfile)
{
#ifdef TIME
	clock_t start=clock();
#endif
	int fd;
	char *p=NULL;
	//打开文件
	fd=open(inputfile.c_str(),O_RDONLY);
	//获取文件大小
	int len=lseek(fd,0,SEEK_END);
	//创建共享映射区
	p=(char*)mmap(NULL,len,PROT_READ,MAP_SHARED,fd,0);
	char *q=p;
	char *head;
	char str[16];
	int cnt = 0;
	int i=0;
	while(*q!='\0')
	{
		head=q;
		while(*q!='\n')
		{
			if(*q==',')
			{
				memset(str,0,sizeof(str));
				strncpy(str,head,q-head);
				uint id=stoul(str);
				IdVec[++i]=id;
                if(Maxid<id) Maxid=id;
				head=q+1;
			}
			q++;
		}
		q++;
		++cnt;
	}
    //cout<<"maxid:"<<Maxid<<endl;
	IdVec[0]=i;
	munmap(p,len);
	close(fd);
#ifdef TIME
	clock_t end=clock();
	cout<<"ReadData time="<<(double)(end-start)/1000<<"ms"<<endl;
#endif
#ifdef TEST
	printf("%d Records In Total\n", cnt);
#endif
}

void ConstructGraph() 
{
#ifdef TIME
	clock_t start=clock();
#endif
	for (int i = 0; i <= Maxid; ++i)
	{
		IdWithComma[i] = to_string(i)+',';
		IdWithLF[i] = to_string(i)+'\n';
	}
	int size = IdVec[0];
	int u, v;
	for (int i = 1; i < size; i += 2)
	{
		u = IdVec[i];
		v = IdVec[i + 1];
		Graph[u][OutDegree[u]+1] = v;
        GraphInv[v][InDegree[v]+1]=u;
		Graph[u][0]++;
        GraphInv[v][0]++;
		++InDegree[v];
		++OutDegree[u];
	}
#ifdef TIME
	clock_t end=clock();
	cout<<"Construct Graph time="<<(double)(end-start)/1000<<"ms"<<endl;
#endif
}

void TopoSort()
{
#ifdef TIME
	clock_t start=clock();
#endif
	for (int i = 0; i <= Maxid; ++i)
	{
		if (InDegree[i] == 0)//删去所有入度为0并且有邻接点的节点
		{
			Graph[i][0]=0;
		}
	}

	for (int i = 0; i <=Maxid; ++i)
	{
		if (OutDegree[i] == 0)
		{
			Graph[i][0]=0;
		}
		else
		{
			sort(Graph[i]+1,Graph[i] +1 + Graph[i][0],greater<int>());
		}
	}
#ifdef TIME
	clock_t end=clock();
	cout<<"Toposort time="<<(double)(end-start)/1000<<"ms"<<endl;
#endif
}

void ForRecur(int root,int threadid)
{ 
    Visited[root]=true;
    Path[Top++]=root;
    for(auto it1=Graph[root]+1;*it1>root;++it1)//在第一层遍历第二层
    {
        int v2=*it1;
        if(Visited[v2]) continue;
        Visited[v2]=true;
        Path[Top++]=v2;//root的邻接点  第二层
        for(auto it2=Graph[v2]+1;*it2>root;++it2)//在第二层遍历第三层
        {
            int v3=*it2;
            if(Visited[v3]) continue;
            Visited[v3]=true;
            Path[Top++]=v3;
            if(Reach[v3])
            {
                int size=ansSize[threadid][0]+threadid*circleNum[0]*3;
                for(int i=0;i<Top;i++)
                {
                    ans[0][size+i]=Path[i];
                }
                ansSize[threadid][0]+=3;
                ++AnsCnt;
            }
            for(auto it3=Graph[v3]+1;*it3>root;++it3)//在第三层遍历第四层
            {
                int v4=*it3;
                if(Visited[v4]) continue;
                Visited[v4]=true;
                Path[Top++]=v4;
                if(Reach[v4])
                {
                    int size=ansSize[threadid][1]+threadid*circleNum[1]*4;
                    for(int i=0;i<Top;i++)
                    {
                        ans[1][size+i]=Path[i];
                    }
                    ansSize[threadid][1]+=4;
                    ++AnsCnt;
                }

                for (auto it4=Graph[v4]+1;*it4>root;++it4)//在第四层遍历第五层
                {
                    int v5=*it4;
                    if(!InvReach[v5]||Visited[v5]) continue;
                    Visited[v5]=true;
                    Path[Top++]=v5;
                    if(Reach[v5])
                    {
                        int size=ansSize[threadid][2]+threadid*circleNum[2]*5;
                        for(int i=0;i<Top;i++)
                        {
                            ans[2][size+i]=Path[i];
                        }
                        ansSize[threadid][2]+=5;
                        ++AnsCnt;
                    }
                    for(auto it5=Graph[v5]+1;*it5>root;++it5)//在第五层遍历第六层
                    {
                        int v6=*it5;
                        if(!InvReach[v6]||Visited[v6]) continue;
                        Visited[v6]=true;
                        Path[Top++]=v6;
                        if(Reach[v6])
                        {
                            int size=ansSize[threadid][3]+threadid*circleNum[3]*6;
                            for(int i=0;i<Top;i++)
                            {
                                ans[3][size+i]=Path[i];
                            }
                            ansSize[threadid][3]+=6;
                            ++AnsCnt;
                        }
                        for(auto it6=Graph[v6]+1;*it6>root;++it6)
                        {
                            int v7=*it6;
                            if(!InvReach[v7]||Visited[v7]) continue;
                            Visited[v7]=true;
                            Path[Top++]=v7;
                            if(Reach[v7])
                            {
                                int size=ansSize[threadid][4]+threadid*circleNum[4]*7;
                                for(int i=0;i<Top;i++)
                                {
                                    ans[4][size+i]=Path[i];
                                }
                                ansSize[threadid][4]+=7;
                                ++AnsCnt;
                            }
                            Visited[v7]=false;
                            Top--;
                        }
                        Visited[v6]=false;
                        Top--;
                    }
                    Visited[v5]=false;
                    Top--;
                }
                Visited[v4]=false;
                Top--;
            }
            Visited[v3]=false;
            Top--;
        }
        Visited[v2]=false;
        Top--;
    }
    Visited[root]=false;
    Top--;
}

void *ThreadWork(void *arg)
{
	int Idx=long(arg);
    AnsCnt = 0;
	Top = 0;
	for(int i=Maxid+Idx-Maxid%THREADNUM;i>=0;i-=THREADNUM)
    {
		if (Graph[i][0]!=0)
		{
            for(int vi=1;vi<=GraphInv[i][0];++vi)//对i的所有逆邻接点k遍历
            {
                int k = GraphInv[i][vi];//k为i的逆邻接点
                InvReach[k]=true;
                Reach[k]=true;
                for (int vk = 1; vk <= GraphInv[k][0]; ++vk)//对k的所有逆邻接点j进行遍历
                {
                    int j=GraphInv[k][vk];
                    InvReach[j]=true;
                    for(int vj=1;vj<=GraphInv[j][0];++vj)//对j的所有逆邻接点进行遍历
                    {
                        InvReach[GraphInv[j][vj]]=true;
                    }
                }
            }
            ForRecur(i,Idx);
            for(int vi=1;vi<=GraphInv[i][0];++vi)//对i的所有逆邻接点k遍历
            {
                int k = GraphInv[i][vi];//k为i的逆邻接点
                InvReach[k]=false;
                Reach[k]=false;
                for (int vk = 1; vk <= GraphInv[k][0]; ++vk)//对k的所有逆邻接点j进行遍历
                {
                    int j=GraphInv[k][vk];
                    InvReach[j]=false;
                    for(int vj=1;vj<=GraphInv[j][0];++vj)//对j的所有逆邻接点进行遍历
                    {
                        InvReach[GraphInv[j][vj]]=false;
                    }
                }
            }
		}
    }
    return (void*)AnsCnt;
}


void FindCircle()
{
#ifdef TIME
	clock_t start=clock();
#endif
    pthread_t tid[THREADNUM-1];
    for(int i=0;i<THREADNUM-1;i++)
    {
        pthread_create(&tid[i],NULL,ThreadWork,(void*)(i+1));
    }	
	AnsCnt = 0;
	Top = 0;
	for (int i =Maxid-Maxid%THREADNUM; i >= 0; i-=THREADNUM)
	{
#ifdef TEST
		if (i % 100 == 0) {
			cout << i << "/" << NodeCnt << " ~ " << AnsCnt << endl;
		}
#endif
		if (Graph[i][0]!=0)
		{
            for(int vi=1;vi<=GraphInv[i][0];++vi)//对i的所有逆邻接点k遍历
            {
                int k = GraphInv[i][vi];//k为i的逆邻接点
                InvReach[k]=true;
                Reach[k]=true;
                for (int vk = 1; vk <= GraphInv[k][0]; ++vk)//对k的所有逆邻接点j进行遍历
                {
                    int j=GraphInv[k][vk];
                    InvReach[j]=true;
                    for(int vj=1;vj<=GraphInv[j][0];++vj)//对j的所有逆邻接点进行遍历
                    {
                        InvReach[GraphInv[j][vj]]=true;
                    }
                }
            }
            ForRecur(i,0);
            for(int vi=1;vi<=GraphInv[i][0];++vi)//对i的所有逆邻接点k遍历
            {
                int k = GraphInv[i][vi];//k为i的逆邻接点
                InvReach[k]=false;
                Reach[k]=false;
                for (int vk = 1; vk <= GraphInv[k][0]; ++vk)//对k的所有逆邻接点j进行遍历
                {
                    int j=GraphInv[k][vk];
                    InvReach[j]=false;
                    for(int vj=1;vj<=GraphInv[j][0];++vj)//对j的所有逆邻接点进行遍历
                    {
                        InvReach[GraphInv[j][vj]]=false;
                    }
                }
            }

		}
	}
    int sum[THREADNUM-1];
    for(int i=0;i<THREADNUM-1;i++)
        pthread_join(tid[i],(void**)&sum[i]);
    CircleCnt=AnsCnt;
    for(int i=0;i<THREADNUM-1;++i)
        CircleCnt+=sum[i];
    //printf("CircleCnt:%d\n",CircleCnt);
#ifdef TIME
	clock_t end=clock();
	cout<<"FindCircle time="<<(double)(end-start)/1000<<"ms"<<endl;
#endif
#ifdef TEST
	for (int i = MIN_DEPTH; i <= MAX_DEPTH; i++) {
		printf("Loop Size %d: %d/%d ~ %.5lf\n", i, ansSize[i-3]/i, AnsCnt, ansSize[i-3]*1.0 / (i*AnsCnt));
	}
#endif
}

void *ThreadWrite(void *arg)
{
	PTD *ptd=(PTD*)arg;
	char *p=NULL;
	for(int i=ptd->start;i<=ptd->end;++i)
	{
		p=(char*)mmap(NULL,ptd->size,PROT_READ|PROT_WRITE,MAP_SHARED,ptd->fd,i*ptd->size);
		memcpy(p,pData+i*ptd->size,ptd->size);
		munmap(p,ptd->size);
	}
}

void OutputAnswer(const string &outputfile)
{
#ifdef TIME
	clock_t start=clock();
#endif
	int fd;
	int Datalen=0;
	char *p=NULL;
	char *pTemp=pData;
	//打开文件
	fd=open(outputfile.c_str(),O_RDWR|O_CREAT,0666);
	string Cntstr=to_string(CircleCnt)+'\n';
	int length=Cntstr.size();
	memcpy(pTemp,Cntstr.c_str(),length);
	pTemp+=length;
	Datalen+=length;
	for (int i = 0; i <= 4; ++i)
	{			
		int num = i+3;
        for(int k=0;k<=Maxid;++k)
        {
            int idx=k%THREADNUM;//获得线程索引 
			int index=idx*circleNum[i]*num+ansSize[idx][i]-num;
            while(index>=0&&ans[i][index]==k)
            {
                ansSize[idx][i]-=num;
                for(int n=0;n<num-1;++n)
                {
					int lenTmp=IdWithComma[ans[i][index+n]].size();
					memcpy(pTemp,IdWithComma[ans[i][index+n]].c_str(),lenTmp);
					pTemp+=lenTmp;
					Datalen+=lenTmp;
                }
				int lenTmp=IdWithLF[ans[i][index+num-1]].size();
				memcpy(pTemp,IdWithLF[ans[i][index+num-1]].c_str(),lenTmp);
				pTemp+=lenTmp;
				Datalen+=lenTmp;
                index-=num;
            }
        }
	}
    int threadnum=0;
	int BlockSize=1024*1024;
	 int n=Datalen/BlockSize;
	if(n<=1)
	{
		threadnum=0;
	}
	else if(n==2)
	{
		threadnum=1;
	}
	else if(n==3)
	{
		threadnum=2;
	}
	else
	{
		threadnum=3;
	}
	int m=n/(threadnum+1);
	int t=n%(threadnum+1);
	int Cnt=threadnum>=1?threadnum:1;
	vector<int> CntVec(Cnt,m);
	while(t-->0)
	{
		CntVec[t]+=1;
	}
	int ret=ftruncate(fd,Datalen);
	pthread_t tid[Cnt];
	PTD ptd[Cnt];
	for(int i=0;i<threadnum;++i)
	{
		ptd[i].fd=fd;
		ptd[i].size=BlockSize;
		ptd[i].start=accumulate(CntVec.begin(),CntVec.begin()+i,0);;
		ptd[i].end=ptd[i].start+CntVec[i]-1;
		pthread_create(&tid[i],NULL,ThreadWrite,(void*)&ptd[i]);
	}
	int tmp=0;
	if(threadnum>=1)
		tmp=accumulate(CntVec.begin(),CntVec.end(),0);
	for(int i=tmp;i<n;++i)
	{   
		p=(char*)mmap(NULL,BlockSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,i*BlockSize);
		memcpy(p,pData+i*BlockSize,BlockSize);
		munmap(p,BlockSize);
	}

	int size=Datalen-n*BlockSize;
	if(size>0)
	{
		p=(char*)mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,n*BlockSize);
		memcpy(p,pData+n*BlockSize,size);
		munmap(p,size);
	}
	for(int i=0;i<threadnum;++i)
		pthread_join(tid[i],NULL);
	close(fd);
#ifdef TIME
	clock_t end=clock();
	cout<<"OutputAnswer time="<<(double)(end-start)/1000<<"ms"<<endl;
#endif	

}



int main()
{
	string inputfile = "/data/test_data.txt";
	string ouputfile = "/projects/student/result.txt";
	ReadData(inputfile);
	ConstructGraph();
	TopoSort();
	FindCircle();
	OutputAnswer(ouputfile);
}
