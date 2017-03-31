#ifndef HTML_H
#define HTML_H

#include <filemanager.h>

struct RequestData {
    char * httpRequest;
    char * uri;
    char * method;
    long httpRequestLen;

    void saveStr(char ** saveWhere, char * saveWhat, long strLen) {
        *saveWhere = (char*)malloc(strLen + 100);
        memcpy(*saveWhere, saveWhat, strLen);
        (*saveWhere)[strLen] = '\0';

    }
    //strchr

    RequestData(char * httpRequest, long httpRequestLen) {
        this->httpRequest = httpRequest;
        this->httpRequestLen = httpRequestLen;
        char * oldPos = httpRequest;
        char * newPos = httpRequest;
        newPos = strchr(newPos, ' ');
        saveStr(&method, oldPos, newPos - oldPos);
        newPos++;
        oldPos = newPos;
        newPos = strchr(newPos, ' ');
        saveStr(&uri, oldPos, newPos - oldPos);
    }
    ~RequestData() {

    }
};

struct HTML {
    HTML() {

    }
    ~HTML() {

    }

    char * data;
    long len;
};

char root_path[] = "/home/dmitry/http-test-suite/httptest";
char NOT_FOUND[] = "/notFound.html";

struct ResponseData {

    void toBytes(char * path, HTML * result) {
                 char * test = NOT_FOUND;
        FileData resultFile = getFile(path, root_path);
                test = NOT_FOUND;
        if (resultFile.data == NULL) {
            path = NOT_FOUND;
            resultFile = getFile(path, root_path);
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
        char * protocol = PROTOCOL;
        char * status = STATUS_200;

        result->data = (char*) malloc(10240 + resultFile.length);

        int len = sprintf(result->data, "%s %s\n%s%s\n%s%s\n%s%d\n\n",
                protocol, status,
                HEADER_SERVER, SERVER_NAME,
                HEADER_CONTENT_TYPE, format,
                HEADER_CONTENT_LENGTH, resultFile.length);
        memcpy(result->data + len, resultFile.data, resultFile.length);
        result->len = len + resultFile.length;
    }

    ResponseData() {

    }
    ~ResponseData(){

    }
 };

#endif // HTML_H
