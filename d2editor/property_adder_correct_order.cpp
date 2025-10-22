#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <algorithm>

/*
================================================================================
CORRECT BIT ORDER IMPLEMENTATION
================================================================================
Theo SUMMERY.MD:
1. numberToBinary: MSB-first
2. reverse: Chuyển thành LSB-first cho property bits
3. byteToBinary/binaryToByte: LSB-first

KHÔNG tạo LSB ngay từ đầu, mà phải:
  MSB-first → reverse → LSB-first
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

// Step 1: Create MSB-first binary string
std::string numberToBinaryMSB(int num, int width) {
    std::string result(width, '0');
    for (int i = width - 1; i >= 0; i--) {
        result[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return result;
}

// Step 2: Reverse to get LSB-first (theo SUMMERY.MD)
std::string numberToPropertyBits(int num, int width) {
    std::string msb = numberToBinaryMSB(num, width);
    std::reverse(msb.begin(), msb.end());  // MSB → LSB
    return msb;  // Now LSB-first
}

// Step 3: Convert binary string to bytes (LSB-first within byte)
std::vector<uint8_t> bitsToBytes(const std::string& bits) {
    std::vector<uint8_t> result;
    
    // Pad to byte boundary
    std::string padded = bits;
    int extra = bits.length() % 8;
    if (extra != 0) {
        padded += std::string(8 - extra, '0');
    }
    
    // Convert each 8-bit chunk to byte (LSB-first within byte)
    for (size_t i = 0; i < padded.length(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; j++) {
            if (padded[i + j] == '1') {
                byte |= (1 << j);  // LSB-first: bit 0 = LSB
            }
        }
        result.push_back(byte);
    }
    
    return result;
}

bool addPropertyCorrectOrder(const std::string& filename, int propId, int value) {
    std::cout << "\n=== CORRECT BIT ORDER IMPLEMENTATION ===" << std::endl;
    
    // Read original file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "❌ Cannot open: " << filename << std::endl;
        return false;
    }
    
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "📁 Original file: " << filename << " (" << bytes.size() << " bytes)" << std::endl;
    
    PropertyInfo* prop = getProperty(propId);
    if (!prop) {
        std::cout << "❌ Unknown property ID: " << propId << std::endl;
        return false;
    }
    
    std::cout << "🎯 Adding: " << prop->name << " (ID=" << propId << ", Value=" << value << ")" << std::endl;
    
    // Create property bits theo đúng thứ tự trong SUMMERY.MD
    int rawValue = value + prop->add;
    
    std::cout << "\n📊 Bit conversion process:" << std::endl;
    
    // Property ID
    std::string propIdMSB = numberToBinaryMSB(propId, 9);
    std::cout << "   ID " << propId << " (MSB): " << propIdMSB << std::endl;
    std::string propIdBits = propIdMSB;
    std::reverse(propIdBits.begin(), propIdBits.end());
    std::cout << "   ID " << propId << " (LSB): " << propIdBits << std::endl;
    
    // Property value
    std::string propValueMSB = numberToBinaryMSB(rawValue, prop->bits);
    std::cout << "   Value " << rawValue << " (MSB): " << propValueMSB << std::endl;
    std::string propValueBits = propValueMSB;
    std::reverse(propValueBits.begin(), propValueBits.end());
    std::cout << "   Value " << rawValue << " (LSB): " << propValueBits << std::endl;
    
    // End marker
    std::string endMarkerMSB = numberToBinaryMSB(511, 9);
    std::cout << "   End marker (MSB): " << endMarkerMSB << std::endl;
    std::string endMarkerBits = endMarkerMSB;
    std::reverse(endMarkerBits.begin(), endMarkerBits.end());
    std::cout << "   End marker (LSB): " << endMarkerBits << std::endl;
    
    std::string propertyBits = propIdBits + propValueBits + endMarkerBits;
    
    std::cout << "\n🔗 Combined property bits: " << propertyBits << " (" << propertyBits.length() << " bits)" << std::endl;
    
    // Convert to bytes (LSB-first within byte)
    std::vector<uint8_t> propertyBytes = bitsToBytes(propertyBits);
    
    std::cout << "📦 Property bytes: " << propertyBytes.size() << " bytes = ";
    for (auto b : propertyBytes) {
        printf("%02x ", b);
    }
    std::cout << std::endl;
    
    // Append to original file
    bytes.insert(bytes.end(), propertyBytes.begin(), propertyBytes.end());
    
    std::cout << "📦 New file size: " << bytes.size() << " bytes" << std::endl;
    
    // Write output
    std::string outputFile = filename + ".correct_order";
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cout << "❌ Cannot create: " << outputFile << std::endl;
        return false;
    }
    
    outFile.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    outFile.close();
    
    std::cout << "✅ Created: " << outputFile << std::endl;
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <file.d2i> <property_id> <value>" << std::endl;
        std::cout << "Example: " << argv[0] << " rich.d2i 79 50" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    int propId = std::atoi(argv[2]);
    int value = std::atoi(argv[3]);
    
    if (addPropertyCorrectOrder(filename, propId, value)) {
        std::cout << "\n🎉 Success!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Failed!" << std::endl;
        return 1;
    }
}
