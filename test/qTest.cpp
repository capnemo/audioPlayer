#include <iostream>
#include <thread>
#include "lockedQ.h"

class dataSource {

    public:
    dataSource(lockedQ<int*>& dQ):dataQ(dQ) {}

    void insert(const std::vector<int*>& input) {
        std::cout << "Input" << std::endl;
        for (auto m:input) {
            std::cout << "|" << *m << std::endl;
            dataQ.enQueue(m);
        }
        dataQ.inputComplete();
        std::cout << "*********" << std::endl;
    }

    private:
    lockedQ<int*>& dataQ;
};

class dataSink {

    public:
    dataSink(lockedQ<int*>& dQ):dataQ(dQ) {}

    void extract() {
        while (dataQ.terminateOutput() == false) {
            int *ret = dataQ.deQueue();
            out.push_back(ret);
            std::cout << "$" << *(dataQ.deQueue()) << ",";
            
        }
        std::cout << std::endl;
    }
    
    std::vector<int*> getReturn() 
    {
        return out;
    }
    
    private:
    lockedQ<int*>& dataQ;
    std::vector<int*> out = {};
};

int main(int argc, char *argv[])
{
    lockedQ<int*> lq("intQ");
    dataSource ds(lq);
    dataSink dsk(lq);
    std::vector<int*> r1 = {new int(1), new int(2)};
    std::vector<int*> r2 = {};
    std::vector<std::vector<int*>> rows = {r1, r2};

    int serial = 0;
        std::thread inTh = std::thread(&dataSource::insert, &ds, r1);
        std::thread outTh = std::thread(&dataSink::extract, &dsk);

        outTh.join();
        inTh.join();
    
        std::vector<int*> rVec = dsk.getReturn();
        std::cout << r1.size() << " , " << rVec.size() << std::endl;
        if (rVec == r1) 
            std::cout << "Test " << ++serial << " passed" << std::endl;
        else 
            std::cout << "Test " << ++serial << " failed" << std::endl;
    
#if 0    
    int serial = 0;
    for (auto m:rows) {
        std::thread inTh = std::thread(&dataSource::insert, &ds, m);
        std::thread outTh = std::thread(&dataSink::extract, &dsk);

        outTh.join();
        inTh.join();
    
        std::vector<int*> rVec = dsk.getReturn();
        std::cout << m.size() << " , " << rVec.size() << std::endl;
        if (rVec == m) 
            std::cout << "Test " << ++serial << " passed" << std::endl;
        else 
            std::cout << "Test " << ++serial << " failed" << std::endl;
    }
 
#endif
   
    for (auto r:rows)
        for (auto m:r)
            delete m;
}

