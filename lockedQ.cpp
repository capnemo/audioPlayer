#include <iostream>

#include "lockedQ.h"
#include "streamInit.h"

template <typename T>
void lockedQ<T>::inputComplete()
{
    complete = true;
}

template <typename T>
void lockedQ<T>::enQueue(T n)
{
    std::unique_lock<std::mutex> qLck(qMtx);
    objQ.push(n);
    qCond.notify_one();
    obIn++;
}

template <typename T>
T lockedQ<T>::deQueue()
{
    std::unique_lock<std::mutex> qLck(qMtx);
    if (objQ.size() == 0) 
        qCond.wait(qLck);

    T top = nullptr;
    if (objQ.size() != 0) {
        top = objQ.front();
        objQ.pop();
    }
    obOut++;
    return top;
}

template <typename T>
bool lockedQ<T>::terminateOutput()
{
    if (complete == true) {
        std::unique_lock<std::mutex> qLck(qMtx);
        if (objQ.size() == 0)
            return true;
    }
    return false;
}


template <typename T>
void lockedQ<T>::printStats()
{
    std::cout << qName << " In " << obIn << " Out " << obOut << std::endl;
}

template class lockedQ<AVFrame*>;
template class lockedQ<AVPacket*>;
