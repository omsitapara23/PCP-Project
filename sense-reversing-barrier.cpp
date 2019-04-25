#include <bits/stdc++.h>
#include <thread>
#include <fstream>
#include <chrono>
#include <ctime>
#include <atomic>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
using namespace std;

class Barrier {
public:
    virtual void await(int thID) = 0; 
};

class SenseReversingBarrier : public Barrier {
    atomic<int> count;
    int size;
    bool sense;
    // For maintaining thread local bool variable
    // we have used a map of bool variables for each thread
    unordered_map<int, bool> threadSense;

public:
    SenseReversingBarrier(int n) {
        count = n;
        size = n;
        sense = false;
        for(int i=0; i<n; i++)
            threadSense[i] = !sense;
    }

    void await(int thID) {
        bool mySense = threadSense[thID];
        int position = count.fetch_sub(1);      // Acts as getAndDecrement() of Java
        if(position == 1) {
            // usleep(100);
            count = size;
            sense = mySense;
        } else {
            while(sense != mySense) {};
        }
        threadSense[thID] = !mySense;
    }
};

SenseReversingBarrier* bSense;

void calcPrimes(int thID) {
    int count = 10 * (thID + 1), c = 0, p = 2;
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
    printf("Prime calculation finished by thread %d\n", thID + 1);
    bSense->await(thID);
}

int main(int argc, char const *argv[]) {
    int n;
    cin >> n;
    thread th[n];
    bSense = new SenseReversingBarrier(n);

    for(int i=0; i<n; i++) {
        th[i] = thread(calcPrimes, i);
    }

    for(int i=0; i<n; i++) {
        th[i].join();
    }
    return 0;
}