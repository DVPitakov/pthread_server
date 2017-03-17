#include <iostream>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

struct ClientData {
    sockaddr_in addr;
    int clientDescriptor;
    int clientSize;
};

pthread_mutex_t mutex;
int pointer = 0;
ClientData* clients[128] = {0};
int actives[128] = {0};

pthread_t threads[4];


int findActives() {
    int tg = 0;
    bool fnd = 0;
    while(!fnd) {
        while(clients[tg] == NULL && actives[tg] == 0) {
            tg++;
            if(tg >= 128) {
                tg = 0;
            }
        }
        pthread_mutex_lock(&mutex);
        if(actives[tg] == 0) {
            actives[tg] = 1;
            fnd = 1;
        }
        pthread_mutex_unlock(&mutex);
    }
    return tg;
}

void workerLive() {
    while(true) {
        int i = findActives();
        ClientData*clientData = clients[i];
        char buf[] =
                "HTTP/1.1 200 OK\nDate: Wed, 11 Feb 2009 11:20:59 GMT\n\
     Server: Apache\n\
     Content-Language: ru\n\
     Content-Type: text/html; charset=utf-8\n\
     \n\
                <style type=\"text/css\">\
                 .block1 {\
                    width: parent; \
                    background: #fc0;\
                    padding: 5px;\
                    border: solid 1px black;\
                    top: 10px;\
                    left: -70px;\
                }\
                .block2 {\
                    width: parent;\
                    background: #00a;\
                    padding: 5px;\
                    border: solid 1px black;\
                    top: 100px;\
                    left: -70px;\
                }\
                </style>\
                <div class=\"block1\"><h1 align=\"center\">Server are working!</h1></div>\
                <div class=\"block2\"><h3 id=\"target\">Good!</h3></div>";
        send(clientData->clientDescriptor, buf, sizeof(buf), 0);
        close(clientData->clientDescriptor);
        free(clientData);
        clients[i] = 0;
        actives[i] = 0;

    }
}

void* runWorker(void*) {
    workerLive();
}

void initWorkers(int count) {
    while(count != 0) {
        pthread_create(threads, NULL, runWorker, NULL);
        count--;
    }
}



void addClient(ClientData* clientData) {
    while(clients[pointer] == NULL) {
        pointer++;
        if (pointer >= 128) {
            pointer = 0;
        }
    }
    clients[pointer] = clientData;
    actives[pointer] = 0;
}





sockaddr_in serverAddr;
void mainThread() {

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);


    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(mySocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(mySocket, 256);
    unsigned int clientSize = sizeof(sockaddr_in);

    printf("server started");
    while (true) {
        ClientData * clientData = (ClientData *)malloc(sizeof(ClientData));
        sockaddr_in clientAddr;
        int clientDescriptor = accept(mySocket, (sockaddr*)&clientAddr, &clientSize);
        clientData->addr = clientAddr;
        clientData->clientSize = clientSize;
        clientData->clientDescriptor = clientDescriptor;
        addClient(clientData);
    }

    close(mySocket);
    return;
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&mutex, NULL);
    initWorkers(4);
    mainThread();
    return 0;
}
