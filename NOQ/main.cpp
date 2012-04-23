//
//  main.cpp
//  NOQ
//
//  Created by Clawoo on 1/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "NetworkOperation.h"
#include "NetworkOperationQueue.h"

class MyClass : public NetworkOperationDelegate
{
    public :
    virtual void operationDidFinish(NetworkOperation *operation)
    {
        //std::cout << "operationDidFinish self: " << this << " operation: " << operation << std::endl;
    }
    
    virtual void operationDidFail(NetworkOperation *operation)
    {
        //std::cout << "operationDidFinish self: " << this << " operation: " << operation << std::endl;
    }

    MyClass() : NetworkOperationDelegate()
    {
    }
    
    ~MyClass()
    {
        
    }
};

int main (int argc, const char * argv[])
{

    // insert code here...
    NetworkOperationQueue *queue = new NetworkOperationQueue();
    srand((unsigned)time(NULL));
    while (1) {
        int opsToAdd = rand() % 15;
        for (int i=0; i<opsToAdd; i++) {
            NetworkOperation *op = new NetworkOperation("http://www.example.org");
            op->setDelegate(new MyClass());
            queue->addOperation(op);
        }
        sleep(1);
        queue->update();
    }
    return 0;
}

