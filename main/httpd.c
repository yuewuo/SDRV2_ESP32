#include "lib/lib.h"
#include "lib/duhttp.h"

#define HOSTNAME CONFIG_HOSTNAME

struct DuHttpReceiver duHttpReceiver;
struct DuHttp duHttp;
struct DuHttp sendDuHttp;
void DuHttp_ELOG(const char* str) {
  printf("DuHTTP Error: %s\n", str);
}

extern const char INDEX_HTML[] asm("_binary_index_html_start");
extern const char INDEX_HTML_END[] asm("_binary_index_html_end");
extern const char TEST_HTML[] asm("_binary_test_html_start");
extern const char TEST_HTML_END[] asm("_binary_test_html_end");

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
};
const static int StaticListLength = (sizeof(StaticList) / sizeof(struct StaticListType));

#define UTILBIAS sizeof("/util/")-1
#define UTILTAG "UTIL"
int duHttpUtilHandler(struct DuHttp* inPack, struct DuHttp* outPack) {
  if (!strcmp((inPack->ask.requestedURL) + UTILBIAS, "turnOnLight")) {
    DuHttp_Initialize_RESPONSE(outPack, 200, "OK");
    DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
    DuHttp_EndHeadline(outPack);
    DuHttp_PushDataString(outPack, "turn on finished");
    ESP_LOGI(UTILTAG, "LED turned on");
    gpio_set_level(BLINK_GPIO, 1);
    return 1;
  }
  if (!strcmp((inPack->ask.requestedURL) + UTILBIAS, "turnOffLight")) {
    DuHttp_Initialize_RESPONSE(outPack, 200, "OK");
    DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
    DuHttp_EndHeadline(outPack);
    DuHttp_PushDataString(outPack, "turn off finished");
    ESP_LOGI(UTILTAG, "LED turned off");
    gpio_set_level(BLINK_GPIO, 0);
    return 1;
  }
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
  gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
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
        //move to util handler
        if (duHttpUtilHandler(&duHttp, &sendDuHttp)) {
          returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
        }
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
			if (returnSize) break;
			else {
				//return 404 Page
				DuHttp_Initialize_RESPONSE(&sendDuHttp, 404, "Not Found");
				DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
				DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
				DuHttp_EndHeadline(&sendDuHttp);
				sendDuHttp.contentLength += sprintf((sendDuHttp.content) + (sendDuHttp.contentLength), ERR_404_HTML, duHttp.ask.requestedURL, HOSTNAME); //DuHttp_PushData manually
				returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
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
			/*va = DuHttp_FindValueByKey(&duHttp, "Tst2");
			if (va) {
				cout << "Find: " << va << endl;
			} else cout << "Not Found" << endl;*/
			duHttp.content[duHttp.contentLength] = 0;
			printf("content: %s\n", duHttp.content);
			printf("requestedURL: %s\n", duHttp.ask.requestedURL);
			//DuHttp_Initialize_POST(&sendDuHttp, "/");
			DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
			DuHttp_PushHeadline(&sendDuHttp, "Connection", "keep-alive");
			DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
			DuHttp_EndHeadline(&sendDuHttp);
			DuHttp_PushData(&sendDuHttp, "Hello World! Wahaha", sizeof("Hello World! Wahaha")-1);
			returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
			break;
		case DuHttp_Type_RESPONSE_HTTP1X:
			break;
		}
	}
	DuHttp_Release(&duHttp);
	//printf("    Queue Available Size: %d\n",  DuHttpReceiver_AvailableSize(&duHttpReceiver));
	return returnSize;
}


































const static char http_html_hdr[]  =
      "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

/*const static char INDEXHTML[] = "<!DOCTYPE html>"
      "<html>\n"
      "<head>\n"
      "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
      "  <style type=\"text/css\">\n"
      "    html, body, iframe { margin: 0; padding: 0; height: 100%; }\n"
      "    iframe { display: block; width: 100%; border: none; }\n"
      "  </style>\n"
      "<title>Hello world!</title>\n"
      "</head>\n"
      "<body>\n"
      "<h1>Hello world!!!</h1>\n"
      "</body>\n"
      "</html>\n";*/

/*extern const char INDEX_HTML[] asm("_binary_index_html_start");

static struct {
	const char* key;
	const char* value;
	const char* type;
} StaticList[] = {{"/", INDEX_HTML, "sdsd"}};*/

static char sendbuf[4096];
static void http_server_netconn_serve(struct netconn *conn) {
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  int sendlen;
  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    sendlen = duHttpHandler(buf, buflen, sendbuf, sizeof(sendbuf));
    netconn_write(conn, sendbuf, sendlen, NETCONN_NOCOPY);

    /*DuHttpReceiver_InBuf(&duHttpReceiver, buf, buflen);
    ESP_LOGI("DuHTTP", "buflen = %d, %d, %d", buflen, duHttpReceiver.queue_write, duHttpReceiver.queue_read);
    ESP_LOGI("DuHTTP", "available size: %d", DuHttpReceiver_AvailableSize(&duHttpReceiver));
    while (DuHttpReceiver_TryReadPack(&duHttpReceiver, &duHttp)) {
      ESP_LOGI("DuHTTP", "Got Pack!");
    }*/

    /* Is this an HTTP GET command?
    there are other formats for GET, and we're keeping it very simple )*/
    //buf[buflen] = 0;
    //printf("%s \n", buf);
    //printf("char is:%c\n", buf[9]);
    //wuyue motor!!!
    /*if(buf[9] == 'z') {
    	motor_cnt += 5;
    }
    if(buf[9] == 'f') {
    	motor_cnt -= 5;
    }*/

     /* Send the HTML header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
     */
     /////////netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
     /* Send our HTML page */
     /////////netconn_write(conn, INDEXHTML, sizeof(INDEXHTML)-1, NETCONN_NOCOPY);
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
