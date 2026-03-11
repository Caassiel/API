#include <iostream>
#include <fstream>
#include <cstdint>
#include <bitset>

using namespace std;

class BitWriter {
private:
    ofstream file;
    uint8_t temp;
    int c;

public:
    BitWriter(const string& filename) : temp(0), c(0){
        file.open(filename, ios::binary);
    }

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
    ifstream file;
    uint8_t temp;
    int c;

public:
    BitReader(const string& filename) : temp(0), c(0){
        file.open(filename, ios::binary);
    }

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

void PrintBits(const string& filename){
    ifstream file(filename, ios::binary);
    uint8_t byte;

    while (file.read(reinterpret_cast<char*>(&byte), 1)){
        for (int i = 0; i < 8; i++){
            cout << ((byte >> i) & 1);
        }
        cout << " ";
    }

    cout << "\n\n";
}

int main(){
    uint32_t a1 = 0b100001111;
    uint32_t a2 = 0b011101110;
    cout << bitset<9>(a1) << "\n";
    cout << bitset<9>(a2 )<< "\n\n";
    {
        BitWriter writer("bits.bin");
        writer.WriteBitSequence(a1, 9);
        writer.WriteBitSequence(a2, 9);
    }

    PrintBits("bits.bin");

    BitReader reader("bits.bin");

    uint32_t b1 = reader.ReadBitSequence(11);
    uint32_t b2 = reader.ReadBitSequence(7);

    cout << bitset<11>(b1) << "\n";
    cout << bitset<7>(b2) << "\n";

    return 0;
}
