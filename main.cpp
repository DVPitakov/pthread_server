#include <iostream>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <QDebug>
#include <variables.h>
#include <html.h>
#include <semaphore.h>

using namespace std;


pthread_cond_t emptyQueue;

struct Task {
    sockaddr_in client_addr;
    int clientDescriptor;
    int clientSize;
    int dataSize;
    char * data;
    char * externData;
};

struct TaskTurnElement {
    Task * task;
    TaskTurnElement * nextTaskElement;
    TaskTurnElement(Task * task) {
        this->task = task;
        nextTaskElement = NULL;
    }
};

struct TaskTurn {
    TaskTurnElement * firstTaskTurnElement;
    TaskTurnElement * lastTaskTurnElement;
    sem_t * pmutex;

    //mainThread
    TaskTurn() {
        firstTaskTurnElement = NULL;
        lastTaskTurnElement = NULL;
    }

    //mainThread
    void pushLast(Task * task){
        TaskTurnElement * newTaskTurnElement = new TaskTurnElement(task);
        if(lastTaskTurnElement == NULL) {
            lastTaskTurnElement = newTaskTurnElement;
        }
        else {
            lastTaskTurnElement->nextTaskElement = newTaskTurnElement;
            lastTaskTurnElement = lastTaskTurnElement->nextTaskElement;
        }

        if(firstTaskTurnElement == NULL) {
            firstTaskTurnElement = lastTaskTurnElement;
        }

        sem_post(pmutex);

    }

    //workerThread
    Task * popFirst(){
        sem_wait(pmutex);
        Task * result = firstTaskTurnElement->task;
        firstTaskTurnElement = firstTaskTurnElement->nextTaskElement;
        return result;

    }

};


int workers_count = 0;
int current_worker = 0;

TaskTurn * taskTurns;
pthread_t threads[64];
sem_t myMutexArr[64];

//Запускает цикл обработки выполняется в потоке воркера
void workerLive(TaskTurn * taskTurn) {
    Task * task = NULL;
    while(true) {
        task = taskTurn->popFirst();
        while(task == NULL) {
            qDebug() << "LOCK" << taskTurn->pmutex;
            task = taskTurn->popFirst();
        }
        char buf[8192] = {0};
        int i = recv(task->clientDescriptor, buf, (sizeof buf) - 4, 0);

        RequestData requestData(buf, i);
        if (requestData.isValid) {
            ResponseData responseData(&requestData);
            send(task->clientDescriptor, responseData.header, responseData.headerLen, 0);
            if(responseData.dataLen > 0) {
                send(task->clientDescriptor, responseData.data, responseData.dataLen, 0);
            }
        }
        close(task->clientDescriptor);
        free(task);
        task = NULL;
    }
}

void * runWorker(void * taskTurn) {
    workerLive((TaskTurn*)taskTurn);
}

void initWorkers(int count) {
    workers_count = count;
    taskTurns = new TaskTurn[count];
    for(int i = 0; i < count; i++) {
        sem_init(myMutexArr + i, 0, 0);
        taskTurns[i].pmutex = myMutexArr + i;
    }

    while(count != 0) {
        pthread_create(threads, NULL, runWorker, taskTurns + --count);
    }
}

void addTask(Task* task) {
    taskTurns[current_worker].pushLast(task);
    current_worker = (current_worker + 1) % workers_count;
    qDebug() << "new current worker: " << current_worker;
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
        Task * newTask = new Task();
        newTask->clientDescriptor = clientDescriptor;
        newTask->client_addr = clientAddr;
        newTask->clientSize = clientSize;
        addTask(newTask);
    }

    close(mySocket);
    return;

}

//mainThread
int main(int argc, char *argv[])
{
    qDebug() << "main functon runned";
    initWorkers(8);
    mutexs_init();
    mainThreadLoop(3199);
    return 0;
}
