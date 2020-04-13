#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <ctime>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>

#define TEST

using namespace std;

struct Data {
    vector<double> features;
    int label;
    Data(vector<double> f, int l) : features(f), label(l)
    {}
};
struct Param {
    vector<double> wtSet;
};


//Split "mem" into "parts", e.g. if mem = 10 and parts = 4 you will have: 0,2,4,6,10
//if possible the function will split mem into equal chuncks, if not 
//the last chunck will be slightly larger

vector<int> bounds(int parts, int mem) {
    vector<int> bnd;
    int delta = mem / parts;
    int reminder = mem % parts;
    int N1 = 0, N2 = 0;
    bnd.push_back(N1);
    for (int i = 0; i < parts; ++i) {
        N2 = N1 + delta;
        if (i == parts - 1)
            N2 += reminder;
        bnd.push_back(N2);
        N1 = N2;
    }
    return bnd;
}

class LR {
public:
    void train();
    void predict();
    int loadModel();
    int storeModel();
    LR(string trainFile, string testFile, string predictOutFile);

    static void* run(void*);
    static void* runPredict(void*);

private:
    vector<Data> trainDataSet;
    vector<Data> testDataSet;
    vector<int> predictVec;
    Param param;
    string trainFile;
    string testFile;
    string predictOutFile;
    string weightParamFile = "modelweight.txt";

    pthread_mutex_t mutex;
    vector<double> wRes;        //存放权重调整值
    const int threadNum=4;      //线程数

private:
    bool init();
    bool loadTrainData();
    bool loadTestData();
    int storePredict(vector<int> &predict);
    void initParam();
    double wxbCalc(const Data &data);
    double sigmoidCalc(const double wxb);
    double lossCal();
    double gradientSlope(const vector<Data> &dataSet, int index, const vector<double> &sigmoidVec);

private:
    int featuresNum;


    const double wtInitV = 1.0;
    //const double stepSize = 0.1;
    const double stepSize = 0.2;
    //const int maxIterTimes = 300;
    const int maxIterTimes = 500;
    const double predictTrueThresh = 0.5;
    //const int train_show_step = 10;
    const int train_show_step = 10;

    //stepSize:0.2  maxIterTimes:500  accuracy:0.8315
    //stepSize:0.5  maxIterTimes:500  accuracy:0.795
    //stepSize:0.1  maxIterTimes:1000  accuracy:0.8435 
    //stepSize:0.1  maxIterTimes:800  accuracy:0.8425
    //stepSize:0.5  maxIterTimes:800  accuracy:0.848
};

struct threadData{
    LR* pLr;
    int l,r;
};

LR::LR(string trainF, string testF, string predictOutF)
{
    trainFile = trainF;
    testFile = testF;
    predictOutFile = predictOutF;
    featuresNum = 0;
    mutex=PTHREAD_MUTEX_INITIALIZER;
    init();
}

bool LR::loadTrainData()
{
    //ifstream infile(trainFile.c_str());
    //string line;
    FILE* infile=fopen(trainFile.c_str(),"r");
    char line[10000];
    trainDataSet.reserve(10000);

    if (!infile) {
        cout << "打开训练文件失败" << endl;
        exit(0);
    }
    stringstream sin(line);

    while (fgets(line,10000,infile)!=NULL) {
        //fget替换getline，减少类的构造等耗时，直接操作char数组
        //getline(infile, line);
        //if (line.size() > featuresNum) {
            //string和stringstream的构造析构很耗时，尽量避免在深度循环中不断使用
            //stringstream sin(line);
            sin.clear();
            sin.str("");
            sin << line;
            
            char ch;
            double dataV;
            int i;
            vector<double> feature;
            feature.reserve(2000);
            i = 0;

            while (sin) {
                char c = sin.peek();
                if (int(c) != -1) {
                    sin >> dataV;
                    feature.push_back(dataV);
                    sin >> ch;
                    i++;
                } else {
                    cout << "训练文件数据格式不正确，出错行为" << (trainDataSet.size() + 1) << "行" << endl;
                    return false;
                }
            }
            int ftf;  //该行特征数据的标签
            ftf = (int)feature.back();
            feature.pop_back();
            trainDataSet.push_back(Data(feature, ftf));
        //}
    }
    fclose(infile);
    return true;
}

void LR::initParam()
{
    int i;
    for (i = 0; i < featuresNum; i++) {
        param.wtSet.push_back(wtInitV);
    }
}

