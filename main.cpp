#include <iostream>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <arpa/inet.h>
#include <unistd.h>
#include "html.h"

#include <fcntl.h>


using namespace std;



int MAX_REQUEST_LEN = 4096;
class Task {
private:
    int clientDescriptor;
    int usedLen;
    int freeLen;
    RequestData * pRequest;
    ResponseData * pResponse;
    char * requestData;
    char * freePointer;
    bool socketIsClosed;

public:
    Task(int clientDescriptor) {
        pRequest = NULL;
        pResponse = NULL;
        this->clientDescriptor = clientDescriptor;
        requestData = (char*)malloc(MAX_REQUEST_LEN + 1);
        requestData[MAX_REQUEST_LEN] = '\0';
        usedLen = 0;
        freeLen = MAX_REQUEST_LEN;
        freePointer = requestData;

    }

    void setSocketIsClosed() {
        socketIsClosed = true;
    }

    void reqv() {
        move(recv(getClientDescriptor(), getRequestData(), getFreeLen(), 0));
    }

    void openRequest() {
        if (pRequest == NULL) {
            pRequest = new RequestData(requestData);
        }
        else {
            pRequest->init(requestData);
        }

    }

    bool isValidRequest() {
        return pRequest->isValid;

    }

    bool isReady() {
        if (usedLen >= 4
                && (*(freePointer - 1)) == '\n'
                && (*(freePointer - 2)) == '\r'
                && (*(freePointer - 3)) == '\n'
                && (*(freePointer - 4)) == '\r') {
            return true;
        }
        else {
            return false;
        }

    }

    bool sendData() {
        int sended;
        if (pResponse->countHeaderSendedBytes != pResponse->headerLen) {
            sended = send(clientDescriptor, pResponse->header, pResponse->headerLen, 0);
            pResponse->countHeaderSendedBytes += sended;
            if (sended == -1) {
                return true;
            }
        }
        if (pResponse->dataLen > 0 && pResponse->dataLen != pResponse->countDataSendedBytes) {
            sended = send(clientDescriptor, pResponse->data, pResponse->dataLen, 0);
            pResponse->countDataSendedBytes += sended;
            if (sended == -1) {
                return true;
            }
        }
        return (((pResponse->dataLen) == (pResponse->countDataSendedBytes))
                && ((pResponse->countHeaderSendedBytes) == (pResponse->headerLen)));
    }

    bool isKeepAlive() {
        return pRequest->keepAlive;

    }

    int getClientDescriptor() {
        return clientDescriptor;

    }

    char * getRequestData() {
        return requestData;

    }

    char * getFreePointer() {
        return freePointer;

    }

    void move(int len) {
        freeLen -= len;
        usedLen += len;
        freePointer += len;
        *freePointer = '\0';

    }

    int getFreeLen() {
        return freeLen;

    }

    void makeResponse() {
        if (pResponse == NULL) {
              pResponse = new ResponseData(pRequest);
        }
        else {
            pResponse->getHTTPResponse();
        }

    }

    void clear() {
        pResponse->clear();
        pRequest->clear();
        freePointer = requestData;
        usedLen = 0;
        freeLen = MAX_REQUEST_LEN;
    }

    ~Task() {
        if (pRequest != NULL) delete pRequest;
        pRequest = NULL;
        delete pResponse;
         if (pResponse != NULL) pResponse = NULL;
        if (!socketIsClosed) close(clientDescriptor);
        if (requestData != NULL) free(requestData);
        requestData = NULL;

    }

};

int EPOLL_QUEUE_LEN = 65536;
int MAX_EPOLL_EVENTS_PER_RUN = 1;
int RUN_TIMEOUT = 10;

class TaskTurn {
private:
    int epfd;

public:
    TaskTurn() {
        epfd = epoll_create(EPOLL_QUEUE_LEN);

    }

    int getEpfd() {
        return epfd;

    }

    void push(Task * task){
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        ev.data.ptr = (void*)task;
        int res = epoll_ctl(epfd, EPOLL_CTL_ADD, task->getClientDescriptor(), &ev);

    }

