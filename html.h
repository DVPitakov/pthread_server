#ifndef HTML_H
#define HTML_H

#include <filemanager.h>

char MIME_IMAGE_JPEG[]              = "image/jpeg";
char MIME_IMAGE_JPG[]               = "image/jpeg";
char MIME_IMAGE_PNG[]               = "image/png";
char MIME_IMAGE_GIF[]               = "image/gif";
char MIME_SWF[]                     = "application/x-shockwave-flash";
char MIME_TEXT_HTML[]               = "text/html";
char MIME_TEXT_CSS[]                = "text/css";
char MIME_APPLICATION_JAVASCRIPT[]  = "application/javascript";

char CONNECTION_KEEP_ALIVE[] = "keep-alive";
char CONNECTION_CLOSE[] = "close";

char STATUS_200[] = "200 OK";
char STATUS_403[] = "403 Forbidden";
char STATUS_404[] = "404 Not Found";
char STATUS_405[] = "405 Method Not Allowed";

char PROTOCOL[] = "HTTP/1.1";
char SERVER_NAME[] = "PitakovDV";

char HEADER_STATUS_CODE[] = "Status Code: ";
char HEADER_CONTENT_TYPE[] = "Content-Type: ";
char HEADER_CONTENT_LENGTH[] = "Content-Length: ";
char HEADER_SERVER[] = "Server: ";
char HEADER_DATE[] = "Date: ";
char HEADER_CONNECTION[] = "Connection: ";

char METHOD_GET[] = "GET";
char METHOD_HEAD[] = "HEAD";
char METHOD_POST[] = "POST";

char root_path[] = "/home/dmitry/http-test-suite/";
char server_path[] = "/home/dmitry";
char NOT_FOUND[] = "/notFound.html";
char FORBIDDEN[] = "/forbidden.html";
char METHOD_NOT_ALLOWED[] = "/MethodNotAllowed.html";