bool LR::init()
{
    trainDataSet.clear();
    bool status = loadTrainData();
    if (status != true) {
        return false;
    }
    featuresNum = trainDataSet[0].features.size();
    param.wtSet.clear();
    initParam();
    return true;
}

//weight权重向量和feature特征值向量的内积
// double LR::wxbCalc(const Data &data)
// {
//     double mulSum = 0.0L;
//     int i;
//     double wtv, feav;
//     for (i = 0; i < param.wtSet.size(); i++) {
//         wtv = param.wtSet[i];
//         feav = data.features[i];
//         mulSum += wtv * feav;
//     }

//     return mulSum;
// }

//多线程计算向量内积，再合并，map/reduce？
double LR::wxbCalc(const Data &data)
{
    double mulSum = 0.0L;
    int i;
    double wtv, feav;
    for (i = 0; i < param.wtSet.size(); i++) {
        wtv = param.wtSet[i];
        feav = data.features[i];
        mulSum += wtv * feav;
    }

    return mulSum;
}

//逻辑回归的sigmoid函数，用于将内积映射到0～1的区间，若大于0.5则判为1
inline double LR::sigmoidCalc(const double wxb)
{
    double expv = exp(-1 * wxb);
    double expvInv = 1 / (1 + expv);
    return expvInv;
}


double LR::lossCal()
{
    double lossV = 0.0L;
    int i;

    for (i = 0; i < trainDataSet.size(); i++) {
        lossV -= trainDataSet[i].label * log(sigmoidCalc(wxbCalc(trainDataSet[i])));
        lossV -= (1 - trainDataSet[i].label) * log(1 - sigmoidCalc(wxbCalc(trainDataSet[i])));
    }
    lossV /= trainDataSet.size();
    return lossV;
}


double LR::gradientSlope(const vector<Data> &dataSet, int index, const vector<double> &sigmoidVec)
{
    double gsV = 0.0L;
    int i;
    double sigv, label;
    for (i = 0; i < dataSet.size(); i++) {
        sigv = sigmoidVec[i];
        label = dataSet[i].label;
        gsV += (label - sigv) * (dataSet[i].features[index]);
    }

    gsV = gsV / dataSet.size();
    return gsV;
}

//线程的执行函数
void* LR::run(void* arg)
{
    //arg是一个结构体指针，包含this指针、每次要点乘的两个向量的左右边界l、r
    threadData* data=static_cast<threadData*>(arg);
    //double partialSum=0.0;
    LR* p=data->pLr;
    int wNum=p->param.wtSet.size();
    vector<Data> &dataSet=p->trainDataSet;
    vector<double> gsV(wNum);//临时存放梯度变化值
    double sigv, label;
    
    //cout<<data->l<<" "<<data->r <<endl;
	for(int i = data->l; i < data->r; i++){
        sigv=p->sigmoidCalc(p->wxbCalc(dataSet[i]));
        label=dataSet[i].label;
        double dif=label-sigv;
        for(int j=0;j<wNum;j++){
            gsV[j] += dif*(dataSet[i].features[j]);
        }
		//partialSum += p->trainDataSet[i] * v2[i];
	}
	//result += partial_sum;

    pthread_mutex_lock(&p->mutex);
    for(int i=0;i<wNum;i++){
        p->wRes[i]+=gsV[i];
    }
    pthread_mutex_unlock(&p->mutex);
    return NULL;
}


