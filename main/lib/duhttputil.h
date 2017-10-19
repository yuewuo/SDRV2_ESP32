#ifndef DUHTTPUTIL_H
#define DUHTTPUTIL_H

#include <string.h>
#include <stdio.h>

#define DUHTTP_MAXLINESIZE 256
#define DUHTTPRECEIVER_BUFFERSIZE 2048
#define DUHTTP_DATASIZE (DUHTTPRECEIVER_BUFFERSIZE + 0) //must be positive
#define DUHTTPSENDER_DATASIZE 1024

// you need to realize these functions below
void DuHttp_ELOG(const char* str);
// and call initial funtion whenever you need to use them next time!

struct DuHttp_AskStruct
{
    char requestedURI[128];
};

struct DuHttp_ResponseStruct
{
    int statusCode;
    char reasonPhrase[32];
};

struct DuHttp
{
#define DuHttp_Type_UNKNOWN 0xFF
#define DuHttp_Type_GET 0
#define DuHttp_Type_POST 1
#define DuHttp_Type_RESPONSE_HTTP10 2
    char type;
    int contentLength;
    char data[DUHTTP_DATASIZE];
    union {
        struct DuHttp_AskStruct ask;
        struct DuHttp_ResponseStruct response;
    };
};
void DuHttp_Initialize(struct DuHttp* d);

struct DuHttpReceiver
{
#define DuHttpReceiver_State_RecevingHead 0
#define DuHttpReceiver_State_RecevingData 1
    int nowState;
    char headLineStr[DUHTTP_MAXLINESIZE];
    int headLineIndex;
    int hasReadDataLength;
    char queue[DUHTTPRECEIVER_BUFFERSIZE];
    int queue_write;
    int queue_read;
    char isLastLF;
};

void DuHttpReceiver_Reset(struct DuHttpReceiver* r);
void DuHttpReceiver_Initialize(struct DuHttpReceiver* r);
char DuHttpReceiver_InBuf(struct DuHttpReceiver* r,
                          const char* buf, int bufsize);
char DuHttpReceiver_TryReadPack(struct DuHttpReceiver* r, struct DuHttp* pack);
void DuHttpReceiver_TryResolveHeadLine(const char* str, struct DuHttp* pack);
int DuHttpReceiver_AvailableSize(struct DuHttpReceiver* r);

int DuHttpSend(struct DuHttp* h, char* buf, int max_size);

#endif // DUHTTPUTIL_H
