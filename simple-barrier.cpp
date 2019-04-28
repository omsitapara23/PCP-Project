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
    virtual void await() = 0; 
};

class SimpleBarrier : public Barrier {
    atomic<int> count;
    int size;
public:
    SimpleBarrier(int n) {
        count = n;
        size = n;
    }

    void await() {
        int position = count.fetch_sub(1);      // Acts as getAndDecrement() of Java
        if(position == 1) {
            // usleep(100);
            count = size;
        } else {
            while(count != 0) {};
        }
    }
};

SimpleBarrier* b;

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
        b->await();
    }
}

int main(int argc, char const *argv[]) {
    int n;
    cin >> n;
    thread th[n];
    b = new SimpleBarrier(n);

    for(int i=0; i<n; i++) {
        th[i] = thread(calcPrimes, i);
    }

    for(int i=0; i<n; i++) {
        th[i].join();
    }
    return 0;
}
