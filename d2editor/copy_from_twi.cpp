/*
 * Copy structure from twi_1240052515.d2i and replace only the item type with 'u@+'
 * This ensures we use a known working structure
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

// Convert bytes to bits (no prepend - straight conversion)
string bytesToBits(const vector<uint8_t>& bytes) {
    string bits;
    for (uint8_t byte : bytes) {
        for (int i = 0; i < 8; i++) {
            bits += (byte & (1 << i)) ? '1' : '0';
        }
    }
    return bits;
}

// Convert bits to bytes (no prepend - straight conversion)
vector<uint8_t> bitsToBytes(const string& bits) {
    vector<uint8_t> result;
    for (size_t i = 0; i < bits.length(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8 && (i + j) < bits.length(); j++) {
            if (bits[i + j] == '1') {
                byte |= (1 << j);
            }
        }
        result.push_back(byte);
    }
    return result;
}

// Convert number to LSB-first binary string
string numberToBitsLSB(uint32_t number, int bitCount) {
    string msb;
    for (int i = bitCount - 1; i >= 0; i--) {
        msb += (number & (1 << i)) ? '1' : '0';
    }
    // Reverse to get LSB-first
    string lsb(msb.rbegin(), msb.rend());
    return lsb;
}

// Convert string to bits (each char = 8 bits, reversed within each byte)
string stringToBits(const string& str) {
    string bits;
    for (char c : str) {
        bits += numberToBitsLSB((uint8_t)c, 8);
    }
    return bits;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <twi_input.d2i> <output.d2i> [newItemType]" << endl;
        cerr << "Example: " << argv[0] << " twi_1240052515.d2i rich_from_twi.d2i \"u@+ \"" << endl;
        return 1;
    }

    string inputPath = argv[1];
    string outputPath = argv[2];
    string newItemType = (argc > 3) ? argv[3] : "u@+ ";  // Default to rich charm

    if (newItemType.length() != 4) {
        cerr << "Error: Item type must be exactly 4 characters" << endl;
        return 1;
    }

    // Read input file
    ifstream inFile(inputPath, ios::binary);
    if (!inFile) {
        cerr << "Error: Cannot open input file: " << inputPath << endl;
        return 1;
    }

    // Read header
    char header[2];
    inFile.read(header, 2);
    if (header[0] != 'J' || header[1] != 'M') {
        cerr << "Error: Invalid file header" << endl;
        return 1;
    }

    // Read all data
    vector<uint8_t> data;
    char byte;
    while (inFile.read(&byte, 1)) {
        data.push_back((uint8_t)byte);
    }
    inFile.close();

    cout << "Read " << data.size() << " bytes from " << inputPath << endl;

    // Convert to bits
    string bits = bytesToBits(data);
    cout << "Total bits: " << bits.length() << endl;

    // Extract sections
    string headerBits = bits.substr(0, 60);
    string oldItemType = bits.substr(60, 32);
    string rest = bits.substr(92);  // Everything after item type

    cout << "Old item type bits: " << oldItemType << endl;
    
    // Decode old item type
    string oldTypeStr;
    for (int i = 0; i < 32; i += 8) {
        string charBits = oldItemType.substr(i, 8);
        string reversed(charBits.rbegin(), charBits.rend());
        oldTypeStr += (char)stoi(reversed, nullptr, 2);
    }
    cout << "Old item type: '" << oldTypeStr << "'" << endl;

    // Convert new item type to bits
    string newItemTypeBits = stringToBits(newItemType);
    cout << "New item type: '" << newItemType << "'" << endl;
    cout << "New item type bits: " << newItemTypeBits << endl;

    // Construct new bit string
    string newBits = headerBits + newItemTypeBits + rest;
    cout << "New total bits: " << newBits.length() << endl;

    // Pad to byte boundary if needed
    while (newBits.length() % 8 != 0) {
        newBits += '0';
    }

    // Convert to bytes
    vector<uint8_t> newData = bitsToBytes(newBits);
    cout << "New data size: " << newData.size() << " bytes" << endl;

    // Write output file
    ofstream outFile(outputPath, ios::binary);
    if (!outFile) {
        cerr << "Error: Cannot create output file: " << outputPath << endl;
        return 1;
    }

    outFile.write("JM", 2);
    outFile.write((char*)newData.data(), newData.size());
    outFile.close();

    cout << "Successfully created: " << outputPath << endl;
    
    // Print hex dump
    cout << "Hex dump: ";
    for (size_t i = 0; i < min(newData.size(), (size_t)40); i++) {
        printf("%02x", newData[i]);
    }
    if (newData.size() > 40) cout << "...";
    cout << endl;

    return 0;
}
