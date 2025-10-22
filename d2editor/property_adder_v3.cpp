#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <map>

/*
================================================================================
FINAL VERSION - Sử dụng logic chính xác của ReverseBitWriter
================================================================================
Sau khi phân tích sâu ItemParser và ReverseBitWriter, tôi hiểu rằng:

1. BitString được tạo bằng PREPEND: byte cuối file -> đầu bitString
2. ReverseBitReader đọc từ cuối bitString về đầu
3. ReverseBitWriter::insert() chèn bits vào vị trí tính từ cuối bitString

Để thêm property:
- Tìm vị trí bắt đầu property block (offset từ đầu bitString)
- Insert property bits VÀO ĐẦU property block đó
- Vì bitString inverse, vị trí "đầu property block" trong bitString
  tương ứng với "cuối property block" trong file logic
================================================================================
*/

// Property database
struct PropertyInfo {
    int id;
    int add;
    int bits;
    int paramBits;
    std::string name;
};

std::map<int, PropertyInfo> properties = {
    {0, {0, 200, 11, 0, "Strength"}},
    {1, {1, 200, 11, 0, "Energy"}},
    {2, {2, 200, 11, 0, "Dexterity"}},
    {3, {3, 200, 11, 0, "Vitality"}},
    {7, {7, 500, 12, 0, "Life"}},
    {79, {79, 100, 9, 0, "Gold Find"}},
    {80, {80, 100, 9, 0, "Magic Find"}},
    {511, {511, 0, 0, 0, "END_MARKER"}}
};

PropertyInfo* getProperty(int id) {
    return properties.count(id) ? &properties[id] : nullptr;
}

