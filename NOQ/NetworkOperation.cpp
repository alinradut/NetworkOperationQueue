//
//  NetworkOperation.cpp
//  NOQ
//
//  Created by Clawoo on 1/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "NetworkOperation.h"

void NetworkOperation::start()
{
    assert(_status == NetworkOperationStatusReady);
    _status = NetworkOperationStatusRunning;
    pthread_create(&_thread, 0, &NetworkOperation::startThread, this);
}

void NetworkOperation::stop()
{
    if (_status == NetworkOperationStatusFinished)
    {
        printf("canceling an already finished operation: %p", this);
    }
    _status = NetworkOperationStatusCanceled;
    printf("canceling %p\n", this);
    pthread_join(_thread, NULL);
}

NetworkOperationStatus NetworkOperation::getStatus()
{
    return _status;
}

NetworkOperationDelegate *NetworkOperation::getDelegate()
{
    return _delegate;
}

void NetworkOperation::setDelegate(NetworkOperationDelegate *delegate)
{
    _delegate = delegate;
}

const string NetworkOperation::getResponse()
{
    return _buffer;
}

int NetworkOperation::writeProxy(char *data, 
                                 size_t size, 
                                 size_t num, 
                                 void *operation)
{
    reinterpret_cast<NetworkOperation *>(operation)->process(data, size * num);
    return (int)(size * num);
}

int NetworkOperation::progressProxy(void *operation, 
                                    double dltotal, 
                                    double dlnow, 
                                    double ultotal, 
                                    double ulnow)
{
    NetworkOperationStatus status = reinterpret_cast<NetworkOperation *>(operation)->getStatus();
    if ( status == NetworkOperationStatusCanceled )
    {
        // stop the call by returning a non-zero value
        return -1;
    }
    return 0;
}

void * NetworkOperation::startThread(void *operation)
{
    reinterpret_cast<NetworkOperation *>(operation)->execute();
    return NULL;
}

void NetworkOperation::process(char *data, size_t size)
{
    _buffer.append(data, size);
    
    printf("%s\n", data);
    fflush(stdout);
}

void NetworkOperation::execute()
{
    pthread_mutex_lock(&_mutex);
    CURL *curl;
    CURLcode code;

    curl = curl_easy_init();
    if(curl)
    {
        if (_httpMethod == "POST")
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS,this->parameterString());
        }
        else if (_httpMethod == "DELETE")
        {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else if (_params.size())
        {
            _url.append("?").append(this->parameterString());
        }
        curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeProxy);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressProxy);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
        code = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    if (_status == NetworkOperationStatusRunning)
    {
        _status = NetworkOperationStatusFinished;
        _responseCode = code;
    }
    
    pthread_mutex_unlock(&_mutex);
}


string NetworkOperation::urlEncode(const string &c)
{
    string escaped="";
    size_t max = c.length();
    for(int i=0; i<max; i++)
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
            (65 <= c[i] && c[i] <= 90) ||//abc...xyz
            (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
            (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'') //~!*()'
            )
        {
            escaped.append( &c[i], 1);
        }
        else
        {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
    return escaped;
}

const char* NetworkOperation::parameterString()
{
    string paramString="";
    
    map<string, string>::iterator it;
    for(it = _params.begin(); it != _params.end(); it++)
    {
        paramString+= it->first + "=" + urlEncode(it->second)+ "&";
    }
    return paramString.c_str();
}


string NetworkOperation::char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;
    
    string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}