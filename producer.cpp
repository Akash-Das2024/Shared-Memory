#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <limits.h>
#include <random>
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

void add_edge(Graph &g, int u, int v)
{
    if (u < 0 || u >= g.num_of_nodes || v < 0 || v >= g.num_of_nodes)
    {
        cerr << "Error: invalid node number\n";
        return;
    }
    int k1 = g.Adj_List[u].adj_list[0];
    int k2 = g.Adj_List[v].adj_list[0];
    g.Adj_List[u].adj_list[k1 + 1] = v;
    g.Adj_List[u].adj_list[0]++;
    g.Adj_List[v].adj_list[k2 + 1] = u;
    g.Adj_List[v].adj_list[0]++;
}

int main()
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
    
    while (true)
    {
        int total_edge = 0;
        for (int d = 0; d < g->num_of_nodes; d++)
        {
            total_edge += g->Adj_List[d].adj_list[0];
        }

        random_device rd;
        mt19937 gen(rd());

        uniform_int_distribution<> dist_m(10, 30);
        int m = dist_m(gen);

        for (int i = 0; i < m; i++)
        {
            g->num_of_nodes++;
            uniform_int_distribution<> dist_k(1, 20);
            int k = dist_k(gen);
            for (int j = 0; j < k; j++)
            {
                int random_number = rand() % total_edge;

                for (int coord = 0; coord < g->num_of_nodes - 1; coord++)
                {
                    if (random_number < g->Adj_List[coord].adj_list[0])
                    {
                        add_edge(*g, g->num_of_nodes - 1, coord);
                        total_edge++; // an extra edge will be added so
                        break;
                    }
                    random_number -= g->Adj_List[coord].adj_list[0];
                }
            }

            total_edge += k; // for newly added nodes
        }

        cout << "producer: " << g->num_of_nodes << endl;
        ofstream outfile;
        outfile.open("Adj_list.txt", ios::out | ios::trunc);

        for (int i = 0; i < g->num_of_nodes; i++)
        {
            outfile << i << " --> (" << g->Adj_List[i].adj_list[0] << ") ";
            for (int j = 1; j <= g->Adj_List[i].adj_list[0]; j++)
            {
                outfile << g->Adj_List[i].adj_list[j] << " ";
            }
            outfile << endl;
        }
        sleep(50);
    }
    shmdt(ptr);

    return 0;
}
