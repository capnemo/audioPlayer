#include <iostream>

#include "lockedQ.h"
#include "streamInit.h"

/*
 *  Function to assert that the input is complete
 */
template <typename T>
void lockedQ<T>::inputComplete()
{
    complete = true;
}

/*
 *  Add object n to the queue
*/
template <typename T>
void lockedQ<T>::enQueue(T n)
{
    std::unique_lock<std::mutex> qLck(qMtx);
    objQ.push(n);
    qCond.notify_one();
    obIn++;
}

/*
 *  return the object from the top of the queue
*/
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

/*
 *  return true if there is no more input forthcoming and if the
 *  queue is empty
 */

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

/*
 *  print the number of object that have entered and exited the queue.
 */
template <typename T>
void lockedQ<T>::printStats()
{
    std::cout << qName << " In " << obIn << " Out " << obOut << std::endl;
}

template class lockedQ<AVFrame*>;
