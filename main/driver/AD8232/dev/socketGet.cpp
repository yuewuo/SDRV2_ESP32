#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

uint8_t nowBeep = 0;

void my_alarm_handler(int a){
    for (int i=0; i<4; ++i) {
        for (int j=0; j<8; ++j) {
            printf("%d", (nowBeep>>j)&0x01);
            fflush(stdout);
        }
    }
}

typedef struct {
    int start;
    int end;
} normB;

#define TIMEWINDOW 1000 // tick
#define DEQUELEN 50
#define THRESHOLD 500 // mV, need to manuelly fit
#define TICKTHRE 50 // tick, 500ms one beep means about 120 per minute, it's too high

normB dequ[DEQUELEN];
int deqs, deqe;

void print(int tick) {
    int last = -1;
    int cnt = 0, sum = 0;
    for (int i=deqs; i != deqe; i = (i+1)%DEQUELEN ) {
        if (last != -1) {
            //printf("%d > ", dequ[i].start - last);
            sum += (dequ[i].start - last);
            ++cnt;
        }
        last = dequ[i].start;
    }
    //printf("%d / %d = %f tick\n", sum, cnt, sum / (double)cnt );
    double bet = sum / (double)cnt / 100;
    //printf("%d ms: %d per minute\n", tick * 10, (int)(60 / bet));
    nowBeep = (int)(60 / bet);
}

void periodicCall(int tick, int val) {
    if (val > THRESHOLD) {
        if (dequ[deqe].start != -1) {
            if (deqe != deqs && dequ[deqe].start -  dequ[(deqe + DEQUELEN - 1) % DEQUELEN].start <= TICKTHRE) { // not allowed
                dequ[deqe].start = dequ[deqe].end = -1;
                return;
            }
            // add this beep into the deque
            dequ[deqe].end = tick;
            for (; deqs != deqe; deqs = (deqs+1)%DEQUELEN ) {
                if (dequ[deqs].start < tick - TIMEWINDOW) {
                    dequ[deqs].start = dequ[deqs].end = -1;
                } else break;
            } // clear timeout data
            deqe = (deqe+1) % DEQUELEN;
            dequ[deqe].start = dequ[deqe].end = -1;
            // if still not have more space
            if (deqe == deqs) {
                deqs = (deqs+1) % DEQUELEN;
            }
            // update the global value
            print(tick);
        }
    } else {
        if (dequ[deqe].start == -1) {
            dequ[deqe].start = tick;
        }
    }
}

void systemInit() {
    deqs = 0;
    deqe = 0;
    dequ[0].start = dequ[0].end = -1;
}

const char connect_ip[] = "192.168.43.78";
const int connect_port = 8232;

void handleConnect(int sClient) {
  //char sendData[] = "Hello? Is there anyone?\n";
  //send(sClient, sendData, strlen(sendData), 0);
  char recvData[4096];
  while (1) {
      int ret = recv(sClient, recvData, sizeof(recvData), 0);
      if(ret>0){
         //recvData[ret] = 0x00;
         //printf("%s\n", recvData);
          int s = 0;
          for (int i=0; i<ret; ++i) {
              if (recvData[i] == '\n') {
                  recvData[i] = '\0';
                  int tick, v;
                  sscanf(recvData+s, "%d%d", &tick, &v);
                  s = i+1;
                  //printf("%d %d\n", tick, v);
                  periodicCall(tick, v);
              }
          }
      }
  }
}




//compile the program like this
//g++ ./SimpleTcpClient.cpp -o SimpleTcpClient && sudo ./SimpleTcpClient ; rm SimpleTcpClient


int main(int argc, char* argv[]) {
  systemInit();

  int sockfd;
  struct sockaddr_in servaddr;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
    exit(0);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(connect_port);
  //servaddr.sin_addr = inet_addr(connect_ip);
  if( inet_pton(AF_INET, connect_ip, &servaddr.sin_addr) <= 0) {
    printf("inet_pton error for %s\n", connect_ip);
    exit(0);
  }

  if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
  }

  struct itimerval t;
  t.it_interval.tv_usec = 30000;
  t.it_interval.tv_sec = 0;
  t.it_value.tv_usec = 0;
  t.it_value.tv_sec = 1;
  if( setitimer( ITIMER_REAL, &t, NULL) < 0 ){
    printf("%s\n", "settimer error.");
    return -1;
  }
  signal( SIGALRM, my_alarm_handler );


  handleConnect(sockfd);

  close(sockfd);
  exit(0);
}
