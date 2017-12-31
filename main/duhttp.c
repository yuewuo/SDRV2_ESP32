/*
 * Duplex HTTP library
 * author: wy
 * last-modified: 20171230
 * version: 0.1
 */

#include "duhttp.h"

void DuHttp_Initialize(struct DuHttp* d)
{
    d->type = DuHttp_Type_UNKNOWN;
    d->content = d->data;
    d->contentLength = 0;
    d->headlineCount = 0;
}

void DuHttp_Initialize_GET(struct DuHttp* d, const char* requestedURL)
{
	DuHttp_Initialize(d);
	d->type = DuHttp_Type_GET;
	strcpy(d->ask.requestedURL, requestedURL);
}
void DuHttp_Initialize_POST(struct DuHttp* d, const char* requestedURL)
{
	DuHttp_Initialize(d);
	d->type = DuHttp_Type_POST;
	strcpy(d->ask.requestedURL, requestedURL);
}
void DuHttp_Initialize_RESPONSE(struct DuHttp* d, int statusCode, const char* reasonPhrase)
{
	DuHttp_Initialize(d);
	d->type = DuHttp_Type_RESPONSE_HTTP1X;
	d->response.statusCode = statusCode;
	strcpy(d->response.reasonPhrase, reasonPhrase);
}

void DuHttp_Release(struct DuHttp* d)
{
	DuHttp_Initialize(d);
}

char* DuHttp_FindValueByKey(struct DuHttp* d, const char* key)
{
	int i = 0;
	for (; i<d->headlineCount; ++i) {
		if (!strcmp(key, d->headline[i].key)) {
			return d->headline[i].value;
		}
	}
	return 0;
}

void DuHttp_PushHeadline(struct DuHttp* d, const char* key, const char* value)
{
	strcpy(d->content, key); d->content += strlen(key);
	strcpy(d->content, ": "); d->content += 2;
	strcpy(d->content, value); d->content += strlen(value);
	strcpy(d->content, "\r\n"); d->content += 2;
}

void DuHttp_PushData(struct DuHttp* d, const char* data, int dataLength)
{
	memcpy((d->content) + (d->contentLength), data, dataLength);
	d->contentLength += dataLength;
}

void DuHttp_PushDataString(struct DuHttp* d, const char* str)
{
  DuHttp_PushData(d, str, strlen(str));
}

void DuHttp_EndHeadline(struct DuHttp* d)
{
	strcpy(d->content, "\r\n"); d->content += 3;
}

