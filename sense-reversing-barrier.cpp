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

int *avg;

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
        bSense->await(thID);
        auto end = chrono::steady_clock::now();

        int t = chrono::duration_cast<chrono::microseconds>(end - start).count();
        avg[thID] += t;
    }
    avg[thID] = avg[thID]/5;
}

int main(int argc, char const *argv[]) {
    fstream outfile, outfile1;
    outfile1.open("sr_time.txt", ios::out | ios::app);
    for(int itr = 2; itr < 120; itr+=2) {
        int n = itr;
        int avg_mss = 0;
        int avg_time = 0;
        for(int inn = 0; inn < 5; inn++) {
            thread th[n];
            avg = new int[n];
            bSense = new SenseReversingBarrier(n);
            
            for(int i=0; i<n; i++) {
                avg[i] = 0;
                th[i] = thread(calcPrimes, i);
            }

            for(int i=0; i<n; i++) {
                th[i].join();
            }
            
            for(int i=0; i<n; i++) 
                avg_time += avg[i];
        }
        avg_time = avg_time/(n*5);
        outfile1 << n << "," << avg_time << endl;
    }
    
    return 0;
}