#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <limits.h>
#include <sys/wait.h>

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

int main(int argc, char *argv[])
{
    const int SIZE = sizeof(Graph);
    const key_t KEY = 1234;
    int num_nodes = 4039;

    int shm_id = shmget(KEY, SIZE, IPC_CREAT | 0666);
    if (shm_id < 0)
    {
        cerr << "Error: cannot create shared memory\n";
        return -1;
    }

    void *shared_ptr = shmat(shm_id, NULL, 0);
    if (shared_ptr == (void *)-1)
    {
        cerr << "Error: cannot map shared memory\n";
        return -1;
    }

    Graph *g = new (shared_ptr) Graph(num_nodes);

    fstream infile;
    int x, y;
    infile.open("facebook_combined.txt");

    while (infile >> x >> y)
    {
        add_edge(*g, x, y);
    }

    int fd = fork();
    if (fd == 0)
    {
        execl("producer", "./producer", NULL);
    }

    else
    {
        int fc = fork();
        if (fc == 0)
        {
            if (argc > 1 && strcmp(argv[1], "-optimize") == 0)
            {
                execl("consumer", "./consumer", "-optimize", NULL);
            }
            else
            {
                execl("consumer", "./consumer", NULL);
            }
        }
        // wait(NULL);
    }
    wait(NULL);
    // cin.ignore();
    shmdt(shared_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
