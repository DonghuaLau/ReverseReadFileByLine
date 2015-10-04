#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ReverseReadFileByLine.h"

#define BUF_SIZE 1024
#define READ_BUF (10 * BUF_SIZE)

#define NO_ERROR 0
#define OPEN_FILE_ERROR 1
#define MM_MALLOC_ERROR 2
#define READ_FILE_ERROR 3
#define OVER_BUF_ERROR  4
#define NO_OBJECT_ERROR 5
#define NEW_HANDLE_ERROR 6

typedef struct __ReadHandle
{
    FILE* fp;
    unsigned long fileSize;
    unsigned long fileOffset;
    unsigned char* ReadBuf;//fixed size
    unsigned long ReadBufSize;
    unsigned int ReadBufPos;
    unsigned char* LineBuf;
    unsigned int LineBufSize;
    int ErrNo;
    bool fileReadEnd;
    bool workFinished;
}ReadHandle;

//打开文件，及完成相关初始化
void* OpenFile(char* fileName)
{
    ReadHandle *handle;
    handle = new ReadHandle;
	if(NULL == handle){
		return NULL;
	}

    handle->fp = fopen(fileName, "rb");
    if(NULL == handle->fp){
        handle->ErrNo = OPEN_FILE_ERROR;
        CloseFile(handle);
        return NULL;
    }
    handle->ReadBuf = new unsigned char[READ_BUF];
    if(NULL == handle->ReadBuf){
        handle->ErrNo = MM_MALLOC_ERROR;
        CloseFile(handle);
        return NULL;
    }
    handle->LineBuf = new unsigned char[BUF_SIZE];
    if(NULL == handle->LineBuf){
        handle->ErrNo = MM_MALLOC_ERROR;
        CloseFile(handle);
        return NULL;
    }

    fseek(handle->fp, 0,SEEK_END);
    handle->fileSize = handle->fileOffset = ftell(handle->fp);
    handle->ReadBufPos = 0;
    handle->ReadBufSize = 0;
    handle->LineBufSize = 0;
    handle->fileReadEnd = false;
	handle->workFinished = false;

    return handle;
}

//关闭文件，及释放内存
int CloseFile(void* read_handle)
{
    if(NULL == read_handle){
        return NO_OBJECT_ERROR;
    }
    ReadHandle *handle;
    handle = (ReadHandle*)read_handle;

    if(NULL != handle->LineBuf)
        delete[] handle->LineBuf;
    if(NULL != handle->ReadBuf)
        delete[] handle->ReadBuf;
    if(NULL != handle)
        delete handle;
    return 0;
}

//从文件中读取一定数据到缓冲中，返回读取文件数据的大小
inline int ReadToBuffer(ReadHandle *handle)
{
    if(handle->fileOffset != 0){
        if(handle->fileOffset >= (unsigned long)(READ_BUF)){
            fseek(handle->fp, (long)(handle->fileOffset - READ_BUF), SEEK_SET);
            handle->ReadBufSize = fread(handle->ReadBuf, 1, READ_BUF, handle->fp);
        }else{
            fseek(handle->fp, 0, SEEK_SET);
            handle->ReadBufSize = fread(handle->ReadBuf, 1, handle->fileOffset, handle->fp);
            handle->fileReadEnd = true;
        }
        handle->fileOffset -= handle->ReadBufSize;
        handle->ReadBufPos = handle->ReadBufSize - 1;
        if(0 == handle->ReadBufSize){
            handle->ErrNo = READ_FILE_ERROR;
            return 0;
        }
    }else{
        handle->fileReadEnd = true;
        return 0;
    }
    return handle->ReadBufSize;
}

//获取一行数据
int GetLine(void* read_handle, unsigned char** lineBuf, size_t *bufSize)
{
    if(NULL == read_handle){
        return NO_OBJECT_ERROR;
    }
    ReadHandle *handle;
    handle = (ReadHandle*)read_handle;

    if(handle->workFinished){
        return 1;
    }

    bool lineFlag = false;
    bool first_n = true; //igorne the first '\n'
    bool workFinish = false;
    handle->LineBufSize = BUF_SIZE - 1;

    while(!lineFlag){

        if(handle->LineBufSize == 0){
            handle->ErrNo = OVER_BUF_ERROR;
            return handle->ErrNo;
        }
        if(handle->ReadBufPos == 0){
            handle->LineBuf[handle->LineBufSize] = handle->ReadBuf[handle->ReadBufPos];
            if(handle->LineBuf[handle->LineBufSize] == 10){
                lineFlag = true;
            }

            if(handle->fileReadEnd){
                handle->workFinished = true;
                lineFlag = true;
            }
            if(handle->LineBuf[handle->LineBufSize] != 10) handle->LineBufSize--;

            int ret = 0;
            if((ret = ReadToBuffer(handle)) == 1){
                *bufSize = 0;
                return ret;
            }
            if(lineFlag) break;
        }

        unsigned int i = handle->ReadBufPos;
        unsigned int j = handle->LineBufSize;
        if(handle->ReadBuf[i] != 10){
            handle->LineBuf[j] = handle->ReadBuf[i];
            handle->ReadBufPos--;
            handle->LineBufSize--;
            continue;
        }else{
            if(first_n && handle->LineBufSize >= BUF_SIZE - 2){
                first_n = false;
                handle->ReadBufPos--;
                continue;
            }
            lineFlag = true;
            break;
        }
    }//end while(!lineFlag)

    *bufSize = BUF_SIZE - 1 - handle->LineBufSize;
    *lineBuf = new unsigned char[*bufSize+1];

    if(NULL == *lineBuf){
        handle->ErrNo = MM_MALLOC_ERROR;
        return handle->ErrNo;
    }
    memset(*lineBuf, 0, *bufSize+1);
    memcpy(*lineBuf, (void*)(handle->LineBuf+handle->LineBufSize+1), *bufSize);

    return 0;
}

//获取错误码
int GetLastErrno(void* read_handle)
{
    ReadHandle *handle;
    handle = (ReadHandle*)read_handle;
    int err = handle->ErrNo;
    handle->ErrNo = 0;

    return err;
}

//根据错误码获取错误信息
char* GetLastErrMsg(int errNo)
{
    static char msg[100] = {0};
    switch(errNo)
    {
        case NO_ERROR:
            sprintf(msg, "No error");
            break;
        case OPEN_FILE_ERROR:
            sprintf(msg, "Open file error");
            break;
        case MM_MALLOC_ERROR:
            sprintf(msg, "Memory malloc error");
            break;
        case READ_FILE_ERROR:
            sprintf(msg, "Read file error");
            break;
        case OVER_BUF_ERROR:
            sprintf(msg, "Over Buffer");
            break;
        case NO_OBJECT_ERROR:
            sprintf(msg, "No object, may be handle created failed");
            break;
        case NEW_HANDLE_ERROR:
            sprintf(msg, "Handle created failed");
            break;
        default:
            sprintf(msg, "Unkown error");
            break;

    }
    return msg;
}

