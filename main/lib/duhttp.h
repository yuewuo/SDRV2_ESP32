#ifndef DUHTTPUTIL_H
#define DUHTTPUTIL_H

#include <string.h>
#include <stdio.h>

#define DUHTTP_MAXLINESIZE 512
#define DUHTTP_MAXHEADLINECOUNT 32
#define DUHTTPRECEIVER_BUFFERSIZE 4096
#define DUHTTP_DATASIZE (DUHTTPRECEIVER_BUFFERSIZE + 0) //must be positive
#define DUHTTPSENDER_DATASIZE 1024

// you need to realize these functions below
void DuHttp_ELOG(const char* str);
// and call initial funtion whenever you need to use them next time!

#define UINT8 unsigned char

#define DuHttp_Type_UNKNOWN ((UINT8)0)
#define DuHttp_Type_GET ((UINT8)1)
#define DuHttp_Type_POST ((UINT8)2)
#define DuHttp_Type_RESPONSE_HTTP1X ((UINT8)3)
struct DuHttp
{
    UINT8 type;
    char data[DUHTTP_DATASIZE];
    char* content;
    int contentLength;
    int headlineCount;
    struct {
    	char* key;
    	char* value;
	} headline[DUHTTP_MAXHEADLINECOUNT];
    union {
        struct {
			char requestedURL[128];
		} ask;
        struct {
    		int statusCode;
    		char reasonPhrase[32];
		} response;
    };
};
void DuHttp_Initialize(struct DuHttp* d);
void DuHttp_Initialize_GET(struct DuHttp* d, const char* requestedURL);
void DuHttp_Initialize_POST(struct DuHttp* d, const char* requestedURL);
void DuHttp_Initialize_RESPONSE(struct DuHttp* d, int statusCode, const char* reasonPhrase);
void DuHttp_Release(struct DuHttp* d);
char* DuHttp_FindValueByKey(struct DuHttp* d, const char* key);
void DuHttp_PushHeadline(struct DuHttp* d, const char* key, const char* value);
void DuHttp_PushData(struct DuHttp* d, const char* data, int dataLength);
void DuHttp_PushDataString(struct DuHttp* d, const char* str);
void DuHttp_EndHeadline(struct DuHttp* d);

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
};

void DuHttpReceiver_Reset(struct DuHttpReceiver* r);
void DuHttpReceiver_Initialize(struct DuHttpReceiver* r);
char DuHttpReceiver_InBuf(struct DuHttpReceiver* r,
                          const char* buf, int bufsize);
char DuHttpReceiver_TryReadPack(struct DuHttpReceiver* r, struct DuHttp* pack);
void DuHttpReceiver_TryResolveHeadLine(const char* str, struct DuHttp* pack);
int DuHttpReceiver_AvailableSize(struct DuHttpReceiver* r);

int DuHttpSend(struct DuHttp* h, char* buf, int max_size);

#define TRUE 1
#define FALSE 0

#endif // DUHTTPUTIL_H
