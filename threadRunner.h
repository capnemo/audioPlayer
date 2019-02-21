#include <thread>

#ifndef THREADRUNNER_H
#define THREADRUNNER_H
class threadRunner {

    public:
    void startThread()
    {
        thObj = std::thread(&threadRunner::threadFunc, this);
    }
    
    void virtual threadFunc() = 0;

    void joinThread()
    {
        thObj.join();
    }
    
    private:
    std::thread thObj;
};
#endif /* THREADRUNNER_H */
