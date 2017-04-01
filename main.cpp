#include <iostream>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <QDebug>
#include <variables.h>
#include <html.h>

using namespace std;

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
    TaskTurnElement() {
        task = NULL;
        nextTaskElement = NULL;
    }
};

struct TaskTurn {
    //workerThread
    TaskTurnElement * firstTaskTurnElement;
    //workerThread
    TaskTurnElement * lastMakedTurnElement;

    //mainThread
    TaskTurnElement * lastTaskTurnElement;

    //mainThread
    TaskTurn() {
        firstTaskTurnElement = new TaskTurnElement();
        lastTaskTurnElement = firstTaskTurnElement;
        lastMakedTurnElement = NULL;
    }

    //mainThread
    void pushLast(Task * task){
        qDebug() << "pushLast start";
        TaskTurnElement * newTaskTurnElement = new TaskTurnElement();
        newTaskTurnElement->nextTaskElement = NULL;
        newTaskTurnElement->task = task;
        lastTaskTurnElement->nextTaskElement = newTaskTurnElement;
        lastTaskTurnElement = newTaskTurnElement;
        qDebug() << "pushLast end";
    }

    //workerThread
    Task * popFirst(){
        if (lastMakedTurnElement != lastTaskTurnElement) {
            Task * result = firstTaskTurnElement->task;
            if (lastMakedTurnElement == firstTaskTurnElement) {
                result = 0;
            }
            else {
               lastMakedTurnElement = firstTaskTurnElement;
            }
            if(firstTaskTurnElement->nextTaskElement != NULL) {
                firstTaskTurnElement = firstTaskTurnElement->nextTaskElement;
            }
            return result;
        }
        return NULL;
    }

};


int workers_count = 0;
int current_worker = 0;

TaskTurn * taskTurns;
pthread_t threads[8];

//Запускает цикл обработки выполняется в потоке воркера
void workerLive(TaskTurn * taskTurn) {
    Task * task = NULL;
    while(true) {
        while(task == NULL) {
            task = taskTurn->popFirst();
        }
        char buf[8192];
        int i = recv(task->clientDescriptor, buf, sizeof buf, 0);
        RequestData requestData(buf, i);

        ResponseData responseData(&requestData);
        HTML * data = responseData.getHTTPResponse();
        send(task->clientDescriptor, data->data, data->len, 0);
        close(task->clientDescriptor);
        free(task);
        task = NULL;
    }
}


//Выполняется в потоке воркера
//

void * runWorker(void * taskTurn) {
    workerLive((TaskTurn*)taskTurn);
}

//Обьявляет воркеров и запускаетих
//count - количество воркеров
void initWorkers(int count) {
    workers_count = count;
    taskTurns = new TaskTurn[count];

    while(count != 0) {
        pthread_create(threads, NULL, runWorker, taskTurns + --count);
    }
}

//main Thread
void addTask(Task* task) {
    qDebug() << "add task runned";
    taskTurns[current_worker].pushLast(task);
    qDebug() << "task was pushed";
    qDebug() << "in thread turn num: " << current_worker;
    current_worker = (current_worker + 1) % workers_count;
    qDebug() << "new current worker: " << current_worker;
}

sockaddr_in serverAddr;

//mainThread
void mainThreadLoop(const unsigned short port) {

    qDebug() << "mainThreadLoop runed";

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);


    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(mySocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(mySocket, 256);
    unsigned int clientSize = sizeof(sockaddr_in);

    while (true) {
        sockaddr_in clientAddr;
        qDebug() << "listening port: " << port;
        int clientDescriptor = accept(mySocket, (sockaddr*)&clientAddr, &clientSize);
        qDebug() << "new client";
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
    initWorkers(1);
    mainThreadLoop(3156);
    return 0;
}