    ~TaskTurn() {

    }

};


int workers_count = 0;
int current_worker = 0;
sockaddr_in serverAddr;

TaskTurn * taskTurns;
pthread_t threads[256];

void addTask(Task* task) {
    taskTurns[current_worker].push(task);
    current_worker = (current_worker + 1) % workers_count;

}

void workerLive(TaskTurn * taskTurn) {
    Task * curTask;
    epoll_event events[EPOLL_QUEUE_LEN];
    int nfds;
    while(true) {
         nfds = epoll_wait(taskTurn->getEpfd(), events, MAX_EPOLL_EVENTS_PER_RUN, RUN_TIMEOUT);

         for (int i = 0; i < nfds; i++) {
             curTask = (Task*)(events[i].data.ptr);
/*             if ((events[i].events & (EPOLLHUP + EPOLLERR))) {
                 curTask->setSocketIsClosed();
                 delete curTask;
                 continue;
             }
             */

             if(events[i].events & EPOLLHUP) {

                 delete curTask;
                 continue;
             }


             if(events[i].events & EPOLLERR) {
                 delete curTask;
                 continue;
             }

             if (events[i].events & EPOLLIN) {
                if(!(curTask->isReady())) {
                    curTask->reqv();
                }
             }
             if (curTask->isReady()) {
                 curTask->openRequest();
                 if (curTask->isValidRequest()) {
                     curTask->makeResponse();
                     if (curTask->sendData()) {
                         if(curTask->isKeepAlive()) {
                             curTask->clear();
                         }
                         else {
                             delete curTask;
                         }
                     }
                     else {
                         std::cout << "ogogo";
                     }
                 }
                 else {
                     delete curTask;
                 }
             }

        }
    }

}

void * runWorker(void * taskTurn) {
    workerLive((TaskTurn*)taskTurn);

}

void initWorkers(int count) {
    workers_count = count;
    taskTurns = new TaskTurn[count];
    while(count != 0) {
        pthread_create(threads, NULL, runWorker, taskTurns + --count);
    }

}

void mainThreadLoop(const unsigned short port) {

    std::cout << "openning socket on port: " << port << std::endl;

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);


    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(mySocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        std::cout << "Error in bind function" << std::endl;
        return;
    }
    if (listen(mySocket, 4096) != 0) {
        std::cout << "Error in listen function" << std::endl;
        return;
    }
    unsigned int clientSize = sizeof(sockaddr_in);
     std::cout << "success" << std::endl;
    while (true) {
        sockaddr_in clientAddr;
        int clientDescriptor = accept(mySocket, (sockaddr*)&clientAddr, &clientSize);

        fcntl(clientDescriptor, F_SETFL);
        Task * newTask = new Task(clientDescriptor);
        addTask(newTask);
    }

    close(mySocket);
    return;

}

unsigned short port = 80;
unsigned short thread_count = 4;

int setSettings(int argc, char **argv) {
    int i = 1;
    if (argc >= 2 && strcmp(argv[i], "-h") == 0) {
                std::cout << "-r  " << "suite directory"<< std::endl;
                std::cout << "-d  " << "3xx 4xx ... error html pages dirrectory" << std::endl;
                std::cout << "-c  " << "worker threads count" <<std::endl;
                std::cout << "-p  " << "port" << std::endl;
                return -1;
    }
    while ((argc - i) > 1) {
        if (strcmp(argv[i], "-r") == 0) {
            root_path = (char*)malloc(strlen(argv[i + 1]) + 1);
            strcpy(root_path, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-d") == 0) {
            server_path = (char*)malloc(strlen(argv[i + 1]) + 1);
            strcpy(server_path, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-c") == 0) {
            int res = atoi(argv[i + 1]);
            if (res > 0 && res <= 256) {
                thread_count = res;
            }
        }
        else if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
        }

        i += 2;
    }
    return 0;

}

int main(int argc, char *argv[])
{
    if (setSettings(argc, argv) == 0) {
        initWorkers(thread_count);
        mainThreadLoop(port);
    }
    return 0;

}