short ctoi(char ch) {
    if(ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    else if(ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    else if(ch >= 'a' && ch <= 'f') {
         return ch - 'a' + 10;
    }
    else {
        return -1;
    }

}

short oxToChar(char * oxCharCode) {
    short result;
    short chh = ctoi(*(oxCharCode));
    short chl = ctoi(*(oxCharCode + 1));
    if (chh >= 0 && chl >= 0) {
       result = (chh << 4)+ chl;
    }
    else {
        result = -1;
    }
    return result;

}

short utf_8(char * dst, char * src) {
    char * srcPointer = strchr(src, '%');
    while(srcPointer != NULL) {
        memcpy(dst, src, srcPointer - src);
        dst += srcPointer - src;
        src = srcPointer + 3;

        if(((*(srcPointer + 1)) != '\0') && ((*(srcPointer + 2)) != '\0')) {
           short res = oxToChar(srcPointer + 1);
           if (res < 0) {
                return -1;
           }
           *dst = (char)res;
           dst += 1;
           srcPointer += 3;
        }
        else {
            return -2;
        }
       srcPointer = strchr(srcPointer, '%');
    }

    memcpy(dst, src, strlen(src) + 1);
    return 0;

}

char * http_date(tm * time) {
    char * result = (char*)malloc(240);
    const char day_name[][4] =  {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char month_name[][4] = {"Jan", "Feb", "Mar", "Apr",
                                 "May", "Jun", "Jul", "Aug",
                                 "Sep", "Oct", "Nov", "Dec"};
    sprintf(result, "%s, %2d %s %4d %2d:%2d:%2d GMT",
            day_name[time->tm_wday], time->tm_mday,
            month_name[time->tm_mon], 1900 + time->tm_year,
            time->tm_hour, time->tm_min, time->tm_sec);
    return result;
}

struct URI {
    char * path;
    char * query;
    char * data;
    int dataLen;
    bool lastJ;
    URI(char * uriString) {


        bool error = true;
        int len = strlen(uriString);

        char * strEnd =  uriString + len;

        char * pathBegin = NULL;
        char * pathEnd = strEnd;

        char * queryBegin = NULL;
        char * queryEnd = strEnd;

        pathBegin = strchr(uriString, '/');
        if (pathBegin != NULL) {
            queryBegin = strchr(uriString, '?');
            if(queryBegin != NULL) {
                pathEnd = queryBegin;
            }

            dataLen = len + 25;
            data = (char*)malloc(dataLen);
            path = data;
            int pathStrLen = 0;
            memcpy(path, pathBegin, pathEnd - pathBegin);
            if (*(pathEnd - 1) == '/') {
                lastJ = true;
               char ind[] = "index.html";
               pathStrLen = sizeof(ind) - 1 + (pathEnd - pathBegin);
               memcpy(path + (pathEnd - pathBegin), ind, sizeof(ind));
            }
            else {
                lastJ = false;
                pathStrLen = (pathEnd - pathBegin);
            }
            path[pathStrLen] = '\0';

            if(queryBegin != NULL) {
                query = data + pathStrLen + 1;
                memcpy(query, queryBegin, queryEnd - queryBegin);
                query[queryEnd - queryBegin] = '\0';
            }

            char * buf = (char*)malloc(pathStrLen + 25);
            if (utf_8(buf, path) >= 0) {
                memcpy(path, buf, strlen(buf) + 1);
            }
            free(buf);
            error = false;
        }
        else {
            error = true;
        }

    }
    URI(const URI & otherURI) {
        this->dataLen = otherURI.dataLen;
        this->data = (char*)malloc(this->dataLen);
        memcpy(this->data, otherURI.data, this->dataLen);
        this->path = this->data;

        if (otherURI.query != NULL) {
             this->query = this->data + (otherURI.query - otherURI.data);
        }
        else {
            this->query = NULL;
        }
    }

    ~URI() {
       free(data);
    }
};

struct RequestData {
    char * httpRequest;
    char * uriString;
    char * protocol;
    URI * uri;
    char * method;
    char ** headers;
    bool isValid;

    void saveStr(char ** saveWhere, char * begin, char * end) {
        *saveWhere = (char*)malloc(end - begin + 20);
        if (*saveWhere) {
            memcpy(*saveWhere, begin, end - begin);
            (*saveWhere)[end - begin] = '\0';
        }
        else {
            qDebug() << "can not get memory";
        }

    }

    RequestData(char * httpRequest) {
        //headers = (char**)malloc(32);
        isValid = true;
        uri = NULL;
        protocol = NULL;
        method = NULL;
        uriString = NULL;

        this->httpRequest = httpRequest;
        char * oldPos = httpRequest;
        char * newPos = httpRequest;
        newPos = strchr(newPos, ' ');
        if(newPos == NULL) {
            isValid = false;
            return;
        }
        saveStr(&method, oldPos, newPos);

        newPos++;
        oldPos = newPos;
        newPos = strchr(newPos, ' ');
        if(newPos == NULL) {
            isValid = false;
            return;
        }
        saveStr(&uriString, oldPos, newPos);

        newPos++;
        oldPos = newPos;
        newPos = strchr(newPos, ' ');
        if(newPos == NULL) {
            newPos = strchr(oldPos, '\r');
        }
        saveStr(&protocol, oldPos, newPos);

        uri = new URI(uriString);
        /*
        if(newPos >= 0) {
            newPos++;
            oldPos = newPos;
            newPos = strchr(newPos, '\n');
        }

        int i = 0;
        while (i < 32 && newPos != NULL) {
            saveStr(headers + i, oldPos, newPos);
            newPos++;
            oldPos = newPos;
            newPos = strchr(newPos, '\n');
            i++;
        }*/

    }

    ~RequestData() {
        if (uri != NULL) free(uri);
        if (method != NULL) free(method);
        if (uriString != NULL) free(uriString);
        if (protocol != NULL) free(protocol);

    }
};

struct ResponseData {

    RequestData * request;
    char * header;
    char * data;
    int headerLen;
    int dataLen;

    void getHTTPResponse() {

        header = (char*)malloc(4096);
        data = NULL;
        headerLen = 0;
        dataLen = 0;
        char * protocol = PROTOCOL;
        char * status = STATUS_200;
        FileData * resultFile;
        char * path = request->uri->path;
        qDebug() << request->method;
        qDebug() << METHOD_HEAD;
        bool isGET  = strcmp(request->method, METHOD_GET) == 0;
        bool isHEAD = strcmp(request->method, METHOD_HEAD) == 0;
        bool isPOST = strcmp(request->method, METHOD_POST) == 0;
        if (strstr(request->uri->path, "../")) {
            path = FORBIDDEN;
            resultFile = getFile(path, server_path, isGET);
            status = STATUS_403;
        }
        else {
            resultFile = getFile(request->uri->path, root_path, isGET);
        }

        if (resultFile->success != true) {
            if(request->uri->lastJ) {
                path = FORBIDDEN;
                resultFile = getFile(path, server_path, isGET);
                status = STATUS_403;;
            }
            else {
                path = NOT_FOUND;
                resultFile = getFile(path, server_path, isGET);
                status = STATUS_404;
            }
        }
        if (!(isGET || isHEAD)) {
            path = METHOD_NOT_ALLOWED;
            resultFile = getFile(path, server_path, true);
            status = STATUS_405;
        }

        char * format;

        char * ptr = strrchr(path, '.');
        if (ptr < 0) {
            format = MIME_TEXT_HTML;
        }
        else if (strcmp(ptr, ".jpg") == 0) {
            format = MIME_IMAGE_JPG;
        }
        else if(strcmp(ptr, ".png") == 0) {
            format = MIME_IMAGE_PNG;
        }
        else if(strcmp(ptr, ".jpeg") == 0) {
            format = MIME_IMAGE_JPEG;
        }
        else if(strcmp(ptr, ".gif") == 0) {
            format = MIME_IMAGE_GIF;
        }
        else if(strcmp(ptr, ".html") == 0) {
            format = MIME_TEXT_HTML;
        }
        else if(strcmp(ptr, ".css") == 0) {
            format = MIME_TEXT_CSS;
        }
        else if(strcmp(ptr, ".swf") == 0) {
            format = MIME_SWF;
        }
        else if(strcmp(ptr, ".js") == 0) {
            format = MIME_APPLICATION_JAVASCRIPT;
        }
        else {
            format = MIME_TEXT_HTML;
        }


        char * date = http_date(resultFile->date);


        data = NULL;
        dataLen = NULL;
        if (!isHEAD) {
            data = resultFile->data;
            dataLen = resultFile->length;
        }
        else {
            qDebug() << "isHead" << METHOD_HEAD;
        }

        headerLen = sprintf(header, "%s %s\r\n%s%s\r\n%s%s\r\n%s%s\r\n%s%s\r\n%s%d\r\n\r\n",
                protocol, status,
                HEADER_SERVER, SERVER_NAME,
                HEADER_DATE, date,
                HEADER_CONNECTION, CONNECTION_CLOSE,
                HEADER_CONTENT_TYPE, format,
                HEADER_CONTENT_LENGTH, resultFile->length);

    }

    ResponseData(RequestData * request) {
        this->request = request;
        getHTTPResponse();

    }
    ~ResponseData(){
        if (header != NULL) free(header);
        if (data != NULL) free(data);

    }
 };

#endif // HTML_H
