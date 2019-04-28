# PCP-Project
This repo contains the project for the course **CS5300**(Parallel and Concurrent Programming). We have implemented various types of Barriers in C++ and compared their performances according to the time taken for waiting on the barrier.

A barrier is a way of forcing asynchronous threads to act almost as if they were synchronous

We implemented the following barriers:
* **SimpleBarrier:** Most simple type of barrier. Can't be reused.
* **Sense Reversing Barrier:** Reusable barrier, but high contention.
* **Combining Tree Barrier:** Low contention, but high communication overhead.
* **Static Tree Barrier:** Low contention, and low communication.