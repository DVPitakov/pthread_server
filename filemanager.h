#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <sys/stat.h>
#include <QDebug>
#include <string.h>
#include <stdlib.h>
#include <values.h>
#include <time.h>
#include <map>
#include <pthread.h>

using namespace std;


struct FileData {
    unsigned long length;
    char * data;
    tm * date;
    bool success;
    FileData() {

    }

    FileData(const FileData & other) {
        this->data = other.data;
        this->date = other.date;
        this->length = other.length;
        this->success = other.success;
    }

    ~FileData() {
        /*if (data != NULL) {
            free(data);
        }
        if (date != NULL) {
            free(date);
        }*/
    }
};

static pthread_mutex_t mutex_init;
static pthread_mutex_t mutex_write;

void mutexs_init() {
    pthread_mutex_init(&mutex_init, NULL);
    pthread_mutex_init(&mutex_write, NULL);
}

/*class FileCache {
private:

    static FileCache * instance;
    static map <std::string, FileData*> myMap;

    FileCache() {

    }

public:
    ~FileCache() {

    }

    static FileCache * getInstance() {
        if (instance == NULL) {
            pthread_mutex_lock(&mutex_init);
            if (instance == NULL) {
               instance = new FileCache();
            }
            pthread_mutex_unlock(&mutex_init);
        }
        return instance;
    }

    FileData * getFileFromCache(std::string path) {
        FileData * result;
        //
        //return NULL;
        //
        if ((FileCache::myMap).find(path) != (FileCache::myMap).end()) {
            result = (FileCache::myMap).at(path);
        }
        else {
            result =  NULL;
        }
        return result;
    }

    void saveFileToCache(std::string str, FileData* data) {
        //
        //return;
        //
        if((FileCache::myMap).find(str) == (FileCache::myMap).end()) {
            pthread_mutex_lock(&mutex_write);
            if((FileCache::myMap).find(str) == (FileCache::myMap).end()) {
                myMap.insert(pair<std::string,FileData*>(str, data));
            }
            pthread_mutex_unlock(&mutex_write);
        }
    }

};

FileCache * FileCache::instance = NULL;
map<std::string, FileData*> FileCache::myMap;
*/

FileData* getFile(char * path, char * dir_path, bool readFile = true) {

    FileData* resultFile = new FileData;
    FILE *fp;
    int path_len = strlen(path);
    int dir_path_len = strlen(dir_path);
    if(path_len + dir_path_len >= 512) {
        return NULL;
    }

    char name[512];
    memcpy(name,  dir_path, strlen(dir_path));
    memcpy(name + strlen(dir_path), path, path_len + 1);
//    std::string st = std::string(name);
//    FileData * fileDataFromCahe = (FileCache::getInstance())->getFileFromCache(st);
//    if(fileDataFromCahe != NULL) {
//        return fileDataFromCahe;
//    }
//    else {
        fp = fopen(name, "rb");
        if (fp != NULL && (strchr(path, '.') >= 0)) {

            struct stat myStat;
            stat(name, &myStat);
            char * buf = NULL;

            if (readFile == true) {
                buf = (char*)malloc(myStat.st_size + 25);
                fread(buf, 1, myStat.st_size, fp);
                fclose(fp);
            }

            resultFile->data = buf;
            resultFile->length = myStat.st_size;
            resultFile->date = gmtime((time_t*)(&myStat.st_mtim.tv_sec));
            resultFile->success = true;
//          string outSt = std::string(name);
//            if (readFile == true) {
//                (FileCache::getInstance())->saveFileToCache(outSt, resultFile);
//            }
        }
        else {

            resultFile->data = NULL;
            resultFile->date = NULL;
            resultFile->length = 0;
            resultFile->success = false;
        }

        return resultFile;
//    }


}

#endif // FILEMANAGER_H
