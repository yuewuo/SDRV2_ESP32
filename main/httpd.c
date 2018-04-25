#include "autoinclude.h"
#include "shellutil.h"
#include "console/console.h"

// TODO: 改成接收多次的模式，不然收大量数据的时候一定会出错！！！

#define HOSTNAME CONFIG_HOSTNAME

#define EXPIRE_MS 500  // 默认超时时间，单位为ms
static int NO_EXPIRE_ENABLED = 0;  // 当Http请求头部设置了ESP32-Long时不会超时

static struct DuHttpReceiver duHttpReceiver;
static struct DuHttp duHttp;
static struct DuHttp sendDuHttp;
void DuHttp_ELOG(const char* str) {
  printf("DuHTTP Error: %s\n", str);
}

static const char TAG[] = "httpd";

// 添加内嵌的样式和JS库，包含自定义的一些样式，比较大，大约有100KB
extern const char WEUI_MIN_CSS[] asm("_binary_weui_min_css_start");
extern const char WEUI_MIN_CSS_END[] asm("_binary_weui_min_css_end");
extern const char WEUIMY_CSS[] asm("_binary_weuimy_css_start");
extern const char WEUIMY_CSS_END[] asm("_binary_weuimy_css_end");
extern const char WEUI_MIN_JS[] asm("_binary_weui_min_js_start");
extern const char WEUI_MIN_JS_END[] asm("_binary_weui_min_js_end");

extern const char INDEX_HTML[] asm("_binary_index_html_start");
extern const char INDEX_HTML_END[] asm("_binary_index_html_end");
extern const char TEST_HTML[] asm("_binary_test_html_start");
extern const char TEST_HTML_END[] asm("_binary_test_html_end");
extern const char FAVICON_ICO[] asm("_binary_favicon_ico_start");
extern const char FAVICON_ICO_END[] asm("_binary_favicon_ico_end");
extern const char README_HTML[] asm("_binary_readme_html_start");
extern const char README_HTML_END[] asm("_binary_readme_html_end");

// 添加WS2812的基础控制程序
extern const char WS2812_HTML[] asm("_binary_ws2812_html_start");
extern const char WS2812_HTML_END[] asm("_binary_ws2812_html_end");

extern const char ERR_404_HTML[] asm("_binary_404_html_start");
extern const char ERR_404_HTML_END[] asm("_binary_404_html_end");

struct StaticListType{
	const char* key;
	const char* value;
	const char* valueEnd;
	const char* type;
};
#define StaticListLen(x) ((x).valueEnd - (x).value - 1)  // 居然要-1才是真正文件的大小.....不知道为啥
const static struct StaticListType StaticList[] = {
    // 添加基础的css样式和js库
    {"/weui.min.css", WEUI_MIN_CSS, WEUI_MIN_CSS_END, "text/css"},
    {"/weuimy.css", WEUIMY_CSS, WEUIMY_CSS_END, "text/css"},
    {"/weui.min.js", WEUI_MIN_JS, WEUI_MIN_JS_END, "application/x-javascript"},
    // 添加基础的主页
	{"/", INDEX_HTML, INDEX_HTML_END, "text/html"},
    {"/index.html", INDEX_HTML, INDEX_HTML_END, "text/html"},
	{"/404.html", ERR_404_HTML, ERR_404_HTML_END, "text/html"},
	{"/test.html", TEST_HTML, TEST_HTML_END, "text/html"},
    {"/favicon.ico", FAVICON_ICO, FAVICON_ICO_END, "image/x-icon"},
    {"/readme.html", README_HTML, README_HTML_END, "text/html"},
    // 添加WS2812的控制页面
    {"/ws2812.html", WS2812_HTML, WS2812_HTML_END, "text/html"}
};
const static int StaticListLength = (sizeof(StaticList) / sizeof(struct StaticListType));

