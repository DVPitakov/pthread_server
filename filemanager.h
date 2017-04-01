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

FileData getFile(char * path, char * dir_path, bool readFile = true) {
    FileData resultFile;
    FILE *fp;
    int path_len = strlen(path);
    char name[256];
    memcpy(name,  dir_path, strlen( dir_path));
    memcpy(name + strlen(dir_path), path, path_len + 1);
    fp = fopen(name, "rb");
    if (fp != NULL && (strchr(path, '.') >= 0)) {

        struct stat myStat;
        stat(name, &myStat);
        char * buf = NULL;

        if (readFile == true) {
            buf = (char*)malloc(myStat.st_size + 10);
            fread(buf, 1, myStat.st_size, fp);
            fclose(fp);
        }

        resultFile.data = buf;
        resultFile.length = myStat.st_size;
        resultFile.date = gmtime((time_t*)(&myStat.st_mtim.tv_sec));
        resultFile.success = true;
    }
    else {

        resultFile.data = NULL;
        resultFile.date = NULL;
        resultFile.length = 0;
        resultFile.success = false;
    }

      return resultFile;

}

#endif // FILEMANAGER_H
