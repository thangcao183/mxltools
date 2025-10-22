/*
 * Create extended charm from rich.d2i with properties from twi_1240052515.d2i
 * But DON'T copy guid/uniqueId - use 0 instead to avoid name change
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

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <rich.d2i> <twi.d2i> <output.d2i> [quality]" << endl;
        cerr << "quality: 4=magic (default), 7=unique" << endl;
        cerr << "Example: " << argv[0] << " rich.d2i twi_1240052515.d2i rich_magic.d2i 4" << endl;
        return 1;
    }

    string richPath = argv[1];
    string twiPath = argv[2];
    string outputPath = argv[3];
    int quality = (argc > 4) ? atoi(argv[4]) : 4;  // Default to magic

    // Read rich.d2i
    ifstream richFile(richPath, ios::binary);
    if (!richFile) {
        cerr << "Error: Cannot open " << richPath << endl;
        return 1;
    }
    char richHeader[2];
    richFile.read(richHeader, 2);
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

    // Extract sections from rich
    string richHeaderAndType = richBits.substr(0, 92);
    
    // Flip bit 21 (isExtended) from 1 to 0
    if (richHeaderAndType[21] == '1') {
        richHeaderAndType[21] = '0';
        cout << "✓ Flipped bit 21 (isExtended) from 1 to 0" << endl;
    }

    // Create NEW extended fields (don't copy from twi)
    string socketablesNumber = "000";  // 0 (3 bits)
    string guid = string(32, '0');     // 0 (32 bits) - NO UNIQUE NAME
    string ilvl = numberToBitsLSB(99, 7);  // 99 (7 bits)
    string qualityBits = numberToBitsLSB(quality, 4);  // 4 bits
    string hasMultiGraphics = "0";     // 0 (1 bit)
    string autoPrefix = "0";           // 0 (1 bit)

    string extendedFields = socketablesNumber + guid + ilvl + qualityBits + hasMultiGraphics + autoPrefix;
    cout << "✓ Created extended fields (48 bits) with guid=0" << endl;

    // Quality-specific fields
    string qualityFields;
    if (quality == 4) {
        // Magic: prefix + suffix
        string magicPrefix = string(11, '0');   // 0 (11 bits)
        string magicSuffix = string(11, '0');   // 0 (11 bits)
        qualityFields = magicPrefix + magicSuffix;
        cout << "✓ Added magic fields (22 bits): prefix=0, suffix=0" << endl;
    } else if (quality == 7) {
        // Unique: uniqueId
        string uniqueId = string(12, '0');  // 0 (12 bits) - NO SPECIFIC UNIQUE
        qualityFields = uniqueId;
        cout << "✓ Added unique fields (12 bits): uniqueId=0" << endl;
    } else {
        cerr << "Error: Quality " << quality << " not supported yet" << endl;
        return 1;
    }

    // Extract properties from twi (starting at bit 152)
    string twiProperties = twiBits.substr(152);
    cout << "✓ Copied properties from twi (" << twiProperties.length() << " bits)" << endl;

    // Combine everything
    string newBits = richHeaderAndType + extendedFields + qualityFields + twiProperties;

    cout << "\n=== STRUCTURE ===" << endl;
    cout << "  [0-59]   Header (60 bits) - from rich, bit 21 flipped" << endl;
    cout << "  [60-91]  Item type (32 bits) - from rich 'u@+ '" << endl;
    cout << "  [92-139] Extended fields (48 bits) - NEW with guid=0" << endl;
    if (quality == 4) {
        cout << "  [140-161] Magic fields (22 bits) - NEW with prefix=0, suffix=0" << endl;
        cout << "  [162+]   Properties - from twi" << endl;
    } else {
        cout << "  [140-151] Unique ID (12 bits) - NEW with uniqueId=0" << endl;
        cout << "  [152+]   Properties - from twi" << endl;
    }

    // Pad to byte boundary
    while (newBits.length() % 8 != 0) {
        newBits += '0';
    }

    cout << "\nTotal bits: " << newBits.length() << " (" << (newBits.length() / 8) << " bytes)" << endl;

    // Convert to bytes
    vector<uint8_t> newData = bitsToBytes(newBits);

    // Write output
    ofstream outFile(outputPath, ios::binary);
    if (!outFile) {
        cerr << "Error: Cannot create output file" << endl;
        return 1;
    }
    outFile.write("JM", 2);
    outFile.write((char*)newData.data(), newData.size());
    outFile.close();

    cout << "\n✅ Created: " << outputPath << endl;
    cout << "   Quality: " << quality << (quality == 4 ? " (magic)" : " (unique)") << endl;
    cout << "   Hex: ";
    for (size_t i = 0; i < min(newData.size(), (size_t)40); i++) {
        printf("%02x", newData[i]);
    }
    if (newData.size() > 40) cout << "...";
    cout << endl;

    return 0;
}
