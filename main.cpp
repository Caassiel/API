#include <iostream>
#include <fstream>
#include <cstdint>
#include <stack>
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

struct Dictionary{
    int prefix;
    uint8_t symbol;
};

const int max_size = 32768;
Dictionary d[max_size];

int d_size;
void initialize_dict() {
    for (int i = 0; i < 256; i++) {
        d[i].prefix = -1;
        d[i].symbol = (uint8_t)i;
    }
    d_size = 256;
}

int Find(int prefix, uint8_t symbol) {
    for (int i = 0; i < d_size; i++)
        if (d[i].prefix == prefix && d[i].symbol == symbol)
            return i;
    return -1;
}

void compress(const string &input, const string &output, int max_bits, int overflow_mode) {
    ifstream in(input, ios::binary);
    ofstream out(output, ios::binary);

    out.put((uint8_t)max_bits);
    out.put((uint8_t)overflow_mode);

    int max_size = 1 << max_bits;
    int reset_code = max_size - 1;

    initialize_dict(); // 1, 2
    BitWriter writer(out);
    uint8_t byte;

    if (!in.read((char*)&byte, 1)) return;

    int prefix = byte;

    while (in.read((char*)&byte, 1)) { // 3
        int found = Find(prefix, byte); // 4

        if (found != -1) { // 5
            prefix = found; // 6
        } else { // 7
            writer.WriteBitSequence(prefix, max_bits); // 8

            if (d_size < max_size) {
                int limit = (overflow_mode == 1) ? max_size - 1 : max_size;

                if (d_size < limit) {
                    d[d_size].prefix = prefix;
                    d[d_size].symbol = byte;
                    d_size++;
                }
            } else {
                if (overflow_mode == 1) {
                    writer.WriteBitSequence(reset_code, max_bits); // 9
                    initialize_dict();
                }
            }
            prefix = byte; //10
        }
    }
    writer.WriteBitSequence(prefix, max_bits); // 11
}

uint8_t decode_string(int code, ofstream &out) {
    static uint8_t stack[max_size];
    int top = 0;

    while (code != -1 && d[code].prefix != -1) {
        stack[top++] = d[code].symbol;
        code = d[code].prefix;
    }

    stack[top++] = d[code].symbol;
    uint8_t first = stack[top - 1];
    for (int i = top - 1; i >= 0; i--)
        out.put(stack[i]);

    return first;
}

void decompress(const string &input, const string &output) {
    ifstream in(input, ios::binary);
    ofstream out(output, ios::binary);

    int max_bits    = (uint8_t)in.get();
    int overflow_mode = (uint8_t)in.get();

    int max_size   = 1 << max_bits;
    int reset_code = max_size - 1;

    initialize_dict(); // 1

    BitReader reader(in); // 2

    int prev_code = reader.ReadBitSequence(max_bits);
    if (prev_code < 0 || prev_code > 255) return;
    out.put((uint8_t)prev_code); // 3

    while (true) { // 4
        int code = reader.ReadBitSequence(max_bits);

        if (overflow_mode == 1 && code == reset_code) { // 5
            initialize_dict();
            prev_code = reader.ReadBitSequence(max_bits);
            out.put((uint8_t)prev_code);
            continue;
        }

        uint8_t first_byte = 0;

        if (code < d_size) { // 6
            first_byte = decode_string(code, out);
        } else if (code == d_size) {
            first_byte = decode_string(prev_code, out);
            out.put(first_byte);
        }

        int limit = (overflow_mode == 1) ? max_size - 1 : max_size;

        if (d_size < limit) {
            d[d_size].prefix = prev_code;
            d[d_size].symbol = first_byte;
            d_size++;
        }

        prev_code = code;
    }
}

uint64_t GetFileSize(const string& filename){
    ifstream file(filename, ios::binary | ios::ate);
    return file.tellg()/1.024;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        cout << "Usage: program encode input output <max_bits> <overflow_mode>\n";
        cout << "  max_bits:      9 to 15\n";
        cout << "  overflow_mode: 0 = freeze, 1 = clear\n";
        return 1;
    }

    string mode = argv[1];

    if (mode == "encode") {
        if (argc != 6) {
            cout << "Usage: program encode input output <max_bits> <overflow_mode>\n";
            cout << "  max_bits:      9 to 15\n";
            cout << "  overflow_mode: 0 = freeze, 1 = clear\n";
            return 1;
        }

        int max_bits      = stoi(argv[4]);
        int overflow_mode = stoi(argv[5]);

        if (max_bits < 9 || max_bits > 15) {
            cout << "Error: max_bits must be between 9 and 15\n";
            return 1;
        }
        if (overflow_mode != 0 && overflow_mode != 1) {
            cout << "Error: overflow_mode must be 0 (freeze) or 1 (clear)\n";
            return 1;
        }

        compress(argv[2], argv[3], max_bits, overflow_mode);

        uint64_t original = GetFileSize(argv[2]);
        uint64_t encoded  = GetFileSize(argv[3]);
        double ratio = (double)encoded / original;

        cout << "Original size:     " << original << " bytes\n";
        cout << "Encoded size:      " << encoded  << " bytes\n";
        cout << "Compression ratio: " << ratio    << "\n";
        cout << "Max bits:          " << max_bits      << " (dictionary up to " << (1 << max_bits) << " entries)\n";
        cout << "Overflow mode:     " << (overflow_mode == 0 ? "freeze" : "clear") << "\n";
    }
    else if (mode == "decode") {
        if (argc != 4) {
            cout << "Usage: program decode input output\n";
            return 1;
        }
        decompress(argv[2], argv[3]);
    }
    else {
        cout << "Unknown mode: " << mode << "\n";
        cout << "Usage:\n";
        cout << "  program encode input output <max_bits> <overflow_mode>\n";
        cout << "  program decode input output\n";
        return 1;
    }

    return 0;
}
