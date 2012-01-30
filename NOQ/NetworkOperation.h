//
//  NetworkOperation.h
//  NOQ
//
//  Created by Clawoo on 1/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef NOQ_NetworkOperation_h
#define NOQ_NetworkOperation_h

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <curl/curl.h>
#include <assert.h>
#include <string>
#include <map>

using namespace std;

typedef enum  
{
    NetworkOperationStatusReady = 0,
    NetworkOperationStatusRunning,
    NetworkOperationStatusFinished,
    NetworkOperationStatusCanceled,
    NetworkOperationStatusFailed
} NetworkOperationStatus;

class NetworkOperationDelegate;

class NetworkOperation 
{
    
public:
    /**
     Constructor
     
     @param url         The url to call
     @param parameters  A map<string, string> of parameters to send with the 
     request. URL encoding is not necessary as it will be performed within
     the class
     @param httpMethod  HTTP method to use: GET, POST or DELETE (default GET)
     */
    NetworkOperation(const string url, map<string,string> parameters, const string httpMethod) : 
    _url(url),
    _params(parameters),
    _httpMethod(httpMethod)
    {
        this->init();
    }
    
    /**
     Constructor
     
     @param url         The url to call
     */
    NetworkOperation(const string url) : 
    _url(url),
    _params(map<string,string>()),
    _httpMethod("GET")
    {
        this->init();
    }
    
    /**
     Destructor
     */
    ~NetworkOperation()
    {
        pthread_mutex_destroy(&_mutex);
    }
    
    /**
     Starts the operation by creating a new thread
     */
    void start();
    
    /**
     Sends the cancel "signal" to the curl operation and sets the current 
     operation status to NetworkOperationStatusCanceled
     */
    void stop();
    
    /**
     Getter for the current operation status
     */
    NetworkOperationStatus getStatus();
    
    /**
     Getter for the current delegate
     */
    NetworkOperationDelegate* getDelegate();    

    /**
     Setter for the delegate to be called when the operation completes
     */
    void setDelegate(NetworkOperationDelegate *delegate);

    /**
     The actual server response string
     */
    const string getResponse();
    
    /**
     The server response code
     */
    int getResponseCode();

protected:
    
    /**
     Initializes the class members and initializes the thread mutex
     **/
    void init() 
    {
        pthread_mutex_init(&_mutex, NULL);
        _status = NetworkOperationStatusReady;
        _delegate = NULL;
        _thread = NULL;
    }
    
    /**
     Static callback that is called by libcurl every time there is new data
     to be saved from the server.
     @param data       data to be saved
     @param size       byte size
     @param num        number of bytes
     @param operation  reference to the operation that received the data
     @return the number of bytes the method processed (normally size * num)
     */
    static int writeProxy(char *data, size_t size, size_t num, void *operation);

    /**
     Static callback that is periodically called by libcurl 
     @param operation  reference to the operation that received the data
     @return A non-zero value if the operation was canceled by the operation queue,
        or zero if the curl operation should continue
     */
    static int progressProxy(void *operation,
                             double dltotal,
                             double dlnow,
                             double ultotal,
                             double ulnow);
    
private:
    
    /** 
     Static method that is called by pthread_create as the entry point
     for the new thread.
     @param operation   The operation that we need to start
     */
    static void *startThread(void *operation);
    
    /**
     Processes a batch of data when called from writeProxy
     @param data    The byte array of the received data
     @param size    The size of the byte array
     */
    void process(char *data, size_t size);    
    
    /**
     Starts the operation when called from startThread
     */
    void execute();
    
    /**
     Joins the items in the @_params variable while URL encoding the values
     @return    Ready to use parameter string
     */
    const char* parameterString();
    
    /**
     Static method that encodes a string for viable GET/POST usage
     @param c   The string to encode
     @return The encoded string
     */
    static string urlEncode(const string &c);
    
    /**
     Returns the hex value of a char
     @param dec The character to be encoded
     @return The hex value (for example ff)
     */
    static string char2hex( char dec );   
    
    
    // keeps track of the current operation status
    volatile NetworkOperationStatus _status;
    
    // the delegate to be notified when the operation completes
    NetworkOperationDelegate *_delegate;

    // mutex used for thread synchronization
    pthread_mutex_t _mutex;
    
    // thread handle
    pthread_t _thread;
    
    // stores the HTTP response code for the curl operation
    long _responseCode;
    
    // url to open
    string _url;
    
    // the call parameters
    map<string, string> _params;
    
    // buffer that stores the downloaded data
    string _buffer;
    
    // GET, POST or DELETE (default GET)
    string _httpMethod;
};


class NetworkOperationDelegate
{
public:
    virtual void operationDidFinish(NetworkOperation *operation) = 0;
    virtual void operationDidFail(NetworkOperation *operation) = 0;
    virtual ~NetworkOperationDelegate() {};
};

#endif
