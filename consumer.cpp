#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
using namespace std;

const int MAX_NODES = 10000;
const int MAX_FRIENDS = 1000;

struct adj_list
{
    int adj_list[MAX_FRIENDS];
};

struct Graph
{
    int num_of_nodes;
    struct adj_list Adj_List[MAX_NODES];

    Graph(int n)
    {
        num_of_nodes = n;
        memset(Adj_List, 0, sizeof(Adj_List));
    }
    Graph(const Graph &other)
    {
        num_of_nodes = other.num_of_nodes;
        for (int i = 0; i < num_of_nodes; i++)
        {
            memcpy(Adj_List[i].adj_list, other.Adj_List[i].adj_list, sizeof(int) * MAX_FRIENDS);
        }
    }
};

void findPath(int i, vector<int> &parent, string &s)
{
    if (i == -1)
    {
        return;
    }
    findPath(parent[i], parent, s);
    s += to_string(i);
    s += " ";
}

void storePathUtility(vector<int> &sources, vector<int> &parent, int consumerNum, int size)
{
    ofstream file;
    string outputFilename = "output_" + to_string(consumerNum) + ".txt";
    file.open(outputFilename, ios::app);
    for (int i = 0; i < size; i++)
    {
        int flag = 0;
        for (auto x : sources)
        {
            if (i == x)
            {
                flag = 1;
                break;
            }
        }
        if (flag)
        {
            continue;
        }
        string s;
        findPath(i, parent, s);
        file << s << endl;
    }
    file.close();
}

void dikstra(Graph *g, int src, int consumerNum, vector<int> &sources, vector<int> &dist, vector<int> &parentArray)
{
    // vector<int> dist(g->num_of_nodes, INT_MAX);
    vector<bool> finalised(g->num_of_nodes, false);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    dist[src] = 0;
    pq.push(make_pair(0, src));
    // vector<int> parentArray(g->num_of_nodes);
    parentArray[src] = -1;

    for (int cnt = 0; cnt < g->num_of_nodes - 1; cnt++)
    {
        int u = pq.top().second;
        pq.pop();
        finalised[u] = true;
        for (int i = 1; i <= g->Adj_List[u].adj_list[0]; i++)
        // for (auto v : g->adj_list[u])
        {
            int v = g->Adj_List[u].adj_list[i];
            if (!finalised[v] && (dist[u] + 1) < dist[v])
            {
                parentArray[v] = u;
                dist[v] = dist[u] + 1;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
    storePathUtility(sources, parentArray, consumerNum, g->num_of_nodes);
}

void optim(Graph *g, int added_nodes, int src, int consumerNum, vector<int> &sources, vector<int> &dist, vector<int> &parentArray)
{
    int flag = 0;
    for (int i = g->num_of_nodes - added_nodes - 1; i < g->num_of_nodes; i++)
    {
        flag = 0;
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        for (int j = 1; j <= g->Adj_List[src].adj_list[0]; j++)
        {
            if (g->Adj_List[src].adj_list[j] == i)
            {
                dist[i] = 1;
                parentArray[i] = src;
                flag = 1;
                pq.push(make_pair(1, i));
            }
        }
        if (flag == 0)
        {
            int temp_min = INT_MAX, temp_par = -1;
            for (int q = 1; q <= g->Adj_List[i].adj_list[0]; q++)
            {
                if (dist[g->Adj_List[i].adj_list[q]] + 1 < temp_min)
                {
                    temp_min = dist[g->Adj_List[i].adj_list[q]] + 1;
                    temp_par = g->Adj_List[i].adj_list[q];
                }
            }
            dist[i] = temp_min;
            parentArray[i] = temp_par;
            pq.push(make_pair(temp_min, i));
            do
            {
                int u = pq.top().second;
                pq.pop();
                for (int k = 1; k <= g->Adj_List[u].adj_list[0]; k++)
                {
                    int v = g->Adj_List[u].adj_list[k];
                    if (dist[v] > dist[u] + 1)
                    {
                        parentArray[v] = u;
                        dist[v] = dist[u] + 1;
                        pq.push(make_pair(dist[v], v));
                    }
                }
            } while (pq.size() != 0);
        }
    }
    storePathUtility(sources, parentArray, consumerNum, g->num_of_nodes);
}

int main(int argc, char *argv[])
{
    const int SIZE = sizeof(Graph);
    const key_t KEY = 1234;
    int shm_id = shmget(KEY, SIZE, 0666);
    if (shm_id < 0)
    {
        cerr << "Error: cannot access shared memory\n";
        return -1;
    }
    void *ptr = shmat(shm_id, NULL, 0);
    if (ptr == (void *)-1)
    {
        cerr << "Error: cannot map shared memory\n";
        return -1;
    }
    Graph *g = static_cast<Graph *>(ptr);

    vector<vector<int>> vdist(MAX_NODES, vector<int>(MAX_NODES, INT_MAX));
    vector<vector<int>> vparent(MAX_NODES, vector<int>(MAX_NODES, -1));
    int added_nodes = 0;

    // remove the previous content
    for (int i = 0; i < 10; i++)
    {
        ofstream file;
        string outputFilename = "output_" + to_string(i + 1) + ".txt";
        file.open(outputFilename, ios::trunc | ios::out);
        file.close();
    }

    for (int i = 0; i < 10; i++)
    {
        if (fork() == 0)
        {
            while (true)
            {
                vector<int> v;
                Graph *gp = new Graph(*g);
                added_nodes = gp->num_of_nodes - added_nodes;
                int x = gp->num_of_nodes / 10;
                for (int j = 0; j < x; j++)
                {
                    v.push_back(i * x + j);
                }
                if (i == 9)
                {
                    for (int j = (i + 1) * x; j < gp->num_of_nodes; j++)
                    {
                        v.push_back(j);
                    }
                }
                for (int j = 0; j < x; j++)
                {
                    if (argc > 1 && added_nodes != gp->num_of_nodes && strcmp(argv[1], "-optimize") == 0)
                    {
                        // cout << 1 << endl;
                        optim(gp, added_nodes, i * x + j, i + 1, v, vdist[i * x + j], vparent[i * x + j]);
                    }
                    else
                        dikstra(gp, i * x + j, i + 1, v, vdist[i * x + j], vparent[i * x + j]);
                }
                if (i == 9)
                {
                    for (int j = (i + 1) * x; j < gp->num_of_nodes; j++)
                    {
                        if (argc > 1 && added_nodes != gp->num_of_nodes && strcmp(argv[1], "-optimize") == 0)
                        {
                            // cout << 2;
                            optim(gp, added_nodes, j, i + 1, v, vdist[j], vparent[j]);
                        }
                        else
                            dikstra(gp, j, i + 1, v, vdist[j], vparent[j]);
                    }
                }
                added_nodes = gp->num_of_nodes;
                sleep(30);
            }
            exit(1);
        }
    }
}