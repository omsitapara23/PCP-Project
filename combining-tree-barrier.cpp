#include <bits/stdc++.h>
#include <chrono>
#include <ctime>
#include <thread>
#include <sys/time.h>
#include <atomic>
#include <unistd.h>
#include <mutex>
using namespace std;

int radix;
int leaves;
map<int,bool> thread_sense;
atomic<int> mss{0};
int num_nodes = 0;

class Node {
public:
    atomic<int> count;
    Node* parent;
    volatile bool sense;
    int nodeid;
    bool root;
    
    Node(bool isroot, int id) {
        // cout << "setting " << id << endl;
        sense = false;
        parent = NULL;
        count = radix;
        root = isroot;
        nodeid = id;
    }

    Node(Node* myParent, int id, bool isroot) {
        // cout << "setting " << id << endl;
        sense = false;
        parent = NULL;
        count = radix;
        parent = myParent;
        nodeid = id;
        root = isroot;
    }

    void await() {
        bool mysense = thread_sense[nodeid];
        int position = count.fetch_sub(1);
        if(position == 1 ) {
            if(parent != NULL) {
                // printf("Calling await on parent %d by node %d\n", parent->nodeid, nodeid);
                mss += 1;
                parent->await();
                // printf("Freed await on parent %d by node %d\n", parent->nodeid, nodeid);
            }
            count = radix;
            sense = mysense;
        } else {
            // printf("Going in loop on nodeid %d, sense = %d, mysense = %d\n", nodeid, sense, mysense);
            while(sense != mysense) { };
            // printf("Going out loop on nodeid %d\n", nodeid);
        }
        thread_sense[nodeid] = !mysense;
    }
};

Node** leaf;

void buildTree(Node* parent, int depth) {
    if(depth == 0) {
        // cout << "setting leaf " << endl;
        for(int i = 0; i < radix; i++)
            leaf[leaves++] = parent;
    } else {
        for(int i = 0; i < radix; i++) {
            num_nodes++;
            Node* child = new Node(parent,num_nodes, false);
            buildTree(child, depth - 1);
        }
    }
}

void await(int thid) {
    leaf[thid]->await();
}

int *avg;

void combiningTreeBarrier(int n) {
    leaves = 0;
    leaf = new Node*[n];
    avg = new int[n];
    int depth = 0;

    while(n > 1) {
        depth++;
        n = n/radix;
    }
    Node* root = new Node(true, num_nodes);
    buildTree(root, depth - 1);
}

void calcPrimes(int thID) {
    for(int i = 0; i < 5; i++) {
        int count = 100 * (thID + 1) * (i + 1), c = 0, p = 2;
        while(c != count) {
            bool flag = true;
            for(int i=2; i<sqrt(p); i++) {
                if(p % i == 0) {
                    flag = false;
                    break;
                }
            }
            if(flag)
                c++;
        }
        printf("%dth prime calculation finished by thread %d\n", i + 1, thID + 1);
         auto start = chrono::steady_clock::now();
        await(thID);
        auto end = chrono::steady_clock::now();

        int t = chrono::duration_cast<chrono::microseconds>(end - start).count();
        avg[thID] += t;
    }
    avg[thID] = avg[thID]/5;
}

int main() {
    // int n;
    cin >> radix;
    fstream outfile, outfile1;
    outfile.open("ct_messages.txt", ios::out | ios::app);
    outfile1.open("ct_time.txt", ios::out | ios::app);
    for(int itr = 1; itr < 6; itr++) {
        int n = pow(radix, itr);
        int avg_mss = 0;
        int avg_time = 0;
        for(int inn = 0; inn < 5; inn++) {
            combiningTreeBarrier(n);
            
            for(int i = 0; i < num_nodes + 1; i++)
                thread_sense[i] = true;

            thread threads[n];
            for(int i = 0; i < n; i++) {
                avg[i] = 0;
                threads[i] = thread(calcPrimes, i);
            }

            for(auto &th : threads)
                th.join();

            for(int i=0; i<n; i++) 
                avg_time += avg[i];
            avg_mss += mss;
            mss = 0;           
        }
        avg_time = avg_time/(n*5);
        avg_mss = avg_mss/5;
        outfile1 << n << "," << avg_time << endl;
        outfile << n << "," << avg_mss << endl;
    }
    
    return 0;
}

