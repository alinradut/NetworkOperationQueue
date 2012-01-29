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
    it = _inOperation.begin();
    int finished = 0;
    int canceled = 0;
    while (it != _inOperation.end()) 
    {
        if (rand()%5 == 0)
        {
            (* it)->stop();
        }
        if ( (* it)->getStatus() == NetworkOperationStatusFinished )
        {
            // notify its delegate that the operation has succeeded
            // ...
            //cout << "erasing one op" << endl;
            NetworkOperation *op = (* it);
            if (op->getDelegate() != NULL)
            {
                op->getDelegate()->operationDidFinish(op);
            }
            it = _inOperation.erase(it);
            finished++;
        }
        else if ( (* it)->getStatus() == NetworkOperationStatusCanceled )
        {
            it = _inOperation.erase(it);
            canceled++;
        }
        else
        {
            it++;
        }
    }
    cout << "finished operations: " << finished << endl;
    cout << "canceled operations: " << canceled << endl;
    cout << "active operations: " << _inOperation.size() << " queued: " << _queued.size() << endl;
    
    it = _queued.begin();
    while (it != _queued.end()) {
        if (rand()%5 == 0)
        {
            (* it)->stop();
        }
        if ( _inOperation.size() >= _maxConcurrentOperations )
        {
            //cout << "more than 10 operations in queue" << endl;
            break;
        }
        //cout << "adding one op: " << (* it) << endl;
        _inOperation.push_back((* it));
        it = _queued.erase(it);
    }
    
    for(vector<NetworkOperation *>::iterator it = _inOperation.begin(); it != _inOperation.end(); ++it) 
    {
        if ( (* it)->getStatus() == NetworkOperationStatusReady )
        {
            //cout << "starting one op" << endl;
            (* it)->start();
        }
    }
}