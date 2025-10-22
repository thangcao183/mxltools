#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <map>

/*
================================================================================
= Phân tích và Logic mới v3 (dựa trên ItemParser.cpp của code chính)
================================================================================
1.  **Vấn đề của v2**:
    - Lỗi corrupt file (Segfault) xảy ra vì việc tìm `endMarker` quá đơn giản.
    - Chuỗi bit `111111111` có thể xuất hiện ngẫu nhiên trong header của item,
      dẫn đến việc chèn thuộc tính sai vị trí và phá vỡ cấu trúc file.

2.  **Hướng giải quyết triệt để (Logic v3)**:
    - **Mô phỏng `ItemParser`**: Cách duy nhất để làm đúng là phải đọc qua
      header của item một cách chính xác để biết khối thuộc tính bắt đầu từ đâu.
    - **`findPropertyBlockStart` thông minh hơn**:
      - Hàm này sẽ được viết lại để đọc và bỏ qua (skip) từng trường dữ liệu
        trong header, giống hệt cách `ItemParser::parseItem` làm.
      - Nó sẽ đọc các cờ quan trọng như `isExtended`, `quality`, `isSocketed`,
        `isPersonalized`, `isRW`.
      - Dựa vào các cờ này, nó sẽ bỏ qua các khối dữ liệu có độ dài thay đổi
        (ví dụ: tên đồ rare/crafted, thông tin runeword).
      - Vị trí cuối cùng mà con trỏ đọc bit dừng lại chính là điểm bắt đầu
        của khối thuộc tính.
    - **Logic chèn thuộc tính chính xác**:
      - Lấy vị trí bắt đầu khối thuộc tính (`propBlockStartPos`).
      - Tách chuỗi bit gốc thành 2 phần: `header` (mọi thứ trước `propBlockStartPos`)
        và `propertiesAndPadding` (phần còn lại).
      - Tìm `endMarker` trong `propertiesAndPadding`.
      - Chèn thuộc tính mới vào ngay trước `endMarker`.
      - Nối lại: `header` + `thuộc tính đã sửa` + `padding mới`.
================================================================================
*/

// Property database từ props.tsv
struct PropertyInfo {
    int id;
    int add;
    int bits;
    int paramBits;
    std::string name;
};

// Một số properties quan trọng
std::map<int, PropertyInfo> properties = {
    {0, {0, 200, 11, 0, "Strength"}},
    {1, {1, 200, 11, 0, "Energy"}},
    {2, {2, 200, 11, 0, "Dexterity"}},
    {3, {3, 200, 11, 0, "Vitality"}},
    {7, {7, 500, 12, 0, "Life"}},
    {79, {79, 100, 9, 0, "Gold Find"}},
    {80, {80, 100, 9, 0, "Magic Find"}},
    {93, {93, 150, 9, 0, "Attack Speed"}},
    {127, {127, 0, 5, 0, "All Skills"}},
    {511, {511, 0, 0, 0, "END_MARKER"}}
};

PropertyInfo* getProperty(int id) {
    if (properties.count(id)) {
        return &properties[id];
    }
    return nullptr;
}

