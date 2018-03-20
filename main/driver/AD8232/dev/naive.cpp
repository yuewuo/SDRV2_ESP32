#include <iostream>
#include <fstream>
using namespace std;
ifstream data("rawdata.txt");

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
    printf("%d %d\n", tick * 10, (int)(60 / bet));
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

int main() {
    int v, t;
    string st;
    while (data >> t >> v) {
    //while (data >> t >> v >> st) {
        periodicCall(t, v);
    }
}
