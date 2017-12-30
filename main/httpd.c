#include "autoinclude.h"
#include "shellutil.h"
#include "console/console.h"

#define HOSTNAME CONFIG_HOSTNAME

static struct DuHttpReceiver duHttpReceiver;
static struct DuHttp duHttp;
static struct DuHttp sendDuHttp;
void DuHttp_ELOG(const char* str) {
  printf("DuHTTP Error: %s\n", str);
}

extern const char INDEX_HTML[] asm("_binary_index_html_start");
extern const char INDEX_HTML_END[] asm("_binary_index_html_end");
extern const char TEST_HTML[] asm("_binary_test_html_start");
extern const char TEST_HTML_END[] asm("_binary_test_html_end");
extern const char FAVICON_ICO[] asm("_binary_favicon_ico_start");
extern const char FAVICON_ICO_END[] asm("_binary_favicon_ico_end");

extern const char ERR_404_HTML[] asm("_binary_404_html_start");
extern const char ERR_404_HTML_END[] asm("_binary_404_html_end");

struct StaticListType{
	const char* key;
	const char* value;
	const char* valueEnd;
	const char* type;
};
#define StaticListLen(x) ((x).valueEnd - (x).value)
const static struct StaticListType StaticList[] = {
	{"/", INDEX_HTML, INDEX_HTML_END, "text/html"},
	{"/404.html", ERR_404_HTML, ERR_404_HTML_END, "text/html"},
	{"/test.html", TEST_HTML, TEST_HTML_END, "text/html"},
    {"/favicon.ico", FAVICON_ICO, FAVICON_ICO_END, "image/x-icon"},
};
const static int StaticListLength = (sizeof(StaticList) / sizeof(struct StaticListType));

#define UTILBIAS sizeof("/util/")-1
#define UTILTAG "UTIL"
int duHttpUtilHandler(struct DuHttp* inPack, struct DuHttp* outPack) {
    int r, g, b;
    const char *tmpstr;
    if (!strcmp((inPack->ask.requestedURL) + UTILBIAS, "status")) {
        DuHttp_Initialize_RESPONSE(outPack, 200, "OK");
        DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
        DuHttp_PushHeadline(&sendDuHttp,"Access-Control-Allow-Origin", "*");
        DuHttp_EndHeadline(outPack);
        DuHttp_PushDataString(outPack, "ESP32 1.0.0");
        ESP_LOGI(UTILTAG, "Ask for Status");
        return 1;
    }
    return 0;
}
void duHttpInit() {
    // do some things here
}

