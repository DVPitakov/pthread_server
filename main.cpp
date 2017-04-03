#include <iostream>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <QDebug>
#include <variables.h>
#include <html.h>
#include <semaphore.h>


using namespace std;


pthread_cond_t emptyQueue;

int MAX_REQUEST_LEN = 4096;
class Task {
private:
    int clientDescriptor;
    int usedLen;
    int freeLen;
    char * requestData;
    char * freePointer;

public:
    Task(int clientDescriptor) {
        this->clientDescriptor = clientDescriptor;
        requestData = (char*)malloc(MAX_REQUEST_LEN);
        usedLen = 0;
        freeLen = MAX_REQUEST_LEN;
        freePointer = requestData;

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

    }

    int getFreeLen() {
        return freeLen;

    }

    void clear() {
        freePointer = requestData;
        usedLen = 0;
        freeLen = MAX_REQUEST_LEN;

    }

    ~Task() {
        free(requestData);

    }

};

struct TaskTurnElement {
    Task * task;
    TaskTurnElement * nextTaskElement;
    TaskTurnElement(Task * task) {
        this->task = task;
        nextTaskElement = NULL;

    }
};

int EPOLL_QUEUE_LEN = 1024;
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
        ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
        ev.data.ptr = (void*)task;
        int res = epoll_ctl(epfd, EPOLL_CTL_ADD, task->getClientDescriptor(), &ev);

    }

    ~TaskTurn() {

    }

};


int workers_count = 0;
int current_worker = 0;

TaskTurn * taskTurns;
pthread_t threads[64];
sem_t myMutexArr[64];

void addTask(Task* task) {
    taskTurns[current_worker].push(task);
    current_worker = (current_worker + 1) % workers_count;
    qDebug() << "new current worker: " << current_worker;

}

void workerLive(TaskTurn * taskTurn) {
    Task * curTask;
    epoll_event events[EPOLL_QUEUE_LEN];
    int nfds;
    while(true) {
         nfds = epoll_wait(taskTurn->getEpfd(), events, MAX_EPOLL_EVENTS_PER_RUN, RUN_TIMEOUT);
         for (int i = 0; i < nfds; i++) {
             curTask = (Task*)(events[i].data.ptr);
             int readedBytesCount = recv(curTask->getClientDescriptor(), curTask->getRequestData(), curTask->getFreeLen(), 0);
             curTask->move(readedBytesCount );
             if (curTask->isReady()) {
                 RequestData requestData(curTask->getRequestData());
                 if (requestData.isValid) {
                     ResponseData responseData(&requestData);
                     send(curTask->getClientDescriptor(), responseData.header, responseData.headerLen, 0);
                     if(responseData.dataLen > 0) {
                         send(curTask->getClientDescriptor(), responseData.data, responseData.dataLen, 0);
                     }
                 }
                 close(curTask->getClientDescriptor());
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

sockaddr_in serverAddr;

void mainThreadLoop(const unsigned short port) {

    qDebug() << "mainThreadLoop runed";
    qDebug() << "port: " << port;

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);


    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(mySocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(mySocket, 256);
    unsigned int clientSize = sizeof(sockaddr_in);

    while (true) {
        sockaddr_in clientAddr;
        int clientDescriptor = accept(mySocket, (sockaddr*)&clientAddr, &clientSize);
        Task * newTask = new Task(clientDescriptor);
        addTask(newTask);
    }

    close(mySocket);
    return;

}

//mainThread
int main(int argc, char *argv[])
{
    qDebug() << "main functon runned";
    initWorkers(6);
    mainThreadLoop(3211);
    return 0;
}
