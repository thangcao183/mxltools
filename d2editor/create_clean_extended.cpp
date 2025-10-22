/*
 * Create a clean extended charm from rich.d2i
 * Extended fields are set but NO properties (just the end marker 511)
 * This creates a "blank" extended item ready for properties to be added later
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
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <rich.d2i> <output.d2i> [quality] [ilvl]" << endl;
        cerr << "quality: 4=magic (default), 7=unique" << endl;
        cerr << "ilvl: item level (default 99)" << endl;
        cerr << "Example: " << argv[0] << " rich.d2i rich_clean.d2i 4 99" << endl;
        return 1;
    }

    string richPath = argv[1];
    string outputPath = argv[2];
    int quality = (argc > 3) ? atoi(argv[3]) : 4;  // Default to magic
    int ilvl = (argc > 4) ? atoi(argv[4]) : 99;    // Default to 99

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

    cout << "Read " << richData.size() << " bytes from " << richPath << endl;

    // Convert to bits
    string richBits = bytesToBits(richData);

    // Extract header and type from rich
    string richHeaderAndType = richBits.substr(0, 92);
    
    // Flip bit 21 (isExtended) from 1 to 0
    if (richHeaderAndType[21] == '1') {
        richHeaderAndType[21] = '0';
        cout << "✓ Flipped bit 21 (isExtended) from 1 to 0" << endl;
    }

    // Create extended fields
    string socketablesNumber = "000";  // 0 (3 bits)
    string guid = string(32, '0');     // 0 (32 bits) - no unique name
    string ilvlBits = numberToBitsLSB(ilvl, 7);  // 7 bits
    string qualityBits = numberToBitsLSB(quality, 4);  // 4 bits
    string hasMultiGraphics = "0";     // 0 (1 bit)
    string autoPrefix = "0";           // 0 (1 bit)

    string extendedFields = socketablesNumber + guid + ilvlBits + qualityBits + hasMultiGraphics + autoPrefix;
    cout << "✓ Created extended fields (48 bits)" << endl;
    cout << "  - guid: 0 (no unique name)" << endl;
    cout << "  - ilvl: " << ilvl << endl;
    cout << "  - quality: " << quality << (quality == 4 ? " (magic)" : quality == 7 ? " (unique)" : "") << endl;

    // Quality-specific fields
    string qualityFields;
    if (quality == 4) {
        // Magic: prefix + suffix (both 0)
        string magicPrefix = string(11, '0');   // 0 (11 bits)
        string magicSuffix = string(11, '0');   // 0 (11 bits)
        qualityFields = magicPrefix + magicSuffix;
        cout << "✓ Added magic fields (22 bits): prefix=0, suffix=0" << endl;
    } else if (quality == 7) {
        // Unique: uniqueId (0)
        string uniqueId = string(12, '0');  // 0 (12 bits)
        qualityFields = uniqueId;
        cout << "✓ Added unique fields (12 bits): uniqueId=0" << endl;
    } else {
        cerr << "Error: Quality " << quality << " not supported" << endl;
        return 1;
    }

    // Add default property: ID=0, value=10
    // Property ID 0 (9 bits LSB-first)
    string propId = numberToBitsLSB(0, 9);  // 000000000
    // Property value 10 (assume 7 bits for now, adjust if needed)
    string propValue = numberToBitsLSB(10, 7);  // 7 bits LSB-first
    string defaultProperty = propId + propValue;
    cout << "✓ Added default property (16 bits): ID=0, value=10" << endl;
    
    // Property end marker (9 bits all 1s = 511)
    string endMarker = "111111111";  // 511 (9 bits)
    cout << "✓ Added property end marker (9 bits): 511" << endl;

    // Combine everything
    string newBits = richHeaderAndType + extendedFields + qualityFields + defaultProperty + endMarker;

    cout << "\n=== STRUCTURE ===" << endl;
    cout << "  [0-59]    Header (60 bits) - from rich, bit 21 flipped" << endl;
    cout << "  [60-91]   Item type (32 bits) - from rich 'u@+ '" << endl;
    cout << "  [92-139]  Extended fields (48 bits) - NEW, guid=0, ilvl=" << ilvl << endl;
    
    if (quality == 4) {
        cout << "  [140-161] Magic fields (22 bits) - prefix=0, suffix=0" << endl;
        cout << "  [162-177] Default property (16 bits) - ID=0, value=10" << endl;
        cout << "  [178-186] Property end marker (9 bits) - 511" << endl;
        cout << "\nTotal: 187 bits (more properties can be inserted before end marker)" << endl;
    } else {
        cout << "  [140-151] Unique ID (12 bits) - uniqueId=0" << endl;
        cout << "  [152-167] Default property (16 bits) - ID=0, value=10" << endl;
        cout << "  [168-176] Property end marker (9 bits) - 511" << endl;
        cout << "\nTotal: 177 bits (more properties can be inserted before end marker)" << endl;
    }

    // Pad to byte boundary
    while (newBits.length() % 8 != 0) {
        newBits += '0';
        cout << "  + padding to byte boundary" << endl;
        break;
    }

    int finalBits = newBits.length();
    int finalBytes = finalBits / 8;
    cout << "\nFinal: " << finalBits << " bits = " << finalBytes << " bytes" << endl;

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

    cout << "\n✅ Created clean extended charm: " << outputPath << endl;
    cout << "   Quality: " << quality << (quality == 4 ? " (magic)" : " (unique)") << endl;
    cout << "   Has default property: ID=0, value=10" << endl;
    cout << "   Ready to add more properties!" << endl;
    
    cout << "\n   Hex: ";
    for (size_t i = 0; i < newData.size(); i++) {
        printf("%02x", newData[i]);
    }
    cout << endl;

    return 0;
}
