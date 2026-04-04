#include <iostream>
#include <random>
#include <cmath>
using namespace std;

static const int t = 50;
static const uint32_t N = 1 << 16;

wstring random_string() {
    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> len_dist(1, t);
    uniform_int_distribution<> char_dist(0x0400, 0x04FF);

    int len = len_dist(gen);

    wstring wstr;
    for (int i = 0; i < len; i++) {
        wstr.push_back((wchar_t)char_dist(gen));
    }

    return wstr;
}

uint32_t random_key() {
    static random_device rd;
    static mt19937 gen(rd());

    uniform_int_distribution<uint32_t> dist(31, 10000);
    return dist(gen);
}

uint16_t polynomial_hash() {
    wstring s = random_string();
    uint32_t p = random_key();
    uint32_t h = 1;

    for (wchar_t c : s)
        h = (h * p + (uint32_t)c) % N;

    return (uint16_t)h;
}

uint32_t check_hash(const wstring& s, uint32_t p) {
    uint32_t h = 0;

    for (wchar_t c : s)
        h = (h * p + (uint32_t)c) % N;

    return h;
}

class Bloom_Filter{

private:
    int s;
    vector<bool> T;
    vector<uint32_t> v;


public:
    Bloom_Filter(int s) : s(s), T(N, false) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<uint32_t> dist(31, 10000);

        for (int i = 0; i < s; i++) {
            v.push_back(dist(gen));
        }
    }

    void Addition(){
        for (uint32_t p : v) {
            uint32_t idx = polynomial_hash();
            T[idx] = true;
        }
    };

    bool DoesContain(const wstring& str){
       for (uint32_t p : v) {
            uint32_t idx = check_hash(str, p);
            if (!T[idx]) return false;
        }
        return true;
    };

    float Experiment(float a, int s) {
        Bloom_Filter bf(s);

        int n = a * N;

        for (int i = 0; i < n; i++)
            bf.Addition();


        int M = 0;
        int tests = 10000;

        for (int i = 0; i < tests; i++) {
            wstring str = random_string();
            if (bf.DoesContain(str)) {
                M = i;
                break;
            }
        }

        return 1 / M;
    }


};


int main() {


    for (float a = 0.05; a <= 0.5; a = a + 0.05){
        float error = 0;

        for (int i = 0; i < 100; i++) {
            int s = floor(log(2)/a);
            Bloom_Filter b(a);
            error += b.Experiment(a, s);
        }

        cout << "Error for alpha = " << a << ":  " << error << "\n";
    }

    return 0;
}