void DuHttpReceiver_Reset(struct DuHttpReceiver* r)
{
    r->nowState = DuHttpReceiver_State_RecevingHead;
    r->headLineIndex = 0;
    r->hasReadDataLength = 0;
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

#define DuHttpReceiver_TryReadPack_Step (r->queue_read = (r->queue_read + 1) % DUHTTPRECEIVER_BUFFERSIZE)
char DuHttpReceiver_TryReadPack(struct DuHttpReceiver* r, struct DuHttp* pack)
{
    char queueindex;
    for (; r->queue_read != r->queue_write; DuHttpReceiver_TryReadPack_Step) {
        queueindex = r->queue[r->queue_read];
        switch (r->nowState) {
        case DuHttpReceiver_State_RecevingHead:
            if (queueindex != '\r' && queueindex != '\n') {
                r->headLineStr[r->headLineIndex] = queueindex;
                ++(r->headLineIndex);
            } else if (queueindex == '\n') {
                r->headLineStr[r->headLineIndex] = 0;
                //finish one line, resolve the information of it
                if (r->headLineIndex == 0) {
                    //finish head
                    r->nowState = DuHttpReceiver_State_RecevingData;
                    if (pack->contentLength == 0) {
                        //pack receive finished! initialmyself and return
                        DuHttpReceiver_Reset(r);
                        DuHttpReceiver_TryReadPack_Step;
                        return TRUE;
                    }
                } else {
                    r->headLineIndex = 0;
                    DuHttpReceiver_TryResolveHeadLine(r->headLineStr, pack);
                }
            }
            break;
        case DuHttpReceiver_State_RecevingData:
            if (r->hasReadDataLength < pack->contentLength) {
                pack->content[r->hasReadDataLength] = queueindex;
                ++(r->hasReadDataLength);
            }
            if (r->hasReadDataLength == pack->contentLength) {
                //pack receive finished! initialmyself and return
                DuHttpReceiver_Reset(r);
                DuHttpReceiver_TryReadPack_Step;
                return TRUE;
            }
            break;
        default:
            break;
        }
    }
    return FALSE;
}

#define strStartWith(y) (!memcmp(str, y, sizeof(y)-1))
void DuHttpReceiver_TryResolveHeadLine(const char* str, struct DuHttp* pack)
{
	char* tmp;
    if (pack->type == DuHttp_Type_UNKNOWN) {
        if (strStartWith("GET")) {
            pack->type = DuHttp_Type_GET;
            sscanf(str+4, "%s", pack->ask.requestedURL);
            return;
        }
        if (strStartWith("POST")) {
            pack->type = DuHttp_Type_POST;
            sscanf(str+5, "%s", pack->ask.requestedURL);
            return;
        }
        if (strStartWith("HTTP")) {
            pack->type = DuHttp_Type_RESPONSE_HTTP1X;
            sscanf(str+9, "%d %s", &(pack->response.statusCode),
                   pack->response.reasonPhrase);
            return;
        }
        DuHttp_ELOG("Unknow Http First Line:");
        DuHttp_ELOG(str); return;
    } else if (pack->type == DuHttp_Type_GET || pack->type == DuHttp_Type_POST || pack->type == DuHttp_Type_RESPONSE_HTTP1X) {
        if (strStartWith("Content-Length: ")) {
            sscanf(str+sizeof("Content-Length: ")-1, "%d",
                   &(pack->contentLength));
        }
        strcpy(pack->content, str);
        pack->headline[pack->headlineCount].key = pack->content;
        tmp = strchr(pack->content, ':');
        *tmp = 0;
        pack->headline[pack->headlineCount].value = tmp+2;
        //DuHttp_ELOG(pack->headline[pack->headlineCount].key);
        //DuHttp_ELOG(pack->headline[pack->headlineCount].value);
        pack->content += strlen(str) + 1;
        //cout << "headline Length = " << (pack->content) - (pack->data) << endl;
        ++(pack->headlineCount);
        //DuHttp_ELOG("ASK Pack unknown line:");
        //DuHttp_ELOG(str);
		return;
    }
    DuHttp_ELOG("Should not reach here! DuHttpReceiver_TryResolveHeadLine");
}

int DuHttpReceiver_AvailableSize(struct DuHttpReceiver* r)
{
    return ((r->queue_read) - 1 - (r->queue_write) + DUHTTPRECEIVER_BUFFERSIZE) % DUHTTPRECEIVER_BUFFERSIZE;
}

#define DuHttpSend_IO_ConstStr(x) do {\
    int size = (int)sizeof(x) - 1;\
    if (size + now_size >= max_size) return 0;\
    memcpy(buf+now_size, x, size);\
    now_size += size;\
} while(0)
#define DuHttpSend_IO_Str(x) do {\
    int size = (int)strlen(x);\
    if (size + now_size >= max_size) return 0;\
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
        DuHttpSend_IO_Str(h->ask.requestedURL);
        DuHttpSend_IO_ConstStr(" HTTP/1.1\r\n");
    } else if (h->type == DuHttp_Type_RESPONSE_HTTP1X) {
        DuHttpSend_IO_ConstStr("HTTP/1.1 ");
        sprintf(tmp, "%d", h->response.statusCode);
        DuHttpSend_IO_Str(tmp);
        DuHttpSend_IO_ConstStr(" ");
        DuHttpSend_IO_Str(h->response.reasonPhrase);
        DuHttpSend_IO_ConstStr("\r\n");
    }
    if (h->contentLength) {
    	DuHttpSend_IO_ConstStr("Content-Length: ");
    	sprintf(tmp, "%d", h->contentLength);
    	DuHttpSend_IO_Str(tmp);
    	DuHttpSend_IO_ConstStr("\r\n");
	}
    DuHttpSend_IO_Str(h->data); //send head lines (including "\r\n")
    if ((h->contentLength) + now_size >= max_size) return 0;
    memcpy(buf+now_size, h->content, h->contentLength);
    now_size += (h->contentLength);
    buf[now_size] = 0;
    return now_size;
}

#include <ctype.h>

char *url_decode(char *str)
{
    char *dest = str;
    char *data = str;

    while (*data) {
        if (*data == '+')
            *dest = ' ';
        else if (*data == '%' && data[1] && data[2] && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
            *dest = (char) htoi2(data + 1);
            data += 2;
        }
        else
            *dest = *data;
        data++;
        dest++;
    }
    *dest = '\0';
    printf("%s\n", str);
    return str;
}

uint8_t htoi2(char *s) {
    uint8_t value;
    uint8_t c;

    c = ((unsigned char *)s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) << 4;

    c = ((unsigned char *)s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}
