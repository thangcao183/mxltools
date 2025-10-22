#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <algorithm>

/*
================================================================================
CONVERT NON-EXTENDED ITEM TO EXTENDED WITH PROPERTIES
================================================================================
Đây là tool chính xác để convert một item không có extended fields
thành extended item với properties.

Dựa trên phân tích chi tiết của ItemParser.cpp:
- Header: 60 bits
- Item type: 32 bits
- Extended fields (nếu isExtended=true): ~48 bits
- Properties: variable
- End marker: 9 bits
- Padding: align to byte boundary

Cách dùng:
  ./convert_to_extended <input.d2i> <output.d2i> <prop_id> <value>
================================================================================
*/

struct PropertyInfo {
    int id;
    int add;
    int bits;
    int paramBits;
    std::string name;
};

std::map<int, PropertyInfo> properties = {
    {79, {79, 100, 9, 0, "Gold Find"}},
    {80, {80, 100, 9, 0, "Magic Find"}},
};

PropertyInfo* getProperty(int id) {
    return properties.count(id) ? &properties[id] : nullptr;
}

// Convert bytes to bit string (LSB-first within byte)
// NO PREPEND - read bytes in order
std::string bytesToBits(const std::vector<uint8_t>& bytes) {
    std::string result;
    for (auto byte : bytes) {
        for (int i = 0; i < 8; i++) {
            result += (byte & (1 << i)) ? '1' : '0';
        }
    }
    return result;
}

// Convert bit string to bytes (LSB-first within byte, auto-pad)
// NO PREPEND - write bytes in order
std::vector<uint8_t> bitsToBytes(const std::string& bits) {
    std::string padded = bits;
    
    // Pad to byte boundary
    while (padded.length() % 8 != 0) {
        padded += '0';
    }
    
    std::vector<uint8_t> result;
    
    // Convert each 8-bit chunk to byte
    for (size_t i = 0; i < padded.length(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; j++) {
            if (padded[i + j] == '1') {
                byte |= (1 << j);
            }
        }
        result.push_back(byte);  // NO prepend
    }
    
    return result;
}

