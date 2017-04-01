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
    ~FileData() {
        /*if (data != NULL) {
            free(data);
        }
        if (date != NULL) {
            free(date);
        }*/
    }
};


FileData getFile(char * path, char * root_path, bool readFile = true) {
    FileData resultFile;
    FILE *fp;
    int path_len = strlen(path);
    if (*(path + path_len - 1) == '/') {
        char * newString = (char*)malloc(path_len + 11);
        char ind[] = "index.html";
        memcpy(newString, path, path_len);
        memcpy(newString + path_len, ind, 11);
        path = newString;
        path_len += 10;
    }
    char name[256];
    memcpy(name, root_path, strlen(root_path));
    memcpy(name + strlen(root_path), path, path_len + 1);
    struct stat myStat;
    fp = fopen(name, "rb");
    stat(name, &myStat);
    char * buf = NULL;

    if (fp != NULL && (strchr(path, '.') >= 0)) {

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
