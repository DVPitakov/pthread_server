#ifndef VARIABLES_H
#define VARIABLES_H

#endif // VARIABLES_H

char default_path[] = "/home/dmitry/http-test-suite/httptest";
struct SERVER_CONFIG {
    static char * root_dir;
    static unsigned int threads_count;
    void router(char * path) {

    }

    SERVER_CONFIG() {
         root_dir = default_path;
         threads_count = 4;
    }
    ~SERVER_CONFIG() {

    }
};