void LR::train()
{
    double sigmoidVal;
    double wxbVal;
    int i, j;
    vector<pthread_t> threads(threadNum-1);
    vector<threadData> threadDatas(threadNum);
    wRes.resize(param.wtSet.size());

    int n=trainDataSet.size();
    //Split n into threadNum parts
	vector<int> edges = bounds(threadNum, n);
    for(int j=0;j<threadNum;j++){
        threadDatas[j].pLr=this;
        threadDatas[j].l=edges[j];
        threadDatas[j].r=edges[j+1];
    }

    for (i = 0; i < maxIterTimes; i++) {
        //vector<double> sigmoidVec(n);//存储一次迭代过程中所有样本的sigmoid激活值，用于反向传播纠正weight权重

        //Launch a group of threads
        for (int k = 0; k < threadNum-1; k++) {
            pthread_create(&threads[k], NULL, run, (void*)&threadDatas[k]);
        }

        //主线程中运行最后一个间隔段的数据运算 上面应该创建threadNum-1个线程
        {
            double sigv, label;
            int wNum=param.wtSet.size();
            vector<double> gsV(wNum);//临时存放梯度变化值
    
            //cout<<data->l<<" "<<data->r <<endl;
            for(int m = threadDatas[threadNum-1].l; m < threadDatas[threadNum-1].r; m++){
                sigv=sigmoidCalc(wxbCalc(trainDataSet[m]));
                label=trainDataSet[m].label;
                double dif=label-sigv;
                for(int j=0;j<wNum;j++){
                    gsV[j] += dif*(trainDataSet[m].features[j]);
                }
            }

            pthread_mutex_lock(&mutex);
            for(int m=0;m<wNum;m++){
                wRes[m]+=gsV[m];
            }
            pthread_mutex_unlock(&mutex);
        }

        //Join the threads with the main thread
        for (int k = 0; k < threadNum-1; k++) {
            pthread_join(threads[k], NULL);
        }
        //并行计算优化sigmoid向量值的计算
        // for (j = 0; j < n; j++) {
        //     wxbVal = wxbCalc(trainDataSet[j]);
        //     sigmoidVal = sigmoidCalc(wxbVal);
        //     sigmoidVec[j]=sigmoidVal;
        // }

        for (j = 0; j < param.wtSet.size(); j++) {
            //对于训练数据集中的每一列特征数据，点乘真实标签与预测分值的差值，即cost function计算过程，stepSize为学习率，调整梯度下降速度
            //param.wtSet[j] += stepSize * gradientSlope(trainDataSet, j, sigmoidVec);
            param.wtSet[j] += stepSize * wRes[j]/n;
        }

        if (i % train_show_step == 0) {
            //cout << "iter " << i << ". updated weight value is : ";
            // for (j = 0; j < param.wtSet.size(); j++) {
            //     cout << param.wtSet[j] << "  ";
            // }
            cout << "iter " << i <<endl;
            //cout << endl;
        }
    }
}

void* LR::runPredict(void* arg)
{
    threadData* data=static_cast<threadData*>(arg);
    int l=data->l,r=data->r;
    LR* p=data->pLr;
    int predictVal;
    vector<int> predict(p->testDataSet.size());
    vector<Data> &dataSet=p->testDataSet;
    for(int i=l;i<r;i++){
        double sigVal=p->sigmoidCalc(p->wxbCalc(dataSet[i]));
        predictVal = sigVal >= p->predictTrueThresh ? 1 : 0;
        predict[i]=predictVal;
    }
    pthread_mutex_lock(&p->mutex);
    for(int i=l;i<r;i++){
        p->predictVec[i]=predict[i];
    }
    pthread_mutex_unlock(&p->mutex);
    return NULL;
}

void LR::predict()
{
    //double sigVal;
    //int predictVal;
    vector<pthread_t> threads(threadNum-1);
    vector<threadData> threadDatas(threadNum);
    predictVec.reserve(10000);

    loadTestData();
    int n=testDataSet.size();
    predictVec.resize(n);
    //Split n into threadNum parts
	vector<int> edges = bounds(threadNum, n);
    for(int j=0;j<threadNum;j++){
        threadDatas[j].pLr=this;
        threadDatas[j].l=edges[j];
        threadDatas[j].r=edges[j+1];
    }

    for (int k = 0; k < threadNum-1; k++) {
        pthread_create(&threads[k], NULL, runPredict, (void*)&threadDatas[k]);
    }

    {
        double sigVal;
        int predictVal;
        int l=threadDatas[threadNum-1].l;
        int r=threadDatas[threadNum-1].r;
        vector<int> predict(testDataSet.size());
        for(int m = l; m < r; m++){
            sigVal = sigmoidCalc(wxbCalc(testDataSet[m]));
            predictVal = sigVal >= predictTrueThresh ? 1 : 0;
            predict[m]=predictVal;
        }
        pthread_mutex_lock(&mutex);
        for(int i=l;i<r;i++){
            predictVec[i]=predict[i];
        }
        pthread_mutex_unlock(&mutex);
    }
    // for (int j = 0; j < testDataSet.size(); j++) {
    //     sigVal = sigmoidCalc(wxbCalc(testDataSet[j]));
    //     predictVal = sigVal >= predictTrueThresh ? 1 : 0;
    //     predictVec[j]=predictVal;
    // }

    //Join the threads with the main thread
    for (int k = 0; k < threadNum-1; k++) {
        pthread_join(threads[k], NULL);
    }

    storePredict(predictVec);
}

