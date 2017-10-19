#include "lib/lib.h"
#include "lib/duhttputil.h"

static struct DuHttpReceiver duHttpReceiver;
static struct DuHttp duHttp;
void DuHttp_ELOG(const char* str) {
  printf("DuHTTP: %s\n", str);
}

const static char http_html_hdr[]  =
      "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

const static char INDEXHTML[] = "<!DOCTYPE html>"
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
      "</html>\n";

//extern const char INDEXHTML[] asm("_binary_index_html_start");

static void http_server_netconn_serve(struct netconn *conn) {
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    DuHttpReceiver_InBuf(&duHttpReceiver, buf, buflen);
    ESP_LOGI("DuHTTP", "buflen = %d, %d, %d", buflen, duHttpReceiver.queue_write, duHttpReceiver.queue_read);
    ESP_LOGI("DuHTTP", "available size: %d", DuHttpReceiver_AvailableSize(&duHttpReceiver));
    while (DuHttpReceiver_TryReadPack(&duHttpReceiver, &duHttp)) {
      ESP_LOGI("DuHTTP", "Got Pack!");
    }

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
