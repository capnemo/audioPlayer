#include <iostream>
#include <thread>
#include "lockedQ.h"

class dataSource {

    public:
    dataSource(lockedQ<int*>& dQ):dataQ(dQ) {}

    void insert(const std::vector<int*>& input) {
        for (auto m:input) 
            dataQ.enQueue(m);
        dataQ.inputComplete();
    }

    private:
    lockedQ<int*>& dataQ;
};

class dataSink {

    public:
    dataSink(lockedQ<int*>& dQ):dataQ(dQ) {}

    void extract() 
    {
        while (dataQ.terminateOutput() == false) 
            out.push_back(dataQ.deQueue());
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
    std::vector<std::vector<int*>> rows = {{new int(1), new int(2)},{}};
    int serial = 0;
    for (auto m:rows) {
        lockedQ<int*> lq("intQ");
        dataSource ds(lq);
        dataSink dsk(lq);
        std::thread inTh = std::thread(&dataSource::insert, &ds, m);
        std::thread outTh = std::thread(&dataSink::extract, &dsk);
        
        inTh.join();
        outTh.join();

        std::vector<int*> r = dsk.getReturn();
        if (r == m) 
            std::cout << "Test " << ++serial << " passed" << std::endl;
        else
            std::cout << "Test " << ++serial << " failed" << std::endl;
    }
}

