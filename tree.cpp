#include <bits/stdc++.h>
#include <chrono>
#include <ctime>
#include <thread>
#include <sys/time.h>
#include <atomic>
#include <unistd.h>
#include <mutex>
using namespace std;

mutex mtx;
int radix;
int leaves;
map<int,bool> thread_sense;
int num_nodes = 0;
class Node
{
    public:
        atomic<int> count;
        Node* parent;
        volatile bool sense;
        int nodeid;
        bool root;
        Node(bool isroot, int id)
        {
            cout << "setting " << id << endl;
            sense = false;
            parent = NULL;
            count = radix;
            root = isroot;
            nodeid = id;
        }
        Node(Node* myParent, int id, bool isroot)
        {
            cout << "setting " << id << endl;
            sense = false;
            parent = NULL;
            count = radix;
            parent = myParent;
            nodeid = id;
            root = isroot;
        }

        void await()
        {

            // if(root)
            // {
            //     bool mysense = thread_sense[nodeid];
            //     int position = count.fetch_sub(1);
            //     if(position == 1 )
            //     {
            //         count = radix;
            //         sense = mysense;
            //     }
            //     else
            //     {
            //         while(sense != mysense)
            //         {

            //         };
            //     }
            //     thread_sense[nodeid] = !mysense;
            // }
            // else
            // {
                bool mysense = thread_sense[nodeid];
                int position = count.fetch_sub(1);
                if(position == 1 )
                {
                    // mtx.lock();
                    // cout << "leaving towards parent " << nodeid << endl;
                    // mtx.unlock();

                    if(parent != NULL)
                    {
                        parent->await();
                    }
                    count = radix;
                    sense = mysense;
                }
                else
                {
                    // mtx.lock();

                    // cout << "going wait " << nodeid << endl;
                    // mtx.unlock();

                    while(sense != mysense)
                    {

                    };
                    // mtx.lock();

                    // cout << "going wait " << nodeid << endl;
                    // mtx.unlock();
                }
                thread_sense[nodeid] = !mysense;
            // }
            
            

            
        }

};

Node** leaf;



void buildTree(Node* parent, int depth)
{
    if(depth == 0)
    {
        cout << "setting leaf " << endl;
        // for(int i = 0; i < radix; i++)
        // {
            leaf[leaves++] = parent;
        // }
    }
    else
    {
        for(int i = 0; i < radix; i++)
        {
            num_nodes++;
            Node* child = new Node(parent,num_nodes, false);
            buildTree(child, depth - 1);
        }
    }
    
}
void TreeBarrier(int n, int r)
{
    radix = r;
    leaves = 0;
    leaf = new Node*[n/r];
    int depth = 0;

    while(n > 1)
    {
        depth++;
        n = n/r;
    }
    cout << "Depth " << depth << endl;
    Node* root = new Node(true, num_nodes);
    buildTree(root, depth - 1);
}

void await(int thid)
{
    leaf[thid/radix]->await();
}

void th_func(int id)
{
    for(int i = 0; i < 5; i++)
    {
        mtx.lock();
        cout << "Thread id " << id << " for " << i << endl;
        mtx.unlock();
        usleep(1000000);
        await(id);
    }
}

int main()
{
    int n;
    cin >> n;
    TreeBarrier(n,2);
    for(int i = 0; i < num_nodes; i++)
        thread_sense[i] = true;
    thread threads[n];
    for(int i = 0; i < n; i++)
    {
        threads[i] = thread(th_func, i);
    }
    for(auto &th : threads)
        th.join();
    return 0;
}

