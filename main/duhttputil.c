#include "lib/duhttputil.h"

#define TRUE 1
#define FALSE 0

void DuHttp_Initialize(struct DuHttp* d)
{
    d->type = DuHttp_Type_UNKNOWN;
    d->contentLength = 0;
}

void DuHttpReceiver_Reset(struct DuHttpReceiver* r)
{
    r->nowState = DuHttpReceiver_State_RecevingHead;
    r->headLineIndex = 0;
    r->hasReadDataLength = 0;
    r->isLastLF = FALSE;
}

void DuHttpReceiver_Initialize(struct DuHttpReceiver* r)
{
    DuHttpReceiver_Reset(r);
    r->queue_write = 0;
    r->queue_read = 0;
}

char DuHttpReceiver_InBuf(struct DuHttpReceiver* r,
                          const char* buf, int bufsize)
{
    int i;
    char* queue = r->queue;
    int endIndex = bufsize + r->queue_write;
    if (bufsize > DuHttpReceiver_AvailableSize(r)) {
        DuHttp_ELOG("DuHttpReceiver Buffer Fulled");
        return FALSE;
    }
    for (i = r->queue_write; i < endIndex; ++i) {
        queue[i % DUHTTPRECEIVER_BUFFERSIZE] = *(buf++);
    }
    r->queue_write = endIndex % DUHTTPRECEIVER_BUFFERSIZE;
    return TRUE;
}

char DuHttpReceiver_TryReadPack(struct DuHttpReceiver* r, struct DuHttp* pack)
{
    char* queue = r->queue;
    int index = r->queue_read;
    for (index = r->queue_read; index != r->queue_write;
         index = (index + 1) % DUHTTPRECEIVER_BUFFERSIZE) {
        if (r->nowState == DuHttpReceiver_State_RecevingHead) {

        }
        switch (r->nowState) {
        case DuHttpReceiver_State_RecevingHead:
            if (queue[index] != '\r' && queue[index] != '\n') {
                r->headLineStr[r->headLineIndex] = queue[index];
                ++(r->headLineIndex);
            } else if (queue[index] == '\n') {
                r->headLineStr[r->headLineIndex] = 0;
                //finish one line, resolve the information of it
                if (r->headLineIndex == 0) {
                    if (r->isLastLF) {
                        //finish head
                        r->nowState = DuHttpReceiver_State_RecevingData;
                        if (pack->contentLength == 0) {
                            //pack receive finished! initialmyself and return
                            DuHttpReceiver_Reset(r);
                            return TRUE;
                        }
                    } else {
                        r->isLastLF = TRUE;
                    }
                } else {
                    r->headLineIndex = 0;
                    DuHttpReceiver_TryResolveHeadLine(r->headLineStr, pack);
                }
            }
            break;
        case DuHttpReceiver_State_RecevingData:
            if (r->hasReadDataLength < pack->contentLength) {
                pack->data[r->hasReadDataLength] = queue[index];
                ++(r->hasReadDataLength);
            }
            if (r->hasReadDataLength == pack->contentLength) {
                //pack receive finished! initialmyself and return
                DuHttpReceiver_Reset(r);
                return TRUE;
            }
            break;
        default:
            break;
        }
    }
    return FALSE;
}

#define strStartWith(y) (memcmp(str, y, sizeof(y)))
void DuHttpReceiver_TryResolveHeadLine(const char* str, struct DuHttp* pack)
{
    if (pack->type == DuHttp_Type_UNKNOWN) {
        if (strStartWith("GET")) {
            pack->type = DuHttp_Type_GET;
            sscanf(str+4, "%s", pack->ask.requestedURI);
            return;
        }
        if (strStartWith("POST")) {
            pack->type = DuHttp_Type_POST;
            sscanf(str+5, "%s", pack->ask.requestedURI);
            return;
        }
        if (strStartWith("HTTP/1.0")) {
            pack->type = DuHttp_Type_RESPONSE_HTTP10;
            sscanf(str+9, "%d %s", &(pack->response.statusCode),
                   pack->response.reasonPhrase);
            return;
        }
        DuHttp_ELOG("Unknow Http First Line:");
        DuHttp_ELOG(str); return;
    } else if (pack->type == DuHttp_Type_GET || pack->type == DuHttp_Type_POST) {
        if (strStartWith("Content-Length: ")) {
            sscanf(str+sizeof("Content-Length: "), "%d",
                   &(pack->contentLength));
        }
        DuHttp_ELOG("ASK Pack unknown line:");
        DuHttp_ELOG(str); return;
    } else if (pack->type == DuHttp_Type_RESPONSE_HTTP10) {
        if (strStartWith("Content-Length: ")) {
            sscanf(str+sizeof("Content-Length: "), "%d",
                   &(pack->contentLength));
        }
        DuHttp_ELOG("RESPONSE Pack unknown line:");
        DuHttp_ELOG(str); return;
    }
    DuHttp_ELOG("Should not reach here! DuHttpReceiver_TryResolveHeadLine");
}

int DuHttpReceiver_AvailableSize(struct DuHttpReceiver* r)
{
    return ((r->queue_read) - 1 - (r->queue_write) + DUHTTPRECEIVER_BUFFERSIZE) % DUHTTPRECEIVER_BUFFERSIZE;
}

#define DuHttpSend_IO_ConstStr(x) do {\
    int size = (int)sizeof(x) - 1;\
    if (size + now_size >= max_size) return -1;\
    memcpy(buf+now_size, x, size);\
    now_size += size;\
} while(0)
#define DuHttpSend_IO_Str(x) do {\
    int size = (int)strlen(x);\
    if (size + now_size >= max_size) return -1;\
    memcpy(buf+now_size, x, size);\
    now_size += size;\
} while(0)
int DuHttpSend(struct DuHttp* h, char* buf, int max_size)
{
    int now_size = 0;
    char tmp[16];
    if (h->type == DuHttp_Type_GET || h->type == DuHttp_Type_POST) {
        if (h->type == DuHttp_Type_GET) {
            DuHttpSend_IO_ConstStr("GET ");
        } else {
            DuHttpSend_IO_ConstStr("POST ");
        }
        DuHttpSend_IO_Str(h->ask.requestedURI);
        DuHttpSend_IO_ConstStr(" HTTP/1.0\r\n\r\n");
    } else if (h->type == DuHttp_Type_RESPONSE_HTTP10) {
        DuHttpSend_IO_ConstStr("HTTP/1.0 ");
        sprintf(tmp, "%d", h->response.statusCode);
        DuHttpSend_IO_Str(tmp);
        DuHttpSend_IO_ConstStr(" ");
        DuHttpSend_IO_Str(h->response.reasonPhrase);
        DuHttpSend_IO_ConstStr("\r\n\r\n");
    }
    return now_size;
}
