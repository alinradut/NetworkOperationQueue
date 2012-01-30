//
//  NetworkOperationQueue.cpp
//  NOQ
//
//  Created by Clawoo on 1/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "NetworkOperationQueue.h"

NetworkOperationQueue* NetworkOperationQueue::_instance = NULL;

NetworkOperationQueue *NetworkOperationQueue::sharedInstance()
{
    if (!_instance)
    {
        NetworkOperationQueue::_instance = new NetworkOperationQueue;
    }
    return _instance;
}

void NetworkOperationQueue::addOperation(NetworkOperation *op)
{
    cout << "adding operation " << op << endl;
    _queued.push_back(op);
}

void NetworkOperationQueue::cancelOperationsForDelegate(NetworkOperationDelegate *delegate)
{
    vector<NetworkOperation *>::iterator it;

    it = _inOperation.begin();
    while (it != _inOperation.end()) 
    {
        if ( (* it)->getDelegate() == delegate )
        {
            (* it)->stop();
        }
        else
        {
            it++;
        }
    }
}

void NetworkOperationQueue::update()
{
    vector<NetworkOperation *>::iterator it;
    
    int finished = 0;
    int failed = 0;
    int canceled = 0;
    int started = 0;
    
    it = _inOperation.begin();
    while (it != _inOperation.end()) 
    {
        NetworkOperation *op = (* it);

        if ( op->getStatus() == NetworkOperationStatusFinished )
        {
            if (op->getDelegate() != NULL)
            {
                op->getDelegate()->operationDidFinish(op);
            }
            
            cout << "operation finished: " << op << endl;
            
            delete op;
            
            it = _inOperation.erase(it);
            finished++;
        }
        else if ( op->getStatus() == NetworkOperationStatusFailed )
        {
            if (op->getDelegate() != NULL)
            {
                op->getDelegate()->operationDidFail(op);
            }
            cout << "operation failed: " << op << endl;
            delete op;
            
            it = _inOperation.erase(it);
            failed++;
        }
        else if ( op->getStatus() == NetworkOperationStatusCanceled )
        {
            
            cout << "operation canceled: " << op << endl;
            delete op;
            it = _inOperation.erase(it);
            canceled++;
        }
        else
        {
            it++;
        }
    }
    
    it = _queued.begin();
    while (it != _queued.end()) {
        
        if ( _inOperation.size() >= _maxConcurrentOperations )
        {
            break;
        }
        NetworkOperation *op = (* it);
        
        _inOperation.push_back(op);
        it = _queued.erase(it);
    }
    
    for(vector<NetworkOperation *>::iterator it = _inOperation.begin(); it != _inOperation.end(); ++it) 
    {
        NetworkOperation *op = (* it);
        if ( op->getStatus() == NetworkOperationStatusReady )
        {
            op->start();
            started++;
        }
    }
    cout << "[" + string(_inOperation.size(), '*') + string(_maxConcurrentOperations - _inOperation.size(), '.') + "] (+ " << _queued.size() << ")" << endl;
    cout << finished << " finished " << failed << " failed " << canceled << " canceled " << started << " started " << endl;
}