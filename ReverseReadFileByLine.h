#ifndef _RRFBL_HEADER_

#define _RRFBL_HEADER_

void* OpenFile(char* fileName);
int CloseFile(void* read_handle);
int GetLine(void* read_handle, unsigned char** lineBuf, size_t *bufSize);
int GetLastErrno(void* read_handle);
char* GetLastErrMsg(int errNo);

#endif