// Convert number to binary string (LSB-first) for writing property values
std::string numberToBinaryLSB(int number, int bits) {
    std::string result = "";
    for (int i = 0; i < bits; i++) {
        result += ((number >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Convert byte to binary string (MSB-first, standard byte representation)
// Each byte is stored in MSB-first format in the bitString
// Example: byte 0b11001010 (0xCA, 202) -> string "11001010"
std::string byteToBinaryString(uint8_t byte) {
    std::string result = "";
    for (int i = 7; i >= 0; i--) {
        result += ((byte >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Convert binary string (MSB-first) to byte
uint8_t binaryStringToByte(const std::string& binary) {
    uint8_t result = 0;
    for (int i = 0; i < 8 && i < binary.length(); i++) {
        if (binary[i] == '1') {
            result |= (1 << (7 - i)); // MSB-first
        }
    }
    return result;
}

// Create ItemParser-style bitString from bytes - CORRECTED PREPEND LOGIC
std::string createBitStringFromBytes(const std::vector<uint8_t>& bytes) {
    std::string bitString = "";
    // Bỏ qua 2 byte header 'JM'
    for (size_t i = 2; i < bytes.size(); ++i) {
        bitString.insert(0, byteToBinaryString(bytes[i]));
    }
    return bitString;
}

// Convert bitString back to bytes - CORRECTED LOGIC v2
// Matching ItemParser::writeItems logic:
// for (int i = 0, n = item->bitString.length(); i < n; i += 8)
//     itemBytes.prepend(item->bitString.mid(i, 8).toShort(0, 2));
std::vector<uint8_t> bitStringToBytes(const std::string& bitString) {
    std::vector<uint8_t> finalBytes;
    finalBytes.push_back('J');
    finalBytes.push_back('M');
    
    // Pad at the END if necessary to make length multiple of 8
    std::string paddedBitString = bitString;
    int extraBits = paddedBitString.length() % 8;
    if (extraBits != 0) {
        paddedBitString.append(std::string(8 - extraBits, '0'));
    }
    
    // Process from left to right, prepending each byte
    // This matches: for (i=0; i<n; i+=8) itemBytes.prepend(...)
    std::vector<uint8_t> dataBytes;
    for (size_t i = 0; i < paddedBitString.length(); i += 8) {
        std::string chunk = paddedBitString.substr(i, 8);
        uint8_t byteValue = binaryStringToByte(chunk);
        dataBytes.insert(dataBytes.begin(), byteValue);  // prepend
    }
    
    finalBytes.insert(finalBytes.end(), dataBytes.begin(), dataBytes.end());
    return finalBytes;
}

// A simple bit reader that mimics ReverseBitReader
// IMPORTANT: This reads from END to BEGINNING, just like ReverseBitReader
class MiniBitReader {
private:
    const std::string& bitString;
    int currentPos; // Position from the END of string (like ReverseBitReader::_pos)

public:
    MiniBitReader(const std::string& s) : bitString(s), currentPos(s.length()) {}

    bool eof() const {
        return currentPos <= 0;
    }

    bool readBool() {
        if (eof()) throw std::runtime_error("Read past end of bitstring");
        currentPos--;
        return bitString[currentPos] == '1';
    }

    int readNumber(int bits) {
        if (currentPos - bits < 0) {
            throw std::runtime_error("Read past end of bitstring");
        }
        currentPos -= bits;
        std::string chunk = bitString.substr(currentPos, bits);
        
        int result = 0;
        // MSB-first reading (Big-Endian), matching Qt's toLongLong behavior
        for (int i = 0; i < bits; ++i) {
            if (chunk[i] == '1') {
                result |= (1 << (bits - 1 - i));
            }
        }
        return result;
    }

    std::string readString(int bits) {
        if (currentPos - bits < 0) {
            throw std::runtime_error("Read past end of bitstring");
        }
        currentPos -= bits;
        return bitString.substr(currentPos, bits);
    }

    void skip(int bits) {
        if (currentPos - bits < 0) {
            throw std::runtime_error("Skip past end of bitstring");
        }
        currentPos -= bits;
    }

    // Returns position from the START (like ReverseBitReader::pos())
    int pos() const {
        return bitString.length() - currentPos;
    }
};

// Finds the starting position of the property block by parsing the item header.
// Returns -1 on failure.
int findPropertyBlockStart(const std::string& bitString) {
    MiniBitReader reader(bitString);
    try {
        reader.skip(1); // isQuest
        reader.skip(3);
        reader.skip(1); // isIdentified
        reader.skip(5);
        reader.skip(1); // is duped
        bool isSocketed = reader.readBool();
        reader.skip(2);
        reader.skip(2); // is illegal equip + unk
        bool isEar = reader.readBool();
        reader.skip(1); // isStarter
        reader.skip(2);
        reader.skip(1);
        bool isExtended = !reader.readBool();
        reader.skip(1); // isEthereal
        reader.skip(1);
        bool isPersonalized = reader.readBool();
        reader.skip(1);
        bool isRW = reader.readBool();
        reader.skip(5);
        reader.skip(8); // version
        reader.skip(2);
        reader.skip(3); // location
        reader.skip(4); // whereEquipped
        reader.skip(4); // column
        reader.skip(4); // row
        reader.skip(3); // storage

        if (isEar) {
            std::cout << "  [Parser] Item is an Ear, no properties." << std::endl;
            return -1;
        }

        std::string itemTypeStr = "";
        for(int i=0; i<4; ++i) itemTypeStr += (char)reader.readNumber(8);
        std::cout << "  [Parser] Item Type: " << itemTypeStr << std::endl;


        if (!isExtended) {
            std::cout << "  [Parser] Item is NOT extended. Properties start at " << reader.pos() << std::endl;
            return reader.pos();
        }

        std::cout << "  [Parser] Item is Extended." << std::endl;
        reader.skip(3); // socketablesNumber
        reader.skip(32); // guid
        reader.skip(7); // ilvl
        int quality = reader.readNumber(4);
        std::cout << "  [Parser] Quality: " << quality << std::endl;

        if (reader.readBool()) reader.skip(3); // variableGraphicIndex
        if (reader.readBool()) reader.skip(11); // autoprefix

        switch (quality) {
            case 1: break; // Normal
            case 2: case 3: reader.skip(3); break; // Low/High quality
            case 4: reader.skip(22); break; // Magic
            case 5: case 7: reader.skip(15); break; // Set/Unique
            case 6: case 8: // Rare/Crafted
                reader.skip(16);
                for (int i = 0; i < 6; ++i) if (reader.readBool()) reader.skip(11);
                break;
            case 9: reader.skip(16); break; // Honorific
        }

        if (isRW) reader.skip(16);
        if (isPersonalized) {
            for (int i = 0; i < 16; ++i) {
                if (reader.readNumber(7) == 0) break;
            }
        }

        // This is a major simplification. The real parser checks item type to see if these fields exist.
        // We assume they don't for a simple item like a ring or amulet.
        // For armor/weapons, this will fail.
        // bool isArmor = ...;
        // if (isArmor) reader.skip(11); // defense
        // if (isArmor || isWeapon) {
        //     reader.skip(8); // max dura
        //     reader.skip(8); // current dura
        // }
        // if (isStackable) reader.skip(9);

        if (isSocketed) {
            reader.skip(4); // socketsNumber
        }

        if (quality == 5) { // Set
            for (int i = 0; i < 5; ++i) if (reader.readBool()) {}
        }

        std::cout << "  [Parser] Properties start at bit: " << reader.pos() << std::endl;
        return reader.pos();

    } catch (const std::exception& e) {
        std::cerr << "  [Parser] Error during header parsing: " << e.what() << std::endl;
        return -1;
    }
}


bool addPropertyToFile_v2(const std::string& filename, int propId, int value) {
    // Step 1: Read original file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "❌ Cannot open file: " << filename << std::endl;
        return false;
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::cout << "\n=== ADDING PROPERTY (v3 LOGIC) ===" << std::endl;
    std::cout << "📁 File: " << filename << " (" << bytes.size() << " bytes)" << std::endl;

    // Get property info
    PropertyInfo* prop = getProperty(propId);
    if (!prop) {
        std::cout << "❌ Unknown property ID: " << propId << std::endl;
        return false;
    }
    std::cout << "🎯 Adding Property: " << prop->name << " (ID=" << propId << ", Value=" << value << ")" << std::endl;

    // Step 2: Create bitString using PREPEND logic
    std::string originalBitString = createBitStringFromBytes(bytes);
    std::cout << "📊 Original bitString: " << originalBitString.length() << " bits (using prepend)" << std::endl;

    // Step 3: Find where the property block starts by parsing the header
    int propBlockStartPos = findPropertyBlockStart(originalBitString);
    if (propBlockStartPos < 0) {
        std::cout << "❌ Failed to parse item header to find property block start." << std::endl;
        return false;
    }

    // Step 4: Split the bitstring into header and properties
    std::string headerPart = originalBitString.substr(0, propBlockStartPos);
    std::string propertiesPart = originalBitString.substr(propBlockStartPos);
    std::cout << "   - Header part: " << headerPart.length() << " bits" << std::endl;
    std::cout << "   - Properties part: " << propertiesPart.length() << " bits" << std::endl;

    // Step 5: Create the new property bit string (already LSB-first from numberToBinaryLSB)
    std::string propIdBits = numberToBinaryLSB(propId, 9);

    int rawValue = value + prop->add;
    std::string propValueBits = numberToBinaryLSB(rawValue, prop->bits);

    std::string newPropertyBits = propIdBits + propValueBits;
    std::cout << "🔧 New property bits (LSB-first): " << newPropertyBits << std::endl;

    // Step 6: Find end marker in the properties part and insert the new property
    std::string endMarker = "111111111";
    size_t endMarkerPos = propertiesPart.find(endMarker);

    std::string newPropertiesPart;
    if (endMarkerPos == std::string::npos) {
        std::cout << "🤔 No end marker found in property block. Appending new property and marker." << std::endl;
        newPropertiesPart = propertiesPart + newPropertyBits + endMarker;
    } else {
        std::cout << "🔍 Found end marker in properties at pos " << endMarkerPos << ". Inserting before it." << std::endl;
        newPropertiesPart = propertiesPart;
        newPropertiesPart.insert(endMarkerPos, newPropertyBits);
    }

    // Step 7: PREPEND new properties to bitString (because of prepend byte logic!)
    // When we prepend to bitString, after conversion with prepend logic, 
    // these bits will appear at the END of the file (where properties belong)
    std::string finalBitString = newPropertiesPart + headerPart + propertiesPart;
    
    // Padding goes at the END of bitString (will be at start of file after prepend conversion)
    int paddingNeeded = (8 - (finalBitString.length() % 8)) % 8;
    if (paddingNeeded > 0) {
        finalBitString += std::string(paddingNeeded, '0');
    }
    std::cout << "🔢 Padding added: " << paddingNeeded << " bits" << std::endl;
    std::cout << "🎯 Final bitString length: " << finalBitString.length() << " bits" << std::endl;

    // Step 8: Convert back to bytes
    std::vector<uint8_t> newBytes = bitStringToBytes(finalBitString);
    std::cout << "📦 New file size: " << newBytes.size() << " bytes" << std::endl;

    // Step 9: Write to new file
    std::string outputFile = filename + ".added_v3";
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cout << "❌ Cannot create output file: " << outputFile << std::endl;
        return false;
    }
    outFile.write(reinterpret_cast<const char*>(newBytes.data()), newBytes.size());
    outFile.close();

    std::cout << "✅ Successfully created: " << outputFile << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <file.d2i> <property_id> <value>" << std::endl;
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  " << argv[0] << " rich.d2i 79 50 3  # Add 50% Gold Find" << std::endl;
        std::cout << "  " << argv[0] << " item.d2i 7 10    # Add +10 Life" << std::endl;
        std::cout << "\nAvailable Properties:" << std::endl;
        for (const auto& pair : properties) {
            if (pair.first != 511) {
                std::cout << "  ID " << pair.first << " = " << pair.second.name << std::endl;
            }
        }
        return 1;
    }

    std::string filename = argv[1];
    int propId = std::stoi(argv[2]);
    int value = std::stoi(argv[3]);

    bool success = addPropertyToFile_v2(filename, propId, value);

    if (success) {
        std::cout << "\n🎉 Property addition completed successfully!" << std::endl;
    } else {
        std::cout << "\n❌ Property addition failed!" << std::endl;
        return 1;
    }

    return 0;
}
