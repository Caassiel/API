#include <iostream>
#include <fstream>
#include <cstdint>
#include <queue>
#include <vector>
using namespace std;

class BitWriter {
private:
    ofstream &file;
    uint8_t temp;
    int c;

public:
    BitWriter(ofstream &f) : file(f), temp(0), c(0){}

    void WriteBitSequence(uint32_t data, int length){
        for (int i = length - 1; i >= 0; i--){
            int bit = (data >> i) & 1;
            temp = (temp << 1) | bit;
            c++;

            if (c == 8){
                file.put(temp);
                temp = 0;
                c = 0;
            }
        }
    }

    void Padding(){
        if (c > 0){
            temp <<= (8 - c);
            file.put(temp);

            temp = 0;
            c = 0;
        }
    }

    ~BitWriter(){
        Padding();
        file.close();
    }
};

class BitReader {
private:
    ifstream &file;
    uint8_t temp;
    int c;

public:
    BitReader(ifstream &f) : file(f), temp(0), c(0){}

    uint32_t ReadBitSequence(int length){
        uint32_t result = 0;

        for (int i = 0; i < length; i++){
            if (c == 0){
                int byte = file.get();
                temp = static_cast<uint8_t>(byte);
                c = 8;
            }

            int bit = (temp >> 7) & 1;
            temp <<= 1;
            c--;
            result = (result << 1) | bit;
        }

        return result;
    }

    ~BitReader(){
        file.close();
    }
};

struct Node{
    uint32_t freq;
    int symbol;
    int left;
    int right;
};

struct Code{
    uint32_t bits;
    int length;
};

Node tree[512];
Code codes[256];


void build_codes(int node, uint32_t bits, int length){
    if (tree[node].symbol >= 0){
        codes[tree[node].symbol] = {bits, length};
        return;
    }

    build_codes(tree[node].left, bits << 1, length + 1);
    build_codes(tree[node].right, (bits << 1) | 1, length + 1);
}

int build_tree(uint32_t freq[256]){
    struct PQNode{
        uint32_t freq;
        int index;
        bool operator>(const PQNode &o) const { return freq > o.freq; }
    };

    priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;

    int node_count = 0;

    for (int i = 0; i < 256; i++){
        if (freq[i] > 0){
            tree[node_count] = {freq[i], i, -1, -1};
            pq.push({freq[i], node_count});
            node_count++;
        }
    }

    if (pq.size() == 1){
        int i = pq.top().index;
        tree[node_count] = {freq[i], -1, i, -1};
        return node_count;
    }

    while (pq.size() > 1){
        auto a = pq.top();
        pq.pop();

        auto b = pq.top();
        pq.pop();

        tree[node_count] = {
            a.freq + b.freq,
            -1,
            a.index,
            b.index
        };

        pq.push({tree[node_count].freq, node_count});
        node_count++;
    }

    return pq.top().index;
}


void compress(const string &input, const string &output){
    ifstream in(input, ios::binary);
    ofstream out(output, ios::binary);

    uint32_t freq[256] = {0};
    vector<uint8_t> data;
    uint8_t byte;

    while (in.read((char *)&byte, 1)){
        freq[byte]++;
        data.push_back(byte);
    }

    for (int i = 0; i < 256; i++) out.write((char *)&freq[i], sizeof(uint32_t));

    int root = build_tree(freq);
    build_codes(root, 0, 0);
    BitWriter writer(out);

    for (uint8_t b : data) writer.WriteBitSequence(codes[b].bits, codes[b].length);
}

void decompress(const string &input, const string &output){
    ifstream in(input, ios::binary);
    ofstream out(output, ios::binary);

    uint32_t freq[256];

    for (int i = 0; i < 256; i++) in.read((char *)&freq[i], sizeof(uint32_t));
    uint64_t total = 0;
    for (int i = 0; i < 256; i++) total += freq[i];

    int root = build_tree(freq);
    BitReader reader(in);
    int node = root;

    vector<uint8_t> buffer;
    buffer.reserve(65536);

    while (total > 0){
        int bit = reader.ReadBitSequence(1);
        node = (bit == 0) ? tree[node].left : tree[node].right;

        if (tree[node].symbol >= 0){
            buffer.push_back((uint8_t)tree[node].symbol);

            node = root;
            total--;

            if (buffer.size() >= 65536){
                out.write((char*)buffer.data(), buffer.size());
                buffer.clear();
            }
        }
    }

    if (!buffer.empty()) out.write((char*)buffer.data(), buffer.size());
}

uint64_t GetFileSize(const string& filename){
    ifstream file(filename, ios::binary | ios::ate);
    return file.tellg()/1.024;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        cout << "Usage:\n";
        cout << " program encode input output\n";
        cout << " program decode input output\n";
        return 1;
    }

    string mode = argv[1];

    if (mode == "encode"){
        compress(argv[2], argv[3]);
        uint64_t original = GetFileSize(argv[2]);
        uint64_t encoded  = GetFileSize(argv[3]);

        double ratio = (double)encoded / original;

        cout << "Original size: " << original << " bytes\n";
        cout << "Encoded size:  " << encoded  << " bytes\n";
        cout << "Compression ratio: " << ratio << "\n";
    }
    else if (mode == "decode") decompress(argv[2], argv[3]);
    else cout << "Unknown mode\n";

    return 0;
}