#define UTILBIAS sizeof("/util/")-1
#define UTILTAG "UTIL"
int duHttpUtilHandler(struct DuHttp* inPack, struct DuHttp* outPack) {
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

//int duHttpHandler(const char* inbuf, int size, char* outbuf, int maxsize) {
int duHttpHandler(char* outbuf, int maxsize, struct netconn *conn) {  // 新增参数用来发送非常非常多的字节
    int returnSize = -1;  //
	//DuHttpReceiver_InBuf(&duHttpReceiver, inbuf, size);
	//printf("    Queue Available Size: %d\n",  DuHttpReceiver_AvailableSize(&duHttpReceiver));
	//while (DuHttpReceiver_TryReadPack(&duHttpReceiver, &duHttp)) {
    if (DuHttpReceiver_TryReadPack(&duHttpReceiver, &duHttp)) {
		printf("Got one Pack!\n");
        char* proxyTo = DuHttp_FindValueByKey(&duHttp, "ESPProxyTo");
        if (DuHttp_FindValueByKey(&duHttp, "ESP32-Long")) {
            ESP_LOGI(TAG, "NO_EXPIRE_ENABLED");
            NO_EXPIRE_ENABLED = 1;  // 链接不会超时
        }
        if ((duHttp.type == DuHttp_Type_GET || duHttp.type == DuHttp_Type_POST)
                && proxyTo != NULL) {
            // do proxy
            int proxyRet;
            if (0 == strcmp(proxyTo, "UART1")) {
                proxyRet = UARTGrpProxy.doProxy(1, &duHttp, 1000 / portTICK_RATE_MS,
                    outbuf, maxsize, &returnSize);
            } else if (0 == strcmp(proxyTo, "UART2")) {
                proxyRet = UARTGrpProxy.doProxy(2, &duHttp, 1000 / portTICK_RATE_MS,
                    outbuf, maxsize, &returnSize);
            }
        } else switch(duHttp.type) {
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
                DuHttp_PushHeadline(&sendDuHttp, "Connection", NO_EXPIRE_ENABLED ? "keep-alive": "close");
                DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/plain");
                DuHttp_EndHeadline(&sendDuHttp);
                DuHttp_PushData(&sendDuHttp, Shell.Out.buffer(), Shell.Out.length());
                returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
            } else for (int i=0; i<StaticListLength; ++i) {
				if (!strcmp(StaticList[i].key, duHttp.ask.requestedURL)) {
					DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
					DuHttp_PushHeadline(&sendDuHttp, "Connection", NO_EXPIRE_ENABLED ? "keep-alive": "close");
					DuHttp_PushHeadline(&sendDuHttp,"Content-Type", StaticList[i].type);
					DuHttp_EndHeadline(&sendDuHttp);
                    sendDuHttp.contentLength = StaticListLen(StaticList[i]);  // 强行更改长度
                    returnSize = DuHttpSendHeadOnly(&sendDuHttp, outbuf, maxsize);  // 不输出内容
                    netconn_write(conn, outbuf, returnSize, NETCONN_NOCOPY);  // 向网络输出头部
                    netconn_write(conn, StaticList[i].value, StaticListLen(StaticList[i]), NETCONN_NOCOPY);  // 向网络输出文件主体
                    returnSize = 0;
					break;
				}
			}
			//do other things
			break;
		case DuHttp_Type_POST:
			printf("Pack Type is: DuHttp_Type_POST\n");
			/*printf("Outputing Key-Value List: cnt=%d\n", duHttp.headlineCount);
			for (int i=0; i<duHttp.headlineCount; ++i) {
				printf("Key: %s\n", duHttp.headline[i].key);
				printf("Value: %s\n", duHttp.headline[i].value);
			}*/
            if (!memcmp(duHttp.ask.requestedURL, "/exec", sizeof("/exec")-1)) {
                // move to console to execute
                Shell.Out.clear();
                duHttp.content[duHttp.contentLength] = 0;
                Console.exec(duHttp.content);
                DuHttp_Initialize_RESPONSE(&sendDuHttp, 200, "OK");
                DuHttp_PushHeadline(&sendDuHttp, "Connection", NO_EXPIRE_ENABLED ? "keep-alive": "close");
                DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/plain");
                DuHttp_EndHeadline(&sendDuHttp);
                DuHttp_PushData(&sendDuHttp, Shell.Out.buffer(), Shell.Out.length());
                returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
            }
			/*va = DuHttp_FindValueByKey(&duHttp, "Tst2");
			if (va) {
				cout << "Find: " << va << endl;
			} else cout << "Not Found" << endl;*/
			duHttp.content[duHttp.contentLength] = 0;
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
        if (returnSize == -1) {
            //return 404 Page
            DuHttp_Initialize_RESPONSE(&sendDuHttp, 404, "Not Found");
            DuHttp_PushHeadline(&sendDuHttp, "Connection", NO_EXPIRE_ENABLED ? "keep-alive": "close");
            DuHttp_PushHeadline(&sendDuHttp,"Content-Type", "text/html");
            DuHttp_EndHeadline(&sendDuHttp);
            sendDuHttp.contentLength += sprintf((sendDuHttp.content) + (sendDuHttp.contentLength), ERR_404_HTML, duHttp.ask.requestedURL, HOSTNAME); //DuHttp_PushData manually
            returnSize = DuHttpSend(&sendDuHttp, outbuf, maxsize);
        }
        DuHttp_Release(&duHttp);
	}
	//printf("    Queue Available Size: %d\n",  DuHttpReceiver_AvailableSize(&duHttpReceiver));
	return returnSize;
}


































const static char http_html_hdr[]  =
      "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

static char sendbuf[8192];
static void http_server_netconn_serve(struct netconn *conn) {
    TickType_t expireAt = xTaskGetTickCount() + EXPIRE_MS / portTICK_PERIOD_MS; // 如果不是认证的链接，500ms后还没有消息则超时，以保证系统的稳定性
    NO_EXPIRE_ENABLED = 0;
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;
    int sendlen;
    /* Read the data from the port, blocking if nothing yet there.
    We assume the request (the part we care about) is in one netbuf */
    // 改成接收多次的模式，不然收大量数据的时候一定会出错！！！
    ESP_LOGI(TAG, "Long Connection Start");
    netconn_set_recvtimeout(conn, EXPIRE_MS);  // 设置超时时间
    err = netconn_recv(conn, &inbuf);
    while (err == ERR_OK || err == ERR_TIMEOUT) {
        if (err == ERR_OK) {
            netbuf_data(inbuf, (void**)&buf, &buflen);
            DuHttpReceiver_InBuf(&duHttpReceiver, buf, buflen);
            /* Delete the buffer (netconn_recv gives us ownership,
            so we have to make sure to deallocate the buffer) */
            netbuf_delete(inbuf);
            while ((sendlen = duHttpHandler(sendbuf, sizeof(sendbuf), conn)) != -1) {
                ESP_LOGI(TAG, "Long Connection Handle one");
                expireAt = xTaskGetTickCount() + EXPIRE_MS / portTICK_PERIOD_MS; // 超时时间从没有收到消息开始，这样浏览器可以连续好几个请求用同一个Tcp链接
                netconn_write(conn, sendbuf, sendlen, NETCONN_NOCOPY);
            }
        }
        if (!NO_EXPIRE_ENABLED && xTaskGetTickCount() > expireAt) break;  // 超时，不再接收数据
        err = netconn_recv(conn, &inbuf);
    }
    ESP_LOGE(TAG, "Long Connection Break");
    /*err = netconn_recv(conn, &inbuf);
    if (err == ERR_OK) {
        netbuf_data(inbuf, (void**)&buf, &buflen);
        sendlen = duHttpHandler(buf, buflen, sendbuf, sizeof(sendbuf));
        netconn_write(conn, sendbuf, sendlen, NETCONN_NOCOPY);
    }*/
    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);
    
}

void httpd(){
    struct netconn *conn, *newconn;
    err_t err;

    DuHttpReceiver_Initialize(&duHttpReceiver);
    DuHttp_Initialize(&duHttp);
    ESP_LOGI("DuHTTP", "available size: %d",
        DuHttpReceiver_AvailableSize(&duHttpReceiver));

    conn = netconn_new(NETCONN_TCP);
    //netconn_set_recvtimeout(conn, EXPIRE_MS);  // 设置超时时间，实际发现经常是netconn_accept卡住导致web服务不可用（后来查出问题是buf没有release....）
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    ESP_LOGI("http_server", "listened");
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while(err == ERR_OK);
    ESP_LOGE("http_server", "err???");
    netconn_close(conn);
    netconn_delete(conn);
}
