#ifndef HTML_H
#define HTML_H

#include <filemanager.h>

char MIME_IMAGE_JPEG[]              = "image/jpeg";
char MIME_IMAGE_JPG[]               = "image/pjpeg";
char MIME_IMAGE_PNG[]               = "image/png";
char MIME_IMAGE_GIF[]               = "image/gif";
char MIME_SWF[]                     = "application/x-shockwave-flash";
char MIME_TEXT_HTML[]               = "text/html";
char MIME_TEXT_CSS[]                = "text/css";
char MIME_APPLICATION_JAVASCRIPT[]  = "application/javascript";

char CONNECTION_KEEP_ALIVE[] = "keep-alive";
char CONNECTION_CLOSE[] = "close";

char STATUS_200[] = "200 OK";
char STATUS_404[] = "404 Not Found";

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


char root_path[] = "/home/dmitry/http-test-suite/httptest";
char server_path[] = "/home/dmitry";
char NOT_FOUND[] = "/notFound.html";

struct HTML {
    HTML() {

    }
    ~HTML() {

    }

    char * data;
    long len;
};

char * http_date(tm * time) {
    char * result = (char*)malloc(40);
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
    char * fragment;
    char * data;
    int dataLen;
    URI(char * uriString) {


        bool error = true;
        int len = strlen(uriString);

        char * strEnd =  uriString + len;

        char * pathBegin = NULL;
        char * pathEnd = strEnd;

        char * queryBegin = NULL;
        char * queryEnd = strEnd;

        char * fragmentBegin = NULL;
        char * fragmentEnd = strEnd;

        pathBegin = strchr(uriString, '/');
        if (pathBegin != NULL) {
            queryBegin = strchr(uriString, '?');
            if(queryBegin != NULL) {
                pathEnd = queryBegin;
            }
            fragmentBegin = strchr(uriString, '#');
            if(queryBegin == NULL && fragmentBegin != NULL) {
                pathEnd = fragmentBegin;
            }

            if(queryBegin != NULL && fragmentBegin != NULL) {
                queryEnd = fragmentBegin;
            }


            dataLen = len + 15;
            data = (char*)malloc(dataLen);

            memcpy(data, pathBegin, pathEnd - pathBegin);
            path[pathEnd - pathBegin] = '\0';
            path = data;

            if(queryBegin != NULL) {
                memcpy(query, queryBegin, queryEnd - queryBegin);
                path[queryEnd - queryBegin] = '\0';
                query = data;
            }

            if(fragmentBegin != NULL) {
                memcpy(fragment, fragmentBegin, fragmentEnd - fragmentBegin);
                path[fragmentEnd - fragmentBegin] = '\0';
                fragment = data;
            }

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

        if (otherURI.fragment != NULL) {
             this->fragment = this->data + (otherURI.data - otherURI.fragment) ;
        }
        else {
            this->fragment = NULL;
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
    long httpRequestLen;
    char ** headers;

    void saveStr(char ** saveWhere, char * begin, char * end) {
        *saveWhere = (char*)malloc(end - begin + 1);
        memcpy(*saveWhere, begin, end - begin);
        (*saveWhere)[end - begin] = '\0';

    }

    RequestData(char * httpRequest, long httpRequestLen) {
        headers = (char**)malloc(32);

        this->httpRequest = httpRequest;
        this->httpRequestLen = httpRequestLen;
        char * oldPos = httpRequest;
        char * newPos = httpRequest;
        newPos = strchr(newPos, ' ');
        saveStr(&method, oldPos, newPos);

        newPos++;
        oldPos = newPos;
        newPos = strchr(newPos, ' ');
        saveStr(&uriString, oldPos, newPos);

        newPos++;
        oldPos = newPos;
        newPos = strchr(newPos, ' ');
        saveStr(&protocol, oldPos, newPos);

        uri = new URI(uriString);

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
        }



    }
    ~RequestData() {
        free(uri);
    }
};

struct ResponseData {

    RequestData * request;

    HTML * getHTTPResponse() {


        HTML * result = new HTML();
        char * protocol = PROTOCOL;
        char * status = STATUS_200;
        FileData resultFile;
        bool isHEAD = strcmp(request->method, METHOD_HEAD) == 0;
        resultFile = getFile(request->uri->path, root_path, !isHEAD);

        char * path = request->uri->path;

        if (resultFile.success != true) {
            path = NOT_FOUND;
            resultFile = getFile(path, server_path, !isHEAD);
            status = STATUS_404;
        }

        char * format;

        char * ptr = strchr(path, '.');
        if (ptr < 0) {
            format = MIME_TEXT_HTML;
        }
        else if (strcmp(ptr, ".jpg") == 0) {
            format = MIME_IMAGE_JPEG;
        }
        else if(strcmp(ptr, ".png") == 0) {
            format = MIME_IMAGE_PNG;
        }
        else if(strcmp(ptr, ".jpg") == 0) {
            format = MIME_IMAGE_JPG;
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
        else {
            format = MIME_TEXT_HTML;
        }


        char * date = http_date(resultFile.date);


        if (isHEAD) {
            result->data = (char*) malloc(1024);
        }
        else {
            result->data = (char*) malloc(1024 + resultFile.length);
        }

        int len = sprintf(result->data, "%s %s\n%s%s\n%s%s\n%s%s\n%s%s\n%s%d\n\n",
                protocol, status,
                HEADER_SERVER, SERVER_NAME,
                HEADER_DATE, date,
                HEADER_CONNECTION, CONNECTION_CLOSE,
                HEADER_CONTENT_TYPE, format,
                HEADER_CONTENT_LENGTH, resultFile.length);

        if (isHEAD) {
            result->len = len;
        }
        else {
            memcpy(result->data + len, resultFile.data, resultFile.length);
            result->len = len + resultFile.length;
        }
        return result;
    }

    ResponseData(RequestData * request) {
        this->request = request;


    }
    ~ResponseData(){

    }
 };

#endif // HTML_H