// Convert number to MSB-first binary string
std::string numberToBitsMSB(int num, int width) {
    std::string result(width, '0');
    for (int i = width - 1; i >= 0; i--) {
        result[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return result;
}

// Convert number to LSB-first binary string (MSB → reverse → LSB)
std::string numberToBitsLSB(int num, int width) {
    std::string msb = numberToBitsMSB(num, width);
    std::reverse(msb.begin(), msb.end());
    return msb;
}

bool convertToExtended(const std::string& inputFile, const std::string& outputFile,
                      int propId, int propValue, int guid, int ilvl, int quality) {
    
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "CONVERTING TO EXTENDED WITH PROPERTIES" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    // Read input file
    std::ifstream file(inputFile, std::ios::binary);
    if (!file) {
        std::cout << "❌ Cannot open: " << inputFile << std::endl;
        return false;
    }
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), 
                              std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "\n📁 Input: " << inputFile << " (" << data.size() << " bytes)" << std::endl;
    
    if (data.size() < 2 || data[0] != 'J' || data[1] != 'M') {
        std::cout << "❌ Invalid D2I file (missing JM header)" << std::endl;
        return false;
    }
    
    // Split JM header and item data
    std::vector<uint8_t> jmHeader(data.begin(), data.begin() + 2);
    std::vector<uint8_t> itemData(data.begin() + 2, data.end());
    
    // Convert to bits
    std::string bits = bytesToBits(itemData);
    
    std::cout << "   Original bits: " << bits.length() << " bits" << std::endl;
    std::cout << "   Hex: ";
    for (auto b : data) printf("%02x", b);
    std::cout << std::endl;
    
    // Step 1: Check and flip isExtended bit (bit 21)
    std::cout << "\n--- Step 1: Flip isExtended bit (bit 21) ---" << std::endl;
    
    if (bits.length() < 92) {
        std::cout << "❌ File too short (need at least 92 bits)" << std::endl;
        return false;
    }
    
    bool isExtended = (bits[21] == '0');
    std::cout << "   Current isExtended bit = " << bits[21] 
              << " → isExtended = " << isExtended << std::endl;
    
    if (isExtended) {
        std::cout << "⚠️  Item is already extended! This may corrupt the file." << std::endl;
        std::cout << "   Proceeding anyway..." << std::endl;
    }
    
    // Flip to extended
    bits[21] = '0';
    std::cout << "   After flip: bit 21 = " << bits[21] << " → isExtended = true" << std::endl;
    
    // Step 2: Insert extended fields after bit 92
    std::cout << "\n--- Step 2: Insert extended fields after bit 92 ---" << std::endl;
    
    std::string extendedFields;
    extendedFields += numberToBitsLSB(0, 3);           // socketablesNumber = 0
    extendedFields += numberToBitsLSB(guid, 32);       // guid
    extendedFields += numberToBitsLSB(ilvl, 7);        // ilvl
    extendedFields += numberToBitsLSB(quality, 4);     // quality
    extendedFields += numberToBitsLSB(0, 1);           // hasMultiGraphics = 0
    extendedFields += numberToBitsLSB(0, 1);           // autoPrefix = 0
    
    std::cout << "   Extended fields (" << extendedFields.length() << " bits):" << std::endl;
    std::cout << "     socketablesNumber = 0" << std::endl;
    std::cout << "     guid = " << guid << std::endl;
    std::cout << "     ilvl = " << ilvl << std::endl;
    std::cout << "     quality = " << quality << std::endl;
    std::cout << "     hasMultiGraphics = 0" << std::endl;
    std::cout << "     autoPrefix = 0" << std::endl;
    
    // Insert after bit 92 (after item type)
    bits = bits.substr(0, 92) + extendedFields + bits.substr(92);
    
    std::cout << "   After insertion: " << bits.length() << " bits" << std::endl;
    
    // Step 2.5: Insert quality-specific fields (IMPORTANT!)
    std::cout << "\n--- Step 2.5: Insert quality-specific fields ---" << std::endl;
    
    std::string qualityFields;
    int insertPos = 92 + extendedFields.length();
    
    if (quality == 4) {  // Magic
        std::cout << "   Quality = MAGIC → insert magicPrefix + magicSuffix" << std::endl;
        qualityFields += numberToBitsLSB(0, 11);  // magicPrefix = 0 (none)
        qualityFields += numberToBitsLSB(0, 11);  // magicSuffix = 0 (none)
        std::cout << "     magicPrefix (11 bits) = 0" << std::endl;
        std::cout << "     magicSuffix (11 bits) = 0" << std::endl;
    }
    else if (quality == 5) {  // Set
        std::cout << "   Quality = SET → insert setId" << std::endl;
        qualityFields += numberToBitsLSB(0, 12);  // setId = 0
        std::cout << "     setId (12 bits) = 0" << std::endl;
    }
    else if (quality == 6 || quality == 8) {  // Rare or Crafted
        std::cout << "   Quality = RARE/CRAFTED → insert rare names + affixes" << std::endl;
        qualityFields += numberToBitsLSB(0, 8);   // rareName1 = 0
        qualityFields += numberToBitsLSB(0, 8);   // rareName2 = 0
        // 6 prefixes (all 0 = none)
        for (int i = 0; i < 6; i++) {
            qualityFields += '0';  // No prefix
        }
        // 6 suffixes (all 0 = none)
        for (int i = 0; i < 6; i++) {
            qualityFields += '0';  // No suffix
        }
        std::cout << "     rareName1 (8 bits) = 0" << std::endl;
        std::cout << "     rareName2 (8 bits) = 0" << std::endl;
        std::cout << "     6 prefixes = none" << std::endl;
        std::cout << "     6 suffixes = none" << std::endl;
    }
    else if (quality == 7) {  // Unique
        std::cout << "   Quality = UNIQUE → insert uniqueId" << std::endl;
        qualityFields += numberToBitsLSB(0, 12);  // uniqueId = 0
        std::cout << "     uniqueId (12 bits) = 0" << std::endl;
    }
    else {
        std::cout << "   Quality = " << quality << " → no quality-specific fields" << std::endl;
    }
    
    if (!qualityFields.empty()) {
        bits = bits.substr(0, insertPos) + qualityFields + bits.substr(insertPos);
        insertPos += qualityFields.length();
        std::cout << "   Quality fields inserted: " << qualityFields.length() << " bits" << std::endl;
        std::cout << "   After insertion: " << bits.length() << " bits" << std::endl;
    }
    
    // Step 3: Insert property
    std::cout << "\n--- Step 3: Insert property ---" << std::endl;
    
    PropertyInfo* prop = getProperty(propId);
    if (!prop) {
        std::cout << "❌ Unknown property ID: " << propId << std::endl;
        return false;
    }
    
    int rawValue = propValue + prop->add;
    
    std::cout << "   Property: " << prop->name << std::endl;
    std::cout << "   ID: " << propId << " (9 bits)" << std::endl;
    std::cout << "   Value: " << propValue << " + " << prop->add 
              << " = " << rawValue << " (" << prop->bits << " bits)" << std::endl;
    
    // Create property bits (MSB → reverse → LSB)
    std::string propIdBits = numberToBitsLSB(propId, 9);
    std::string propValueBits = numberToBitsLSB(rawValue, prop->bits);
    std::string endMarkerBits = numberToBitsLSB(511, 9);
    
    std::string propertyBits = propIdBits + propValueBits + endMarkerBits;
    
    std::cout << "   Property bits (" << propertyBits.length() << " bits):" << std::endl;
    std::cout << "     ID (LSB): " << propIdBits << std::endl;
    std::cout << "     Value (LSB): " << propValueBits << std::endl;
    std::cout << "     End marker (LSB): " << endMarkerBits << std::endl;
    
    // Insert after all previous fields
    bits = bits.substr(0, insertPos) + propertyBits + bits.substr(insertPos);
    
    std::cout << "   Properties inserted at bit " << insertPos << std::endl;
    std::cout << "   After insertion: " << bits.length() << " bits" << std::endl;
    
    // Step 4: Byte-align
    std::cout << "\n--- Step 4: Byte-align ---" << std::endl;
    
    int paddingNeeded = (8 - (bits.length() % 8)) % 8;
    std::cout << "   Padding needed: " << paddingNeeded << " bits" << std::endl;
    
    // Convert to bytes (auto-pads)
    std::vector<uint8_t> newItemData = bitsToBytes(bits);
    
    std::cout << "   New item data: " << newItemData.size() << " bytes" << std::endl;
    
    // Reconstruct full file
    std::vector<uint8_t> newFile = jmHeader;
    newFile.insert(newFile.end(), newItemData.begin(), newItemData.end());
    
    std::cout << "\n📦 Final file: " << newFile.size() << " bytes" << std::endl;
    std::cout << "   Hex: ";
    for (auto b : newFile) printf("%02x", b);
    std::cout << std::endl;
    
    // Write output
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cout << "❌ Cannot create: " << outputFile << std::endl;
        return false;
    }
    
    outFile.write(reinterpret_cast<const char*>(newFile.data()), newFile.size());
    outFile.close();
    
    std::cout << "\n✅ Saved: " << outputFile << std::endl;
    
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "D2I Item Converter - Non-Extended → Extended with Properties" << std::endl;
    
    if (argc < 5) {
        std::cout << "\nUsage: " << argv[0] << " <input.d2i> <output.d2i> <property_id> <value> [guid] [ilvl] [quality]" << std::endl;
        std::cout << "\nExample: " << argv[0] << " rich.d2i rich_extended.d2i 79 50 999999999 50 4" << std::endl;
        std::cout << "\nDefaults:" << std::endl;
        std::cout << "  guid = 999999999" << std::endl;
        std::cout << "  ilvl = 50" << std::endl;
        std::cout << "  quality = 4 (magic)" << std::endl;
        std::cout << "\nCommon property IDs:" << std::endl;
        std::cout << "  79 = Gold Find (+100 base)" << std::endl;
        std::cout << "  80 = Magic Find (+100 base)" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    int propId = std::atoi(argv[3]);
    int propValue = std::atoi(argv[4]);
    
    // Optional parameters with defaults
    int guid = (argc > 5) ? std::atoi(argv[5]) : 999999999;
    int ilvl = (argc > 6) ? std::atoi(argv[6]) : 50;
    int quality = (argc > 7) ? std::atoi(argv[7]) : 4;  // 4 = magic
    
    if (convertToExtended(inputFile, outputFile, propId, propValue, guid, ilvl, quality)) {
        std::cout << "\n🎉 SUCCESS!" << std::endl;
        std::cout << "\nNext step: Load " << outputFile << " in MedianXLOfflineTools to verify" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ FAILED!" << std::endl;
        return 1;
    }
}