int duHttpHandler(const char* inbuf, int size, char* outbuf, int maxsize) {
	int returnSize = 0;
	DuHttpReceiver_InBuf(&duHttpReceiver, inbuf, size);
	//printf("    Queue Available Size: %d\n",  DuHttpReceiver_AvailableSize(&duHttpReceiver));
	while (DuHttpReceiver_TryReadPack(&duHttpReceiver, &duHttp)) {
		printf("Got one Pack!\n");
		switch(duHttp.type) {
		case DuHttp_Type_UNKNOWN:
			printf("Pack Type is: DuHttp_Type_UNKNOWN\n");
			break;
		case DuHttp_Type_GET:
			printf("Pack Type is: DuHttp_Type_GET\n");
			printf("requestedURL: %s\n", duHttp.ask.requestedURL);
            if (!memcmp(duHttp.ask.requestedURL, "/util/", sizeof("/util/")-1)) {
                // move to util handler
                if (duHttpUtilHandler(&duHttp, &sendDuHttp)) {
                    returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
                }
            } else if (!memcmp(duHttp.ask.requestedURL, "/exec/", sizeof("/exec/")-1)) {
                // move to console to execute
                Shell.Out.clear();
                Console.exec(url_decode(duHttp.ask.requestedURL + sizeof("/exec/")-1));
                DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
                DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
                DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/plain");
                DuHttp_EndHeadline(&sendDuHttp);
                DuHttp_PushData(&sendDuHttp, Shell.Out.buffer(), Shell.Out.length());
                returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
            } else for (int i=0; i<StaticListLength; ++i) {
				if (!strcmp(StaticList[i].key, duHttp.ask.requestedURL)) {
					DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
					DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
					DuHttp_PushHeadline(&sendDuHttp,"Content-Type", StaticList[i].type);
					DuHttp_EndHeadline(&sendDuHttp);
					DuHttp_PushData(&sendDuHttp, StaticList[i].value, StaticListLen(StaticList[i]));
					returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
					break;
				}
			}
			//do other things
			break;
		case DuHttp_Type_POST:
			printf("Pack Type is: DuHttp_Type_POST\n");
			printf("Outputing Key-Value List: cnt=%d\n", duHttp.headlineCount);
			for (int i=0; i<duHttp.headlineCount; ++i) {
				printf("Key: %s\n", duHttp.headline[i].key);
				printf("Value: %s\n", duHttp.headline[i].value);
			}
            if (!memcmp(duHttp.ask.requestedURL, "/exec/", sizeof("/exec/")-1)) {
                // move to console to execute
                Shell.Out.clear();
                duHttp.content[duHttp.contentLength] = 0;
                Console.exec(duHttp.content);
                DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
                DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
                DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/plain");
                DuHttp_EndHeadline(&sendDuHttp);
                DuHttp_PushData(&sendDuHttp, Shell.Out.buffer(), Shell.Out.length());
                returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
            }
			/*va = DuHttp_FindValueByKey(&duHttp, "Tst2");
			if (va) {
				cout << "Find: " << va << endl;
			} else cout << "Not Found" << endl;*/
			//duHttp.content[duHttp.contentLength] = 0;
			printf("content: %s\n", duHttp.content);
			printf("requestedURL: %s\n", duHttp.ask.requestedURL);
			//DuHttp_Initialize_POST(&sendDuHttp, "/");
			/*DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
			DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
			DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
			DuHttp_EndHeadline(&sendDuHttp);
			DuHttp_PushData(&sendDuHttp, "Hello World! Wahaha", sizeof("Hello World! Wahaha")-1);
			returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);*/
			break;
		case DuHttp_Type_RESPONSE_HTTP1X:
			break;
		}
        if (returnSize == 0) {
            //return 404 Page
            DuHttp_Initialize_RESPONSE(&sendDuHttp, 404, "Not Found");
            DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
            DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
            DuHttp_EndHeadline(&sendDuHttp);
            sendDuHttp.contentLength += sprintf((sendDuHttp.content) + (sendDuHttp.contentLength), ERR_404_HTML, duHttp.ask.requestedURL, HOSTNAME); //DuHttp_PushData manually
            returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
        }
	}
	DuHttp_Release(&duHttp);
	//printf("    Queue Available Size: %d\n",  DuHttpReceiver_AvailableSize(&duHttpReceiver));
	return returnSize;
}


































const static char http_html_hdr[]  =
      "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

static char sendbuf[8192];
static void http_server_netconn_serve(struct netconn *conn) {
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  int sendlen;
  //printf("here %d\n", __LINE__);
  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    sendlen = duHttpHandler(buf, buflen, sendbuf, sizeof(sendbuf));
    netconn_write(conn, sendbuf, sendlen, NETCONN_NOCOPY);
  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);
  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

void httpd(){
  struct netconn *conn, *newconn;
  err_t err;

  DuHttpReceiver_Initialize(&duHttpReceiver);
  DuHttp_Initialize(&duHttp);
  ESP_LOGI("DuHTTP", "available size: %d",
      DuHttpReceiver_AvailableSize(&duHttpReceiver));

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, NULL, 80);
  netconn_listen(conn);
  ESP_LOGI("http_server", "listened");
  do {
      //ESP_LOGI("http_server", "doing");
      err = netconn_accept(conn, &newconn);
      ESP_LOGI("http_server", "accepted");
      if (err == ERR_OK) {
          http_server_netconn_serve(newconn);
          netconn_delete(newconn);
      }
   } while(err == ERR_OK);
   ESP_LOGI("http_server", "err???");
   netconn_close(conn);
   netconn_delete(conn);
}
