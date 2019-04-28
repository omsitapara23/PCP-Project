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
atomic<int> mss{0};

// class Barrier {
// public:
//     virtual void await(int thID) = 0;
// };

int radix;
bool sense;
map<int, bool> threadSense;

class Node {
public:
    atomic<int> childCount;
    Node* parent;
    int children;
    
    Node(Node* myParent, int count) {
        children = count;
        childCount = count;
        parent = myParent;
    }

    void await(int thID) {
        bool mySense = threadSense[thID];
        while(childCount > 0) {};
        childCount = children;

        if(parent != NULL) {
            mss+=1;
            parent->childDone();
            while(sense != mySense) {};
        } else {
            sense = !sense;
        }
        threadSense[thID] = !mySense;
    }

    void childDone() {
        childCount.fetch_sub(1);
    }
};

Node** node;
int nodes;

void buildTree(Node* parent, int depth) {
    if(depth == 0) {
        node[nodes++] = new Node(parent, 0);
    } else {
        Node* myNode = new Node(parent, radix);
        node[nodes++] = myNode;
        for(int i=0; i<radix; i++) {
            buildTree(myNode, depth - 1);
        }
    }
}

void await(int thID) {
    node[thID]->await(thID);
}

int *avg;

void staticTreeBarrier(int n) {
    nodes = 0;
    int size = n;
    node = new Node*[size];
    avg = new int[n];
    int depth = 0;

    while(size > 1) {
        depth++;
        size = size / radix;
    }
    buildTree(NULL, depth);
    sense = false;
}

void calcPrimes(int thID) {
    for(int i=0; i<5; i++) {
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
    int n;
    cin  >> radix;
    fstream outfile, outfile1;
    outfile.open("st_messages.txt", ios::out | ios::app);
    outfile1.open("st_time.txt", ios::out | ios::app);
    for(int itr = 1; itr < 6; itr++)
    {
        int n = (pow(radix,itr) - 1)/(radix - 1);
        int avg_mss = 0;
        int avg_time = 0;
        for(int inn = 0; inn < 5; inn++)
        {
            staticTreeBarrier(n);

            // int numNodes = pow(radix, depth + 1);
            for(int i=0; i<nodes; i++)
                threadSense[i] = !sense;

            thread threads[n];
            for(int i=0; i<n; i++) {
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
