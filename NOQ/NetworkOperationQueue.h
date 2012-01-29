//
//  NetworkOperationQueue.h
//  NOQ
//
//  Created by Clawoo on 1/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef NOQ_NetworkOperationQueue_h
#define NOQ_NetworkOperationQueue_h

#include <iostream>
#include <vector>
#include "NetworkOperation.h"

using namespace std;

class NetworkOperationQueue 
{
public:
    /**
     Constructor
     */
    NetworkOperationQueue() : _maxConcurrentOperations(10) {};
    ~NetworkOperationQueue();
    
    /**
     Singleton method that returns a globally shared instance of the class
     */
    static NetworkOperationQueue* sharedInstance();
    
    /**
     Adds a new operation to the queue
     @param op  A ready to use instance of the operation
     */
    void addOperation(NetworkOperation *op);
    
    /**
     Cancels all the operations for a given delegate
     @param delegate An object that implments the delegate protocol
     */
    void cancelOperationsForDelegate(NetworkOperationDelegate *delegate);
    
    /**
     Step method that should be called periodically (not necessarily each frame, 
     but two - three times per second is a good choice. This method will cause 
     the completed operations to notify their delegates on the main thread and
     kickstart the next operations.
     */
    void update();
    
    /**
     Setter for the maximum number of concurrent operations. If more operations 
     are added they are queued.Default 10.
     @param maxOps  Maximum number of concurrent operations.
     */
    void setMaxConcurrentOperations(int maxOps);
    
    /** 
     Getter for the maximum number of concurrent operations
     */
    int getMaxConcurrentOperations();
    
private:
    
    // maximum numbe of concurrent operations
    int _maxConcurrentOperations;
    
    // operations currently being executed
    vector<NetworkOperation *> _inOperation;
    
    // operations queued for execution
    vector<NetworkOperation *> _queued;
    
    // static instance of the current class, used as a singleton
    static NetworkOperationQueue *_instance;
};

#endif
