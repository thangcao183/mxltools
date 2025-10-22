#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

// Property database từ props.tsv
struct PropertyInfo {
    int id;
    int add;
    int bits;
    int paramBits;
    std::string name;
};

// Một số properties quan trọng
std::vector<PropertyInfo> properties = {
    {0, 200, 11, 0, "Strength"},
    {1, 200, 11, 0, "Energy"}, 
    {2, 200, 11, 0, "Dexterity"},
    {3, 200, 11, 0, "Vitality"},
    {7, 500, 12, 0, "Life"},  // Property ID=7 từ props.tsv
    {79, 100, 9, 0, "Gold Find"},
    {80, 100, 9, 0, "Magic Find"},
    {93, 150, 9, 0, "Attack Speed"},
    {127, 0, 5, 0, "All Skills"},
    {511, 0, 0, 0, "END_MARKER"}
};

PropertyInfo* getProperty(int id) {
    for (auto& prop : properties) {
        if (prop.id == id) return &prop;
    }
    return nullptr;
}

// Convert byte to LSB-first binary string (như ItemParser)
std::string byteToBinary(uint8_t byte) {
    std::string result = "";
    for (int i = 0; i < 8; i++) { // LSB-first: bit 0 trước, bit 7 sau
        result += ((byte >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Convert binary string to byte - LSB-first (match với byteToBinary)
uint8_t binaryToByte(const std::string& binary) {
    uint8_t result = 0;
    for (int i = 0; i < 8 && i < binary.length(); i++) {
        if (binary[i] == '1') {
            result |= (1 << i); // LSB-first: bit 0 ở position 0, bit 7 ở position 7
        }
    }
    return result;
}

// Create ItemParser-style bitString from bytes - FIXED VERSION
std::string createBitString(const std::vector<uint8_t>& bytes) {
    std::string bitString = "";
    
    // ItemParser logic theo research_d2i_structure:
    // Đọc từng byte từ file, convert thành 8-bit binary, kết nối theo thứ tự
    // KHÔNG phải prepend logic, mà là append từ trái sang phải
    
    for (size_t i = 2; i < bytes.size(); i++) { // Skip JM header
        std::string binary = byteToBinary(bytes[i]);
        bitString += binary; // APPEND, không phải prepend
    }
    
    return bitString;
}

// Convert bitString back to bytes - FIXED VERSION
std::vector<uint8_t> bitStringToBytes(const std::string& bitString) {
    std::vector<uint8_t> bytes;
    
    // Add JM header
    bytes.push_back('J');
    bytes.push_back('M');
    
    // Convert bitString to bytes theo thứ tự thẳng (không prepend)
    // Vì createBitString() đã dùng append, nên ngược lại cũng phải thẳng
    
    for (int i = 0; i < bitString.length(); i += 8) {
        std::string chunk = bitString.substr(i, 8);
        if (chunk.length() < 8) {
            chunk += std::string(8 - chunk.length(), '0'); // Pad với zeros nếu thiếu
        }
        uint8_t byteValue = binaryToByte(chunk);
        bytes.push_back(byteValue); // APPEND thay vì prepend
    }
    
    return bytes;
}

// Convert number to binary string
std::string numberToBinary(int number, int bits) {
    std::string result = "";
    for (int i = bits - 1; i >= 0; i--) {
        result += ((number >> i) & 1) ? '1' : '0';
    }
    return result;
}

bool addPropertyToFile(const std::string& filename, int propId, int value) {
    // Step 1: Read original file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "❌ Cannot open file: " << filename << std::endl;
        return false;
    }
    
    std::vector<uint8_t> bytes;
    uint8_t byte;
    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        bytes.push_back(byte);
    }
    file.close();
    
    std::cout << "\n=== ADDING PROPERTY TO FILE ===" << std::endl;
    std::cout << "📁 File: " << filename << " (" << bytes.size() << " bytes)" << std::endl;
    
    // Get property info
    PropertyInfo* prop = getProperty(propId);
    if (!prop) {
        std::cout << "❌ Unknown property ID: " << propId << std::endl;
        return false;
    }
    
    std::cout << "🎯 Adding Property: " << prop->name << " (ID=" << propId 
              << ", Value=" << value << ")" << std::endl;
    
    // Step 2: Create bitString từ file bytes
    std::string originalBitString = createBitString(bytes);
    std::cout << "📊 Original bitString: " << originalBitString.length() << " bits" << std::endl;
    
    // Step 3: Debug và tìm end marker thật
    std::cout << "🔍 Debug: Last 30 bits: \"" << originalBitString.substr(originalBitString.length() - 30) << "\"" << std::endl;
    std::cout << "🔍 Debug: Bits 430-450: \"" << originalBitString.substr(430, 20) << "\"" << std::endl;
    
    // Tìm tất cả vị trí có "111111111" và chọn đúng
    std::string endMarker = "111111111"; // 511 in 9 bits
    std::vector<size_t> allPositions;
    
    size_t pos = 0;
    while ((pos = originalBitString.find(endMarker, pos)) != std::string::npos) {
        allPositions.push_back(pos);
        pos++;
    }
    
    std::cout << "🔍 Found " << allPositions.size() << " occurrences of end marker:" << std::endl;
    for (size_t p : allPositions) {
        std::cout << "   - Position " << p << std::endl;
    }
    
    // Chọn end marker cuối cùng (có lý nhất)
    if (allPositions.empty()) {
        std::cout << "❌ Cannot find any end marker in bitString" << std::endl;
        return false;
    }
    
    size_t endPos = allPositions.back(); // Lấy vị trí cuối cùng
    std::cout << "🔍 Using end marker at position: " << endPos << std::endl;
    
    // Step 4: Structure thực tế: [content][end marker][padding]
    // End marker ở position endPos, vậy:
    int currentContentBits = endPos;  // Content từ 0 đến endPos
    int currentPaddingBits = originalBitString.length() - endPos - 9; // Padding sau end marker
    int totalBits = originalBitString.length();
    
    std::cout << "📏 Analysis:" << std::endl;
    std::cout << "   - Total bitString: " << totalBits << " bits" << std::endl;
    std::cout << "   - End marker position: " << endPos << std::endl;
    std::cout << "   - Current padding: " << currentPaddingBits << " bits" << std::endl;
    std::cout << "   - Current content: " << currentContentBits << " bits" << std::endl;
    std::cout << "   - End marker: 9 bits" << std::endl;
    
    if (currentPaddingBits > 0) {
        std::string currentPadding = originalBitString.substr(endPos + 9, currentPaddingBits);
        std::cout << "🔢 Current padding pattern: \"" << currentPadding << "\"" << std::endl;
    }
    
    // Step 5: Cắt bỏ end marker và padding → lấy pure content
    // Content nằm TRƯỚC end marker: từ 0 đến endPos
    std::string pureContent = originalBitString.substr(0, currentContentBits);
    std::cout << "✂️  Pure content (no end marker/padding): " << pureContent.length() << " bits" << std::endl;
    
    // Step 6: Tạo property bits mới (sử dụng LSB-first encoding như ItemParser)
    int rawValue = value + prop->add; // Add the add value
    
    // Convert to binary và reverse để thành LSB-first (như ItemParser)
    std::string propertyIdBits = numberToBinary(propId, 9);
    std::reverse(propertyIdBits.begin(), propertyIdBits.end()); // LSB-first
    
    std::string propertyValueBits = numberToBinary(rawValue, prop->bits);  
    std::reverse(propertyValueBits.begin(), propertyValueBits.end()); // LSB-first
    
    std::string newPropertyBits = propertyIdBits + propertyValueBits;
    
    std::cout << "🔧 New property bits (LSB-first):" << std::endl;
    std::cout << "   - ID bits (9): " << propertyIdBits << std::endl;
    std::cout << "   - Value bits (" << prop->bits << "): " << propertyValueBits 
              << " (raw=" << rawValue << ")" << std::endl;
    std::cout << "   - Total: " << newPropertyBits.length() << " bits" << std::endl;
    
    // Step 7: Thêm property vào CUỐI pure content (content sẽ nằm sau end marker)
    std::string newContent = pureContent + newPropertyBits;
    
    std::cout << "➕ Content after adding property: " << newContent.length() << " bits" << std::endl;
    
    // Step 8: Chuẩn bị end marker mới (sẽ được thêm vào giữa padding và content)
    std::string newEndMarker = "111111111"; // End marker 511 in LSB-first (cũng là 111111111)
    
    std::cout << "🔚 New end marker ready: " << newEndMarker << std::endl;
    
    // Step 7: Tính padding mới để đạt byte boundary cho toàn bộ: padding + end marker + content
    int totalContentBits = 9 + newContent.length(); // 9 bits end marker + content bits
    int newPaddingNeeded = (8 - (totalContentBits % 8)) % 8;
    std::string newPadding = std::string(newPaddingNeeded, '0');
    
    std::cout << "🔢 New padding needed: " << newPaddingNeeded << " bits" << std::endl;
    if (newPaddingNeeded > 0) {
        std::cout << "📋 New padding pattern: \"" << newPadding << "\"" << std::endl;
    }
    
    // Step 8: Tạo bitString cuối cùng theo cấu trúc thực tế: [content][end marker][padding]
    std::string finalBitString = newContent + newEndMarker + newPadding;
    std::cout << "🎯 Final bitString: " << finalBitString.length() << " bits" << std::endl;
    
    // Step 9: Convert back to bytes
    std::vector<uint8_t> newBytes = bitStringToBytes(finalBitString);
    std::cout << "📦 New file size: " << newBytes.size() << " bytes" << std::endl;
    std::cout << "📈 Size change: " << bytes.size() << " → " << newBytes.size() 
              << " (+" << (newBytes.size() - bytes.size()) << " bytes)" << std::endl;
    
    // Step 10: Write to new file
    std::string outputFile = filename + ".added";
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cout << "❌ Cannot create output file: " << outputFile << std::endl;
        return false;
    }
    
    for (uint8_t b : newBytes) {
        outFile.write(reinterpret_cast<const char*>(&b), 1);
    }
    outFile.close();
    
    std::cout << "✅ Successfully created: " << outputFile << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <file.d2i> <property_id> <value>" << std::endl;
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  " << argv[0] << " amu_2482595313.d2i 7 10   # Add +10 Life" << std::endl;
        std::cout << "  " << argv[0] << " amu_2482595313.d2i 0 20   # Add +20 Strength" << std::endl;
        std::cout << "  " << argv[0] << " amu_2482595313.d2i 80 30  # Add 30% Magic Find" << std::endl;
        std::cout << "\nAvailable Properties:" << std::endl;
        for (const auto& prop : properties) {
            if (prop.id != 511) {
                std::cout << "  ID " << prop.id << " = " << prop.name << std::endl;
            }
        }
        return 1;
    }
    
    std::string filename = argv[1];
    int propId = std::stoi(argv[2]);
    int value = std::stoi(argv[3]);
    
    bool success = addPropertyToFile(filename, propId, value);
    
    if (success) {
        std::cout << "\n🎉 Property addition completed successfully!" << std::endl;
        std::cout << "📋 You can now test the modified file with MedianXL Tools" << std::endl;
    } else {
        std::cout << "\n❌ Property addition failed!" << std::endl;
        return 1;
    }
    
    return 0;
}