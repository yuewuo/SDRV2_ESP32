#include <iostream>
#include <fstream>
using namespace std;
ifstream data("rawdata.txt");
ifstream nextdata("rawdata.txt");
ifstream nextnextdata("rawdata.txt");

#define TOMATHEMATICA false
#define THIRDROUND

int biast = 720;

int main() {
    int v, t;
    string st;
    if (TOMATHEMATICA) cout << "{";
    int firstT = -1;
    while (data >> t >> v) {
        if (firstT == -1) firstT = t;
    //while (data >> t >> v >> st) {
        if (TOMATHEMATICA) printf("{%d,%d},", t, v);
        else printf("%d %d\n", t, v);
        if (t - firstT >= biast) {
            int bias = 3 + biast;
            while (nextdata >> t >> v) {
                if (TOMATHEMATICA) printf("{%d,%d},", t + bias, v);
                else printf("%d %d\n", t + bias, v);
                #ifdef THIRDROUND
                if (t - firstT >= biast) {
                    int bias = 3 + 2 * biast;
                    while (nextnextdata >> t >> v) {
                        if (TOMATHEMATICA) printf("{%d,%d},", t + bias, v);
                        else printf("%d %d\n", t + bias, v);
                    }
                    break;
                }
                #endif
            }
            break;
        }
    }
    if (TOMATHEMATICA) cout << "}";
}