// Convert byte to MSB-first binary
std::string byteToBinaryString(uint8_t byte) {
    std::string result = "";
    for (int i = 7; i >= 0; i--) {
        result += ((byte >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Convert MSB-first binary to byte
uint8_t binaryStringToByte(const std::string& binary) {
    uint8_t result = 0;
    for (int i = 0; i < 8 && i < binary.length(); i++) {
        if (binary[i] == '1') {
            result |= (1 << (7 - i));
        }
    }
    return result;
}

// Create MSB-first binary string from number (used for property values inside bitString)
std::string numberToBinaryMSB(int num, int bits) {
    std::string result;
    for (int i = bits - 1; i >= 0; i--) {
        result += ((num >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Create LSB-first binary string from number (NOT used for properties!)
std::string numberToBinaryLSB(int num, int bits) {
    std::string result;
    for (int i = 0; i < bits; i++) {
        result += (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return result;
}

// Create bitString using PREPEND (like ItemParser)
std::string createBitStringFromBytes(const std::vector<uint8_t>& bytes) {
    std::string bitString = "";
    for (size_t i = 2; i < bytes.size(); ++i) {
        bitString.insert(0, byteToBinaryString(bytes[i])); // prepend
    }
    return bitString;
}

// Convert bitString to bytes using PREPEND (like ItemParser::writeItems)
std::vector<uint8_t> bitStringToBytes(const std::string& bitString) {
    std::vector<uint8_t> finalBytes;
    finalBytes.push_back('J');
    finalBytes.push_back('M');
    
    // Pad at END
    std::string paddedBitString = bitString;
    int extraBits = paddedBitString.length() % 8;
    if (extraBits != 0) {
        paddedBitString.append(std::string(8 - extraBits, '0'));
    }
    
    // Process from left to right, prepending each byte
    std::vector<uint8_t> dataBytes;
    for (size_t i = 0; i < paddedBitString.length(); i += 8) {
        std::string chunk = paddedBitString.substr(i, 8);
        uint8_t byteValue = binaryStringToByte(chunk);
        dataBytes.insert(dataBytes.begin(), byteValue);  // prepend
    }
    
    finalBytes.insert(finalBytes.end(), dataBytes.begin(), dataBytes.end());
    return finalBytes;
}

// ReverseBitWriter::insert equivalent - but CORRECT version
// DO NOT use std::string::insert() as it shifts existing bits!
// Instead, reconstruct bitString by concatenating parts
std::string insertBitsCorrect(const std::string& bitString, int offsetWithoutJM, const std::string& bitStringToInsert) {
    // offsetWithoutJM is from the START of logical item (position 0 = first bit after JM header)
    // In bitString (which is reversed), we need to convert this to position from END
    // Formula: insertPos = bitString.length() - offsetWithoutJM
    int insertPos = bitString.length() - offsetWithoutJM;
    
    std::cout << "🔧 Inserting " << bitStringToInsert.length() << " bits at bitString position " 
              << insertPos << " (logical offset " << offsetWithoutJM << " from end)" << std::endl;
    
    // Split bitString into before and after parts
    // Before: bits from 0 to insertPos (exclusive)
    // After: bits from insertPos to end
    std::string before = bitString.substr(0, insertPos);
    std::string after = bitString.substr(insertPos);
    
    std::cout << "   Before: " << before.length() << " bits -> '" << before << "'" << std::endl;
    std::cout << "   Insert: " << bitStringToInsert.length() << " bits -> '" << bitStringToInsert << "'" << std::endl;
    std::cout << "   After:  " << after.length() << " bits -> '" << after << "'" << std::endl;
    
    // Reconstruct: before + inserted + after
    std::string result = before + bitStringToInsert + after;
    std::cout << "   Result: " << result.length() << " bits (should be " << (before.length() + bitStringToInsert.length() + after.length()) << ")" << std::endl;
    return result;
}

// ReverseBitWriter::byteAlignBits equivalent
std::string& byteAlignBits(std::string& bitString) {
    int extraBits = bitString.length() % 8;
    if (extraBits) {
        bitString.append(std::string(8 - extraBits, '0'));
    }
    return bitString;
}

bool addPropertyToFileV3(const std::string& filename, int propId, int value) {
    std::cout << "\n=== ADDING PROPERTY (ReverseBitWriter LOGIC) ===" << std::endl;
    
    // Read file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "❌ Cannot open: " << filename << std::endl;
        return false;
    }
    
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "📁 File: " << filename << " (" << bytes.size() << " bytes)" << std::endl;
    
    PropertyInfo* prop = getProperty(propId);
    if (!prop) {
        std::cout << "❌ Unknown property ID: " << propId << std::endl;
        return false;
    }
    std::cout << "🎯 Property: " << prop->name << " (ID=" << propId << ", Value=" << value << ")" << std::endl;
    
    // Create bitString
    std::string bitString = createBitStringFromBytes(bytes);
    std::cout << "📊 Original bitString: " << bitString.length() << " bits" << std::endl;
    
    // For a simple item (not extended), properties start at bit offset 92 (from logical start)
    // This is: 60 bits (header) + 32 bits (4-char item type) = 92 bits
    // 
    // CRITICAL UNDERSTANDING:
    // File layout (before prepend): [JM][header 60][item type 32][properties...][padding]
    // After prepend (reversed):     [padding][properties...][item type 32][header 60]
    //
    // ReverseBitReader reads from END to BEGINNING of bitString:
    //   - Start at end, read header (60 bits)
    //   - Then read item type (32 bits)  
    //   - Then read properties
    //
    // So in the bitString (reversed):
    //   - Rightmost (end): header
    //   - Before header: item type
    //   - Before item type: properties
    //   - Leftmost (start): padding
    //
    // Original file (96 bits): [padding 4][item type 32][header 60]
    // After adding properties: [padding 4][PROPERTIES 27][item type 32][header 60]
    //
    // Insert position = START of bitString (position 4, after old padding)
    // This way properties come BEFORE item type in the bitString
    
    // Properties should be inserted at the START (after old padding)
    // Old padding is first 4 bits, so insert at position 4
    int oldPaddingBits = bitString.length() % 8;  // Original padding
    if (oldPaddingBits == 0) oldPaddingBits = bitString.length() - 92;  // For 96-bit file, padding = 4
    
    int insertPosInBitString = oldPaddingBits;
    
    std::cout << "🔧 Old padding: " << oldPaddingBits << " bits" << std::endl;
    std::cout << "🔧 Insert position in bitString: " << insertPosInBitString << " (after padding, before item type)" << std::endl;
    
    // Create property bits (LSB-first, like ItemParser appendBits!)
    // ItemParser uses std::reverse() to make LSB-first
    std::string propIdBits = numberToBinaryLSB(propId, 9);
    int rawValue = value + prop->add;
    std::string propValueBits = numberToBinaryLSB(rawValue, prop->bits);
    std::string newPropertyBits = propIdBits + propValueBits;
    
    std::cout << "   Property bits (LSB-first): " << newPropertyBits << " (" << newPropertyBits.length() << " bits)" << std::endl;
    
    // Create end marker (LSB-first)
    std::string endMarkerBits = numberToBinaryLSB(511, 9);
    std::cout << "   End marker: " << endMarkerBits << std::endl;
    
    // For an empty item, we need to:
    // 1. Insert property bits at propertiesStartOffset
    // 2. Insert end marker after property bits
    
    std::string bitsToInsert = newPropertyBits + endMarkerBits;
    std::cout << "   Total bits to insert: " << bitsToInsert.length() << std::endl;
    
    std::cout << "\n🔍 DEBUG: Before insert:" << std::endl;
    std::cout << "   BitString length: " << bitString.length() << std::endl;
    
    // Insert at calculated position (after item type, before header)
    // Split manually instead of using offset calculation
    std::string before = bitString.substr(0, insertPosInBitString);
    std::string after = bitString.substr(insertPosInBitString);
    
    std::cout << "   Split: before=" << before.length() << " bits, after=" << after.length() << " bits" << std::endl;
    std::cout << "   Before (item type + padding): " << before << std::endl;
    std::cout << "   After (header): " << after.substr(after.length() - 20) << "...(showing last 20 bits)" << std::endl;
    
    bitString = before + bitsToInsert + after;
    
    std::cout << "\n🔍 DEBUG: After insert:" << std::endl;
    std::cout << "   BitString length: " << bitString.length() << " bits" << std::endl;
    
    // Save bitString to file for verification
    std::ofstream bitfile("debug_bitstring.txt");
    bitfile << bitString;
    bitfile.close();
    std::cout << "   Saved to: debug_bitstring.txt" << std::endl;
    
    std::cout << "📊 New bitString: " << bitString.length() << " bits" << std::endl;
    
    // Byte align
    byteAlignBits(bitString);
    std::cout << "📊 After byte align: " << bitString.length() << " bits" << std::endl;
    
    // Convert to bytes
    std::vector<uint8_t> newBytes = bitStringToBytes(bitString);
    std::cout << "📦 New file: " << newBytes.size() << " bytes" << std::endl;
    
    // Write file
    std::string outputFile = filename + ".added_v4";
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cout << "❌ Cannot create: " << outputFile << std::endl;
        return false;
    }
    
    outFile.write(reinterpret_cast<const char*>(newBytes.data()), newBytes.size());
    outFile.close();
    
    std::cout << "✅ Created: " << outputFile << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <file.d2i> <property_id> <value>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    int propId = std::stoi(argv[2]);
    int value = std::stoi(argv[3]);
    
    bool success = addPropertyToFileV3(filename, propId, value);
    
    if (success) {
        std::cout << "\n🎉 Success!" << std::endl;
    } else {
        std::cout << "\n❌ Failed!" << std::endl;
        return 1;
    }
    
    return 0;
}
