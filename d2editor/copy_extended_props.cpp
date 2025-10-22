/*
 * Copy extended fields and properties from twi_1240052515.d2i to rich.d2i
 * Keep rich's header (bits 0-59) and item type (bits 60-91)
 * Copy everything from bit 92 onwards from twi
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

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <rich.d2i> <twi.d2i> <output.d2i>" << endl;
        cerr << "Example: " << argv[0] << " rich.d2i twi_1240052515.d2i rich_extended.d2i" << endl;
        return 1;
    }

    string richPath = argv[1];
    string twiPath = argv[2];
    string outputPath = argv[3];

    // Read rich.d2i
    ifstream richFile(richPath, ios::binary);
    if (!richFile) {
        cerr << "Error: Cannot open " << richPath << endl;
        return 1;
    }
    char richHeader[2];
    richFile.read(richHeader, 2);
    if (richHeader[0] != 'J' || richHeader[1] != 'M') {
        cerr << "Error: Invalid rich.d2i header" << endl;
        return 1;
    }
    vector<uint8_t> richData;
    char byte;
    while (richFile.read(&byte, 1)) {
        richData.push_back((uint8_t)byte);
    }
    richFile.close();

    // Read twi.d2i
    ifstream twiFile(twiPath, ios::binary);
    if (!twiFile) {
        cerr << "Error: Cannot open " << twiPath << endl;
        return 1;
    }
    char twiHeader[2];
    twiFile.read(twiHeader, 2);
    if (twiHeader[0] != 'J' || twiHeader[1] != 'M') {
        cerr << "Error: Invalid twi.d2i header" << endl;
        return 1;
    }
    vector<uint8_t> twiData;
    while (twiFile.read(&byte, 1)) {
        twiData.push_back((uint8_t)byte);
    }
    twiFile.close();

    cout << "Read " << richData.size() << " bytes from " << richPath << endl;
    cout << "Read " << twiData.size() << " bytes from " << twiPath << endl;

    // Convert to bits
    string richBits = bytesToBits(richData);
    string twiBits = bytesToBits(twiData);

    cout << "Rich bits: " << richBits.length() << endl;
    cout << "Twi bits: " << twiBits.length() << endl;

    // Extract sections
    // From rich: header (0-59) + item type (60-91) = bits 0-91
    string richHeaderAndType = richBits.substr(0, 92);
    
    // CRITICAL: Flip bit 21 (isExtended) from 1 (not extended) to 0 (extended)
    // Rich has isExtended=1, but we're adding extended fields, so we need isExtended=0
    if (richHeaderAndType[21] == '1') {
        richHeaderAndType[21] = '0';
        cout << "\n✓ Flipped bit 21 (isExtended) from 1 to 0" << endl;
    }
    
    // From twi: everything from bit 92 onwards (extended + properties)
    string twiExtendedAndProps = twiBits.substr(92);

    cout << "Rich header+type: " << richHeaderAndType.length() << " bits" << endl;
    cout << "Twi extended+props: " << twiExtendedAndProps.length() << " bits" << endl;

    // Combine: rich's header+type + twi's extended+properties
    string newBits = richHeaderAndType + twiExtendedAndProps;
    
    cout << "New total bits: " << newBits.length() << endl;

    // Pad to byte boundary if needed
    while (newBits.length() % 8 != 0) {
        newBits += '0';
    }

    cout << "After padding: " << newBits.length() << " bits = " << (newBits.length() / 8) << " bytes" << endl;

    // Convert to bytes
    vector<uint8_t> newData = bitsToBytes(newBits);

    // Write output file
    ofstream outFile(outputPath, ios::binary);
    if (!outFile) {
        cerr << "Error: Cannot create output file: " << outputPath << endl;
        return 1;
    }

    outFile.write("JM", 2);
    outFile.write((char*)newData.data(), newData.size());
    outFile.close();

    cout << "\n✓ Successfully created: " << outputPath << endl;
    cout << "✓ Output size: " << newData.size() << " bytes" << endl;
    
    // Print hex dump
    cout << "\nHex dump: ";
    for (size_t i = 0; i < min(newData.size(), (size_t)40); i++) {
        printf("%02x", newData[i]);
    }
    if (newData.size() > 40) cout << "...";
    cout << endl;

    // Verify item type
    string itemTypeBits = newBits.substr(60, 32);
    string itemType;
    for (int i = 0; i < 32; i += 8) {
        string charBits = itemTypeBits.substr(i, 8);
        string reversed(charBits.rbegin(), charBits.rend());
        itemType += (char)stoi(reversed, nullptr, 2);
    }
    cout << "\n✓ Item type: '" << itemType << "'" << endl;

    return 0;
}
