#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

// Mô phỏng chính xác ItemParser của code gốc

// Convert byte to MSB-first binary string
std::string byteToBinaryString(uint8_t byte) {
    std::string result = "";
    for (int i = 7; i >= 0; i--) {
        result += ((byte >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Create bitString exactly like ItemParser (prepend logic)
std::string createBitStringFromBytes(const std::vector<uint8_t>& bytes) {
    std::string bitString = "";
    // Skip 'JM' header
    for (size_t i = 2; i < bytes.size(); ++i) {
        bitString.insert(0, byteToBinaryString(bytes[i])); // prepend
    }
    return bitString;
}

// ReverseBitReader simulation
class ReverseBitReader {
private:
    const std::string& bitString;
    int pos; // Position from END (like ReverseBitReader::_pos)

public:
    ReverseBitReader(const std::string& s) : bitString(s), pos(s.length()) {}

    bool readBool() {
        if (pos <= 0) throw std::runtime_error("Read past end");
        pos--;
        return bitString[pos] == '1';
    }

    int readNumber(int bits) {
        if (pos - bits < 0) throw std::runtime_error("Read past end");
        pos -= bits;
        std::string chunk = bitString.substr(pos, bits);
        // Convert MSB-first binary to int
        int result = 0;
        for (int i = 0; i < bits; i++) {
            if (chunk[i] == '1') {
                result |= (1 << (bits - 1 - i));
            }
        }
        return result;
    }

    void skip(int bits) {
        if (pos - bits < 0) throw std::runtime_error("Skip past end");
        pos -= bits;
    }

    int absolutePos() const { return pos; }
    int relativePos() const { return bitString.length() - pos; }
};

void parseD2IFile(const std::string& filename) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "PARSING D2I FILE (using ItemParser logic)" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Read file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "❌ Cannot open: " << filename << std::endl;
        return;
    }
    
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "📁 File: " << filename << std::endl;
    std::cout << "   Size: " << bytes.size() << " bytes" << std::endl;
    std::cout << "   Header: " << (char)bytes[0] << (char)bytes[1] << std::endl;
    std::cout << "   Bytes: ";
    for (size_t i = 0; i < bytes.size(); i++) {
        std::cout << std::hex << (int)bytes[i] << " ";
    }
    std::cout << std::dec << std::endl;
    
    // Create bitString
    std::string bitString = createBitStringFromBytes(bytes);
    std::cout << "\n📊 BitString: " << bitString.length() << " bits" << std::endl;
    if (bitString.length() > 100) {
        std::cout << "   Last 100: ..." << bitString.substr(bitString.length() - 100) << std::endl;
    } else {
        std::cout << "   Full: " << bitString << std::endl;
    }
    
    // Parse using ReverseBitReader
    try {
        ReverseBitReader reader(bitString);
        
        std::cout << "\n🔍 Parsing Header:" << std::endl;
        
        bool isQuest = reader.readBool();
        std::cout << "   isQuest: " << isQuest << " (pos=" << reader.relativePos() << ")" << std::endl;
        
        reader.skip(3);
        bool isIdentified = reader.readBool();
        std::cout << "   isIdentified: " << isIdentified << " (pos=" << reader.relativePos() << ")" << std::endl;
        
        reader.skip(5);
        reader.skip(1); // is duped
        bool isSocketed = reader.readBool();
        std::cout << "   isSocketed: " << isSocketed << " (pos=" << reader.relativePos() << ")" << std::endl;
        
        reader.skip(2);
        reader.skip(2); // illegal + unk
        bool isEar = reader.readBool();
        std::cout << "   isEar: " << isEar << " (pos=" << reader.relativePos() << ")" << std::endl;
        
        if (isEar) {
            std::cout << "\n⚠️  Item is an Ear - stopping parse" << std::endl;
            return;
        }
        
        bool isStarter = reader.readBool();
        reader.skip(2);
        reader.skip(1);
        bool isSimple = reader.readBool();
        bool isExtended = !isSimple;
        std::cout << "   isExtended: " << isExtended << " (pos=" << reader.relativePos() << ")" << std::endl;
        
        bool isEthereal = reader.readBool();
        reader.skip(1);
        bool isPersonalized = reader.readBool();
        reader.skip(1);
        bool isRW = reader.readBool();
        std::cout << "   isRW: " << isRW << " (pos=" << reader.relativePos() << ")" << std::endl;
        
        reader.skip(5);
        reader.skip(8); // version
        reader.skip(2);
        
        int location = reader.readNumber(3);
        int whereEquipped = reader.readNumber(4);
        int column = reader.readNumber(4);
        int row = reader.readNumber(4);
        int storage = reader.readNumber(3);
        
        std::cout << "   location: " << location << std::endl;
        std::cout << "   whereEquipped: " << whereEquipped << std::endl;
        std::cout << "   column: " << column << ", row: " << row << std::endl;
        std::cout << "   storage: " << storage << std::endl;
        std::cout << "   Position after basic header: " << reader.relativePos() << std::endl;
        
        // Read item type (4 chars, 8 bits each)
        std::string itemType = "";
        for (int i = 0; i < 4; i++) {
            int charVal = reader.readNumber(8);
            if (charVal != 0) {
                itemType += (char)charVal;
            }
        }
        std::cout << "\n   📦 Item Type: '" << itemType << "'" << std::endl;
        std::cout << "   Position after item type: " << reader.relativePos() << std::endl;
        
        if (!isExtended) {
            std::cout << "\n   ✅ Simple item (not extended)" << std::endl;
            std::cout << "   🔧 Properties should start at position: " << reader.relativePos() << std::endl;
            
            // Try to read first property ID
            std::cout << "\n🔍 Reading Properties:" << std::endl;
            int propId = reader.readNumber(9);
            std::cout << "   First property ID: " << propId;
            if (propId == 511) {
                std::cout << " (END MARKER)" << std::endl;
            } else {
                std::cout << " (ACTUAL PROPERTY - unexpected for empty item!)" << std::endl;
            }
            
            std::cout << "   Position after first prop ID: " << reader.relativePos() << std::endl;
            std::cout << "   Remaining bits: " << (bitString.length() - reader.relativePos()) << std::endl;
            
        } else {
            std::cout << "\n   Extended item parsing not fully implemented in this debug tool" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "\n❌ Exception during parsing: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file1.d2i> [file2.d2i ...]" << std::endl;
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        parseD2IFile(argv[i]);
    }
    
    return 0;
}
