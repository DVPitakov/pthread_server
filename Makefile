httpd: filemanager.h html.h main.cpp
	g++ -o httpd -pthread filemanager.h html.h main.cpp
