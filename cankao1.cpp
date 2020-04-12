#include "bits/stdc++.h"
 
using namespace std;
 
void dfs(const vector<vector<int>> &g, vector<vector<int>> &res, vector<int> &visit, vector<int> &path, int k, int p_o)
{
    for (int i = 0; i < g.size(); i++)
    {
        if (g[k][i] == 0)
            continue;
        if (i == p_o)
        {
            res.push_back(path); //保存找到的环
            continue;
        }
        if (visit[i] == 1)
            continue;
        visit[i] = 1;
        path.push_back(i);
        dfs(g, res, visit, path, i, p_o);
        path.pop_back();
        visit[i] = 0;
    }
}
 
bool cmp(vector<int> a, vector<int> b)
{
    //输出排序比较
    if (a.size() == b.size())
    {
        for (int i = 0; i < a.size(); i++)
        {
            if (a[i] == b[i])
                continue;
            return a[i] < b[i];
        }
    }
    else
        return a.size() < b.size();
    return false;
}
bool isSame(vector<int> &a, vector<int> &b)
{
    //比较两个矩阵是否一样
    if (a.size() != b.size())
        return false;
    for (int i = 0; i < a.size(); i++)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}
void rot_vector(vector<int> &nums)
{
    //统一环的起点，最小id为起点
    int min_num = nums[0];
    int min_idx = 0;
    for (int i = 0; i < nums.size(); i++)
    {
        if (min_num > nums[i])
        {
            min_num = nums[i];
            min_idx = i;
        }
    }
    vector<int> temp(nums);
    for (int i = 0; i < nums.size(); i++)
    {
        nums[i] = temp[(i + min_idx) % nums.size()];
    }
}
 
int main()
{
    clock_t start_time, end_time;
    start_time = clock();
    //--------------------------数据读入-----------------------------
    ifstream infile("test_data.txt");
    vector<int> ids1;
    vector<int> ids2;
    vector<int> vals;
    int id1, id2, val;
    char c;
    while (infile >> id1 >> c >> id2 >> c >> val)
    {
        ids1.push_back(id1);
        ids2.push_back(id2);
        vals.push_back(val);
    }
    //-----------------------创建有向图-----------------------------------
    int max_id = 0;
    for (int i = 0; i < ids1.size(); i++)
    {
        if (i == 0 || max_id < ids1[i])
        {
            max_id = ids1[i];
        }
        if (max_id < ids2[i])
        {
            max_id = ids2[i];
        }
    }
    cout << max_id << endl;
    max_id += 1;
    vector<int> temp(max_id, 0);
    vector<vector<int>> g(max_id, temp);
    vector<int> visit(max_id, -1);
    for (int i = 0; i < ids1.size(); i++)
    {
        visit[ids1[i]] = 0;
        visit[ids2[i]] = 0;
    }
    for (int i = 0; i < ids1.size(); i++)
    {
        g[ids1[i]][ids2[i]] = vals[i];
    }
    //-------------------深度遍历找环------------------------------------
    vector<vector<int>> res;
    vector<int> path;
    for (int i = 0; i < visit.size(); i++)
    {
        if (visit[i] == -1)
            continue;
        visit[i] = 1;
        path.push_back(i);
        dfs(g, res, visit, path, i, i);
        visit[i] = 0;
        path.pop_back();
    }
    //-----------------将环排序去重----------------------------------------
    cout << res.size() << endl;
    vector<vector<int>> res1;
    vector<vector<int>> res2;
    for (int i = 0; i < res.size(); i++)
    {
        if (res[i].size() < 3 || res[i].size() > 7)
            continue;
        rot_vector(res[i]);
        res1.push_back(res[i]);
    }
    sort(res1.begin(), res1.end(), cmp);
    res2.push_back(res1[0]);
    for (int i = 1; i < res1.size(); i++)
    {
        if (isSame(res1[i], res1[i - 1]))
            continue;
        res2.push_back(res1[i]);
    }
    //------------------输出结果--------------------------------------------
    ofstream outfile("result.txt");
    outfile << res2.size() << endl;
    for (int i = 0; i < res2.size(); i++)
    {
        for (int j = 0; j < res2[i].size(); j++)
            outfile << res2[i][j] << ",";
        outfile << endl;
    }
    end_time = clock();
    cout << " time : " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
}