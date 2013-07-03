#include <iostream>
#include <stdio.h>
#include "ReverseReadFileByLine.h"

int main(int argc, char* argv[]){

    void* handle;
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
	char *filename;
	filename = argv[1];

    unsigned char *pBuf = NULL;
    size_t pSize = 0;
    handle = OpenFile(filename);
    if(NULL == handle){
        fprintf(stderr, "Create Handle Failed\n");
        CloseFile(handle);
        return 1;
    }

    while(GetLine(handle, &pBuf, &pSize) == 0){
        if(pBuf){
            printf("%s\n",pBuf);
            delete[] pBuf;
            pBuf = NULL;
        }
    }

    CloseFile(handle);

    return 0;
}