int LR::loadModel()
{
    string line;
    int i;
    vector<double> wtTmp;
    double dbt;

    ifstream fin(weightParamFile.c_str());
    if (!fin) {
        cout << "打开模型参数文件失败" << endl;
        exit(0);
    }

    getline(fin, line);
    stringstream sin(line);
    for (i = 0; i < featuresNum; i++) {
        char c = sin.peek();
        if (c == -1) {
            cout << "模型参数数量少于特征数量，退出" << endl;
            return -1;
        }
        sin >> dbt;
        wtTmp.push_back(dbt);
    }
    param.wtSet.swap(wtTmp);
    fin.close();
    return 0;
}

int LR::storeModel()
{
    string line;
    int i;

    ofstream fout(weightParamFile.c_str());
    if (!fout.is_open()) {
        cout << "打开模型参数文件失败" << endl;
    }
    if (param.wtSet.size() < featuresNum) {
        cout << "wtSet size is " << param.wtSet.size() << endl;
    }
    for (i = 0; i < featuresNum; i++) {
        fout << param.wtSet[i] << " ";
    }
    fout.close();
    return 0;
}


bool LR::loadTestData()
{
    //ifstream infile(testFile.c_str());
    //string lineTitle;
    FILE* infile=fopen(testFile.c_str(),"r");
    char line[10000];
    testDataSet.reserve(10000);

    if (!infile) {
        cout << "打开测试文件失败" << endl;
        exit(0);
    }
    stringstream sin;

    while (fgets(line,10000,infile)!=NULL) {
        vector<double> feature;
        feature.reserve(1000);
        //string line;
        //getline(infile, line);
        //if (line.size() > featuresNum) {
            sin.clear();
            sin.str("");
            sin << line;
            double dataV;
            int i;
            char ch;
            i = 0;
            while (i < featuresNum && sin) {
                char c = sin.peek();
                if (int(c) != -1) {
                    sin >> dataV;
                    feature.push_back(dataV);
                    sin >> ch;
                    i++;
                } else {
                    cout << "测试文件数据格式不正确" << endl;
                    return false;
                }
            }
            testDataSet.push_back(Data(feature, 0));
        //}
    }

    fclose(infile);
    return true;
}

bool loadAnswerData(string awFile, vector<int> &awVec)
{
    ifstream infile(awFile.c_str());
    if (!infile) {
        cout << "打开答案文件失败" << endl;
        exit(0);
    }

    while (infile) {
        string line;
        int aw;
        getline(infile, line);
        if (line.size() > 0) {
            stringstream sin(line);
            sin >> aw;
            awVec.push_back(aw);
        }
    }

    infile.close();
    return true;
}

int LR::storePredict(vector<int> &predict)
{
    string line;
    int i;

    ofstream fout(predictOutFile.c_str());
    if (!fout.is_open()) {
        cout << "打开预测结果文件失败" << endl;
    }
    for (i = 0; i < predict.size(); i++) {
        fout << predict[i] << endl;
    }
    fout.close();
    return 0;
}

int main(int argc, char *argv[])
{
    clock_t time_start=clock();
    vector<int> answerVec;
    vector<int> predictVec;
    int correctCount;
    double accurate;

#ifdef TEST
    string trainFile = "../data/train_data.txt";
    string testFile = "../data/test_data.txt";
    string predictFile = "../projects/student/result.txt";

    string answerFile = "../projects/student/answer.txt";
#else
    //原始文件路径，提交代码时记得改回来
    string trainFile = "/data/train_data.txt";
    string testFile = "/data/test_data.txt";
    string predictFile = "/projects/student/result.txt";

    string answerFile = "/projects/student/answer.txt";
#endif


    LR logist(trainFile, testFile, predictFile);

    cout << "ready to train model" << endl;
    logist.train();

    cout << "training ends, ready to store the model" << endl;
    logist.storeModel();

#ifdef TEST
    cout << "ready to load answer data" << endl;
    answerVec.reserve(10000);
    loadAnswerData(answerFile, answerVec);
#endif

    cout << "let's have a prediction test" << endl;
    logist.predict();

#ifdef TEST
    predictVec.reserve(10000);
    loadAnswerData(predictFile, predictVec);
    cout << "test data set size is " << predictVec.size() << endl;
    correctCount = 0;
    for (int j = 0; j < predictVec.size(); j++) {
        if (j < answerVec.size()) {
            if (answerVec[j] == predictVec[j]) {
                correctCount++;
            }
        } else {
            cout << "answer size less than the real predicted value" << endl;
        }
    }

    accurate = ((double)correctCount) / answerVec.size();
    cout << "the prediction accuracy is " << accurate << endl;
#endif
    clock_t time_end=clock();
    cout<<"time use:"<<1000*(time_end-time_start)/(double)CLOCKS_PER_SEC<<"ms"<<endl;
    return 0;
}
