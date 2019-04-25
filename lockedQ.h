/* 
    A generic class that adds mutual exclusion to std::queue.
*/

#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#ifndef LOCKEDQ_H
#define LOCKEDQ_H

template <typename T>
class lockedQ {
    public:
    lockedQ(std::string nm): qName(nm), complete(false) {}
    lockedQ(const lockedQ&) = delete;
    lockedQ& operator = (lockedQ&) = delete;

    void inputComplete();
    void enQueue(T n);
    T deQueue();
    bool terminateOutput();
    void printStats() const;

    private:
    std::queue<T> objQ;
    std::mutex qMtx;
    std::string qName;
    std::condition_variable qCond;
    std::atomic<bool> complete;
    long obIn = 0,obOut = 0;
};
#endif /* LOCKEDQ_H */
