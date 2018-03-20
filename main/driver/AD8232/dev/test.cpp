#include <iostream>
#include <fstream>

using namespace std;

ifstream data("data.txt");

int main() {
    int v, t;
    string st;
    cout << "{";
    while (data >> t >> v >> st) {
        cout << "{" << t << "," << v << "}" << ',';
    }
    cout << "}";
}