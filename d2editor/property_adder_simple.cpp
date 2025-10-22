#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <map>

/*
================================================================================
SIMPLE APPROACH: Append properties to END of file
================================================================================
Instead of trying to understand the complex reversed bitString logic,
just APPEND property bits directly to the end of the file bytes!

This matches how properties are actually stored in the file:
[JM header][item header][item type][existing properties][NEW PROPERTY][end marker]
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

// Create LSB-first binary string
std::string numberToBitsLSB(int num, int width) {
    std::string result;
    for (int i = 0; i < width; i++) {
        result += (num & 1) ? '1' : '0';
        num >>= 1;
    }
    return result;
}

// Convert binary string to bytes
std::vector<uint8_t> bitsToBytes(const std::string& bits) {
    std::vector<uint8_t> result;
    
    // Pad to byte boundary
    std::string padded = bits;
    int extra = bits.length() % 8;
    if (extra != 0) {
        padded += std::string(8 - extra, '0');
    }
    
    // Convert each 8-bit chunk to byte
    for (size_t i = 0; i < padded.length(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; j++) {
            if (padded[i + j] == '1') {
                byte |= (1 << j);  // LSB-first within byte
            }
        }
        result.push_back(byte);
    }
    
    return result;
}

bool addPropertySimple(const std::string& filename, int propId, int value) {
    std::cout << "\n=== SIMPLE APPEND APPROACH ===" << std::endl;
    
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
    
    // Create property bits (LSB-first)
    int rawValue = value + prop->add;
    std::string propIdBits = numberToBitsLSB(propId, 9);
    std::string propValueBits = numberToBitsLSB(rawValue, prop->bits);
    std::string endMarkerBits = numberToBitsLSB(511, 9);
    
    std::string propertyBits = propIdBits + propValueBits + endMarkerBits;
    
    std::cout << "   Property bits (LSB): " << propertyBits << " (" << propertyBits.length() << " bits)" << std::endl;
    
    // Convert to bytes
    std::vector<uint8_t> propertyBytes = bitsToBytes(propertyBits);
    
    std::cout << "   Property bytes: " << propertyBytes.size() << " bytes = ";
    for (auto b : propertyBytes) {
        printf("%02x ", b);
    }
    std::cout << std::endl;
    
    // Append to original file
    bytes.insert(bytes.end(), propertyBytes.begin(), propertyBytes.end());
    
    std::cout << "📦 New file size: " << bytes.size() << " bytes" << std::endl;
    
    // Write output
    std::string outputFile = filename + ".added_simple";
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
    
    if (addPropertySimple(filename, propId, value)) {
        std::cout << "\n🎉 Success!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Failed!" << std::endl;
        return 1;
    }
}
