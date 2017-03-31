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


char MIME_IMAGE_JPEG[]              = "image/jpeg";
char MIME_IMAGE_JPG[]               = "image/pjpeg";
char MIME_IMAGE_PNG[]               = "image/png";
char MIME_IMAGE_GIF[]               = "image/gif";
char MIME_SWF[]                     = "application/x-shockwave-flash";
char MIME_TEXT_HTML[]               = "text/html";
char MIME_TEXT_CSS[]                = "text/css";
char MIME_APPLICATION_JAVASCRIPT[]  = "application/javascript";

char STATUS_200[] = "200 OK";
char STATUS_301[] = "304 Not Modified";

char PROTOCOL[] = "HTTP/1.1";
char SERVER_NAME[] = "PitakovDV";

char HEADER_STATUS_CODE[] = "Status Code: ";
char HEADER_CONTENT_TYPE[] = "Content-Type: ";
char HEADER_CONTENT_LENGTH[] = "Content-Length: ";
char HEADER_SERVER[] = "Server: ";
char HEADER_DATE[] = "Date: ";

struct FileData {
    unsigned long length;

    char * data;
};

char * http_date(tm * time) {
    char * result = (char*)malloc(40);
    const char day_name[][4] =  {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    const char month_name[][4] = {"Jan", "Feb", "Mar", "Apr",
                                 "May", "Jun", "Jul", "Aug",
                                 "Sep", "Oct", "Nov", "Dec"};
    sprintf(result, "%s, %2d %s %4d %2d:%2d:%2d GMT",
            day_name[time->tm_wday], time->tm_mday,
            month_name[time->tm_mon], time->tm_year,
            time->tm_hour, time->tm_min, time->tm_sec);
    return result;
}

FileData getFile(char * path, char * root_path) {
    FileData resultFile;
    FILE *fp;
    FILE *f2p;
    char name[256];
    memcpy(name, root_path, strlen(root_path));
    memcpy(name + strlen(root_path), path, strlen(path) + 1);
    struct stat myStat;
    fp = fopen(name, "rb");
    f2p = fopen("/home/dmitry/debug.txt", "wb");
    stat(name, &myStat);

    if (fp != NULL && (strchr(path, '.') >= 0)) {
        char * buf = (char*)malloc(myStat.st_size + 10);
        fread(buf, 1, myStat.st_size, fp);
        fwrite(buf, 1, myStat.st_size, f2p);
        timespec myTimespec = myStat.st_mtim;
        time_t myTime = (time_t) myTimespec.tv_sec;
        tm* aTm = gmtime(&myTime);

        fclose(fp);
        fclose(f2p);
        resultFile.data = buf;

        resultFile.length = myStat.st_size;
        return resultFile;
    }
    else {
        resultFile.data = NULL;
        resultFile.length = 0;
        return resultFile;
    }
}

FileData getDirectory(char * root_path) {
    char ch[] = "/test2.jpg";
    FileData fileData = getFile(ch, root_path);
}

#endif // FILEMANAGER_H
