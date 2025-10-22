#!/usr/bin/env python3
"""
Parse rich.d2i EXACTLY following ItemParser logic with ReverseBitReader
"""

class ReverseBitReader:
    def __init__(self, data):
        # Skip JM header
        item_data = data[2:]
        
        # Convert to bitString using PREPEND logic (last byte first)
        self.bitString = ""
        for byte in reversed(item_data):
            # MSB-first within byte
            for i in range(7, -1, -1):
                self.bitString += '1' if (byte & (1 << i)) else '0'
        
        # ReverseBitReader reads from END
        self._pos = len(self.bitString)
        self.length = len(self.bitString)
        
        print(f"BitString length: {self.length}")
        print(f"BitString: {self.bitString}\n")
    
    def pos(self):
        return self._pos
    
    def skip(self, bits=1):
        self._pos -= bits
        if self._pos < 0:
            raise Exception(f"Read past beginning! pos={self._pos}")
    
    def readBool(self):
        self._pos -= 1
        if self._pos < 0:
            raise Exception(f"Read past beginning! pos={self._pos}")
        return self.bitString[self._pos] == '1'
    
    def readNumber(self, bits):
        if self._pos - bits < 0:
            raise Exception(f"Cannot read {bits} bits at pos={self._pos}")
        result = 0
        for i in range(bits):
            self._pos -= 1
            if self.bitString[self._pos] == '1':
                result |= (1 << i)
        return result

def parse_rich_d2i():
    with open('d2i/rich.d2i', 'rb') as f:
        data = f.read()
    
    print(f"File: rich.d2i, size: {len(data)} bytes")
    print(f"Hex: {data.hex()}\n")
    print("="*60)
    
    reader = ReverseBitReader(data)
    
    def skip(bits, name):
        pos_before = reader.pos()
        reader.skip(bits)
        print(f"Position {pos_before:3d} → {reader.pos():3d}: Skip {bits:2d} bits - {name}")
    
    def read_bool(name):
        pos_before = reader.pos()
        value = reader.readBool()
        print(f"Position {pos_before:3d} → {reader.pos():3d}: Read bool = {value:5} - {name}")
        return value
    
    def read_num(bits, name):
        pos_before = reader.pos()
        value = reader.readNumber(bits)
        print(f"Position {pos_before:3d} → {reader.pos():3d}: Read {bits:2d} bits = {value:5d} - {name}")
        return value
    
    print("📊 Parsing item following ItemParser.cpp logic:\n")
    
    # Header flags
    skip(1, "isQuest")
    skip(3, "unknown1")
    skip(1, "isIdentified")
    skip(5, "unknown2")
    skip(1, "isDuped")
    isSocketed = read_bool("isSocketed")
    skip(2, "unknown3")
    skip(2, "illegal+unk")
    isEar = read_bool("isEar")
    isStarter = read_bool("isStarter")
    skip(2, "unknown4")
    skip(1, "unknown5")
    isExtended_bit = read_bool("isExtended bit (will be inverted)")
    isExtended = not isExtended_bit
    print(f"   → isExtended = !{isExtended_bit} = {isExtended}")
    skip(1, "isEthereal")
    skip(1, "unknown6")
    isPersonalized = read_bool("isPersonalized")
    skip(1, "unknown7")
    isRW = read_bool("isRW")
    skip(5, "unknown8")
    skip(8, "version")
    skip(2, "unknown9")
    skip(3, "location")
    skip(4, "whereEquipped")
    skip(4, "column")
    skip(4, "row")
    skip(3, "storage")
    
    print(f"\n✅ After header: position = {reader.pos()}\n")
    
    # Item type (4 chars)
    itemType = ""
    for i in range(4):
        c = read_num(8, f"itemType char {i}")
        if c != 0:
            itemType += chr(c)
    print(f"   → Item type: '{itemType}'\n")
    
    print(f"✅ After item type: position = {reader.pos()}\n")
    
    if isExtended:
        print("📦 Item IS extended, reading extended fields:\n")
        socketablesNumber = read_num(3, "socketablesNumber")
        guid = read_num(32, "guid")
        ilvl = read_num(7, "ilvl")
        quality = read_num(4, "quality")
        
        hasVariableGraphic = read_bool("has variable graphic")
        if hasVariableGraphic:
            variableGraphicIndex = read_num(3, "variable graphic index")
        
        hasAutoprefix = read_bool("has autoprefix")
        if hasAutoprefix:
            skip(11, "autoprefix")
        
        print(f"\n   Quality = {quality}")
        # Quality 4 = Normal, no extra fields
        
        if isRW:
            skip(16, "RW code")
        
        if isPersonalized:
            print("   Reading inscribed name...")
            for i in range(16):
                c = read_num(7, f"inscribed char {i}")
                if c == 0:
                    break
        
        skip(1, "tome of ID bit")
        
        print(f"\n✅ After extended fields: position = {reader.pos()}\n")
        
        # Check if armor/weapon for defense/durability
        print("📦 Checking item type for defense/durability...")
        print(f"   Item type: '{itemType}' → Miscellaneous, no defense/durability\n")
        
        # Quantity (check if stackable)
        print("   Checking if stackable...")
        print(f"   '{itemType}' → Need to check item database")
        print(f"   Assuming NOT stackable for now\n")
        
        if isSocketed:
            socketsNumber = read_num(4, "sockets number")
        
        # Set lists (only for Set items)
        if quality == 5:  # Set
            for i in range(5):
                read_bool(f"has set list {i}")
        
        print(f"\n{'='*60}")
        print(f"🎯 PROPERTIES START AT POSITION: {reader.pos()}")
        print(f"{'='*60}")
        print(f"\nThis is where properties would be read")
        print(f"Position {reader.pos()} from END of bitString")
        print(f"Or index {reader.length - reader.pos()} in bitString\n")
    else:
        print("📦 Item is NOT extended\n")
        print(f"Properties would start at position: {reader.pos()}")

if __name__ == "__main__":
    try:
        parse_rich_d2i()
    except Exception as e:
        print(f"\n❌ ERROR: {e}")
