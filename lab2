#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

using namespace std;

vector<uint8_t> RLE_Encode(const vector<uint8_t>& data) {
    vector<uint8_t> output;

    if (data.empty()) return output;

    uint8_t current = data[0];
    uint8_t c = 1;

    for (size_t i = 1; i < data.size(); i++) {
        if (data[i] == current && c < 255) c++;
        else {
            output.push_back(c);
            output.push_back(current);
            current = data[i];
            c = 1;
        }
    }

    output.push_back(c);
    output.push_back(current);

    return output;
}

bool RLE_Decode(const vector<uint8_t>& data, vector<uint8_t>& output) {

    for (size_t i = 0; i < data.size(); i += 2) {
        uint8_t c = data[i];
        uint8_t v = data[i + 1];
        for (int j = 0; j < c; j++) output.push_back(v);
    }

    return true;
}

bool read_binary_file(const string& filename, vector<uint8_t>& data) {

    ifstream in(filename, ios::binary);
    if (!in) {
        cerr << "Error: Cannot open input file\n";
        return false;
    }

    data.assign(istreambuf_iterator<char>(in), istreambuf_iterator<char>());
    return true;
}

bool write_binary_file(const string& filename, const vector<uint8_t>& data) {

    ofstream out(filename, ios::binary);
    if (!out) {
        cerr << "Error: Cannot open output file\n";
        return false;
    }

    out.write(reinterpret_cast<const char*>(data.data()),data.size());
    return true;
}

int encode_file(const string& input, const string& output) {

    vector<uint8_t> data;

    if (!read_binary_file(input, data)) return 1;
    vector<uint8_t> encoded = RLE_Encode(data);
    if (!write_binary_file(output, encoded)) return 1;

    cout << "Encoding successful\n";
    cout << "Original size: " << data.size() << " bytes\n";
    cout << "Encoded size:  " << encoded.size() << " bytes\n";

    return 0;
}

int decode_file(const string& input, const string& output) {

    vector<uint8_t> data;

    if (!read_binary_file(input, data)) return 1;
    vector<uint8_t> decoded;
    if (!RLE_Decode(data, decoded)) return 1;
    if (!write_binary_file(output, decoded)) return 1;

    cout << "Decoding successful\n";
    cout << "Decoded size:  " << decoded.size() << " bytes\n";

    return 0;
}

string replace_extension(const string& filename, const string& new_ext) {
    size_t pos = filename.find_last_of('.');
    if (pos == string::npos) return filename + new_ext;
    return filename.substr(0, pos) + new_ext;
}

int main(int argc, char* argv[]) {

    if (argc < 3 || argc > 4) {
        cerr << "Usage:\n";
        cerr << "  rle encode <input_file> [output_file]\n";
        cerr << "  rle decode <input_file> [output_file]\n";
        return 1;
    }

    string mode = argv[1];
    string input = argv[2];
    string output;

    if (argc == 4) {
        output = argv[3];
    } else {
        if (mode == "encode") {
            output = replace_extension(input, ".rle");
        }
        else if (mode == "decode") {
            output = replace_extension(input, ".bin");
        }
        else {
            cerr << "Invalid mode. Use 'encode' or 'decode'.\n";
            return 1;
        }
    }

    if (mode == "encode") {
        return encode_file(input, output);
    }
    else if (mode == "decode") {
        return decode_file(input, output);
    }
    else {
        cerr << "Invalid mode. Use 'encode' or 'decode'.\n";
        return 1;
    }
}
