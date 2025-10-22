# D2I Format Complete Guide - MedianXL Offline Tools

## Table of Contents
1. [Overview](#overview)
2. [File Structure](#file-structure)
3. [Bit Encoding Rules](#bit-encoding-rules)
4. [Extended Items](#extended-items)
5. [Quality Types](#quality-types)
6. [Properties](#properties)
7. [Common Issues & Solutions](#common-issues--solutions)
8. [Tools & Scripts](#tools--scripts)
9. [Working Examples](#working-examples)

---

## Overview

### What is .d2i Format?
- Binary format for Diablo 2 items used by MedianXL Offline Tools
- Stores item data as bit sequences with specific field positions
- Two main types: **Non-Extended** (basic items) and **Extended** (items with properties)

### Key Concepts
- **LSB-first encoding**: Within each byte, bits are read from least significant to most significant
- **NO prepend logic**: Unlike ItemParser reading (which uses prepend), actual file format stores bytes in sequential order
- **Bit precision**: Each field has exact bit positions - off by one bit corrupts entire structure

---

## File Structure

### Non-Extended Item (Example: rich.d2i - 12 bytes)
```
[Header: 60 bits] [Item Type: 32 bits] [Other fields...]
```

**Hex**: `1000800065000e5207b402`

**Bit breakdown**:
- Bits 0-59: Header (includes JM signature, unknown fields, location)
- Bits 60-91: Item type code ('u@+ ' for charm)
- Bits 92+: Additional fields (quality, etc.)

### Extended Item Structure
```
[Header: 60 bits]
[Item Type: 32 bits]
[Extended Fields: 48 bits]
[Quality-Specific Fields: variable]
[Properties: variable]
[End Marker: 9 bits (511)]
[Padding to byte boundary]
```

### Extended Fields Detail (48 bits at position 92-139)
| Field | Bits | Position | Description |
|-------|------|----------|-------------|
| socketablesNumber | 3 | 92-94 | Number of sockets (0-6) |
| guid | 32 | 95-126 | **Critical**: Unique item name identifier (0 = no unique name) |
| ilvl | 7 | 127-133 | Item level (0-99) |
| quality | 4 | 134-137 | Quality type (see Quality Types section) |
| hasMultiGraphics | 1 | 138 | Multiple graphics flag |
| autoPrefix | 1 | 139 | Auto magic prefix flag |

**Critical Discovery**: Setting `guid=0` prevents the game from assigning a unique item name, allowing items to use their base type name instead.

---

## Bit Encoding Rules

### LSB-First Within Bytes
Bits are stored LSB-first **within each byte**, but bytes are in sequential order.

**Example**: Number 5 (binary: 101) stored in 3 bits
- LSB-first: `1 0 1` (read right-to-left for value)
- In file: stored as `1 0 1` in the bit sequence

### Byte Order (NO PREPEND)
```cpp
// WRONG (ItemParser prepend logic - only for reading):
itemBytes.prepend(byte);

// CORRECT (for writing .d2i files):
itemBytes.push_back(byte);  // Sequential order
```

### Converting Numbers to Bits LSB-First
```cpp
string numberToBitsLSB(int value, int bits) {
    string result = "";
    for (int i = 0; i < bits; i++) {
        result += (value & (1 << i)) ? '1' : '0';
    }
    return result;
}
```

**Example**: `numberToBitsLSB(7, 4)` produces `"1110"` (not `"0111"`)

### Bit-to-Byte Conversion
```cpp
vector<unsigned char> bitsToBytes(const string& bits) {
    vector<unsigned char> bytes;
    for (size_t i = 0; i < bits.size(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8 && i + j < bits.size(); j++) {
            if (bits[i + j] == '1') {
                byte |= (1 << j);  // LSB-first within byte
            }
        }
        bytes.push_back(byte);  // NO prepend!
    }
    return bytes;
}
```

---

## Extended Items

### How to Create Extended Items

#### Method 1: Set Extended Bit (Bit 21)
Original rich.d2i bit 21 = 0 (non-extended)
```
Bit 21: 0 → 1
```

#### Method 2: Insert Extended Fields After Item Type
```
[Header 60][Type 32] → [Header 60][Type 32][Extended 48]
```

### Extended Field Example
```cpp
// Create extended fields for ilvl=99, quality=7 (unique), guid=0
string socketablesNumber = "000";           // 0 sockets
string guid = "00000000000000000000000000000000";  // 32 zeros (no unique name!)
string ilvl = numberToBitsLSB(99, 7);      // "1100011"
string quality = numberToBitsLSB(7, 4);    // "1110"
string hasMultiGraphics = "0";
string autoPrefix = "0";

string extendedFields = socketablesNumber + guid + ilvl + quality + 
                       hasMultiGraphics + autoPrefix;
```

---

## Quality Types

### Quality Values (4 bits)
| Value | Type | Quality-Specific Fields Required |
|-------|------|----------------------------------|
| 1 | Low Quality | None |
| 2 | Normal | None |
| 3 | Superior | None |
| 4 | Magic | magicPrefix (11 bits) + magicSuffix (11 bits) = 22 bits |
| 5 | Set | setId (12 bits) |
| 6 | Rare | rarePrefix (8 bits) + rareSuffix (8 bits) = 16 bits |
| 7 | Unique | uniqueId (12 bits) |
| 8 | Crafted | Similar to rare |

### Magic Items (Quality 4)
**Bit positions after extended fields** (assuming extended at 92-139):
```
Bit 140-150: magicPrefix (11 bits)
Bit 151-161: magicSuffix (11 bits)
Bit 162+: Properties start here
```

**Example**:
```cpp
string magicPrefix = "00000000000";   // 11 bits (0 = no prefix)
string magicSuffix = "00000000000";   // 11 bits (0 = no suffix)
string qualityFields = magicPrefix + magicSuffix;
```

### Unique Items (Quality 7)
**Bit positions after extended fields**:
```
Bit 140-151: uniqueId (12 bits)
Bit 152+: Properties start here
```

**Example**:
```cpp
string uniqueId = "000000000000";  // 12 bits (0 = no unique name)
string qualityFields = uniqueId;
```

**Critical**: `uniqueId=0` combined with `guid=0` keeps the base item type name (e.g., "u@+ " charm stays as charm, not "Twisted Wire" or other unique name).

---

## Properties

### Property Structure
```
[Property ID: 9 bits LSB][Property Value: N bits LSB]...[End Marker: 9 bits = 511]
```

### Property Definition (props.tsv)
```
ID	name	add	bits	divide	paramBits
79	item_goldbonus	100	9	0	0
```

- **ID**: Property identifier (9 bits in item file)
- **bits**: Number of bits for property value
- **paramBits**: Additional parameter bits (0 for most properties)

### Example: Gold Find +100%
```cpp
string propId = numberToBitsLSB(79, 9);      // Gold Find property
string propValue = numberToBitsLSB(100, 9);  // +100% value (9 bits from props.tsv)
string property = propId + propValue;        // Total: 18 bits
```

### Multiple Properties
```cpp
string properties = "";
properties += numberToBitsLSB(79, 9) + numberToBitsLSB(100, 9);  // Gold Find
properties += numberToBitsLSB(36, 9) + numberToBitsLSB(50, 7);   // Magic Find
properties += "111111111";  // End marker (511)
```

### End Marker (REQUIRED)
```cpp
string endMarker = "111111111";  // 9 bits, value 511
```

**Critical**: Properties section MUST end with marker 511. Without it, parser continues reading random bits.

### Minimum Property Requirement
Even "clean" extended items need at least one property to prevent corruption:
```cpp
string defaultProperty = numberToBitsLSB(0, 9) +      // Property ID 0
                        numberToBitsLSB(10, 7);       // Value 10 (7 bits)
string endMarker = "111111111";
string properties = defaultProperty + endMarker;
```

---

## Common Issues & Solutions

### Issue 1: Wrong Property ID Displayed
**Symptom**: Property shows as ID 39 instead of 79
**Cause**: Missing quality-specific fields (magic prefix/suffix or unique ID)
**Solution**: Add quality-specific fields BEFORE properties
```cpp
// For magic items:
string qualityFields = magicPrefix + magicSuffix;  // 22 bits
// Properties start at bit 162

// For unique items:
string qualityFields = uniqueId;  // 12 bits
// Properties start at bit 152
```

### Issue 2: Item Type Corruption
**Symptom**: Item shows as wrong type (e.g., "rin" instead of "u@+ ")
**Cause**: Using prepend logic when writing bytes
**Solution**: Write bytes in sequential order (NO prepend)
```cpp
// WRONG:
for (auto byte : bytes) {
    itemBytes.prepend(byte);
}

// CORRECT:
for (auto byte : bytes) {
    itemBytes.push_back(byte);
}
```

### Issue 3: Item Name Changes
**Symptom**: Charm becomes "Twisted Wire" or other unique name
**Cause**: Copying guid or uniqueId from another item
**Solution**: Set guid=0 and uniqueId=0 in extended fields
```cpp
string guid = "00000000000000000000000000000000";  // 32 zeros
string uniqueId = "000000000000";  // 12 zeros (for unique quality)
```

### Issue 4: File Corruption Without Properties
**Symptom**: Item corrupts when imported with no properties
**Cause**: Parser expects at least one property between quality fields and end marker
**Solution**: Add default property with ID=0, value=10
```cpp
string defaultProperty = numberToBitsLSB(0, 9) + numberToBitsLSB(10, 7);
string properties = defaultProperty + endMarker;
```

### Issue 5: Bit Position Miscalculation
**Symptom**: Properties read from wrong positions
**Cause**: Not accounting for quality-specific field sizes
**Solution**: Calculate exact positions
```cpp
// Magic (quality 4):
// Header(60) + Type(32) + Extended(48) + Prefix(11) + Suffix(11) = 162
int propertyStartBit = 162;

// Unique (quality 7):
// Header(60) + Type(32) + Extended(48) + UniqueId(12) = 152
int propertyStartBit = 152;
```

---

## Tools & Scripts

### C++ Tools

#### 1. create_clean_extended.cpp
**Purpose**: Create minimal extended charm templates with default property

**Usage**:
```bash
g++ -std=c++11 -o create_clean_extended create_clean_extended.cpp
./create_clean_extended d2i/rich.d2i d2i/output.d2i <quality> <ilvl>
```

**Examples**:
```bash
# Magic charm (quality 4)
./create_clean_extended d2i/rich.d2i d2i/rich_clean_magic.d2i 4 99

# Unique charm (quality 7)
./create_clean_extended d2i/rich.d2i d2i/rich_clean_unique.d2i 7 99
```

**Output**:
- Magic: 26 bytes (includes prefix/suffix fields)
- Unique: 25 bytes (includes uniqueId field)
- Both have: guid=0, default property (ID=0, value=10), end marker 511

#### 2. create_charm_with_props.cpp
**Purpose**: Create extended charm copying properties from another file

**Usage**:
```bash
g++ -std=c++11 -o create_charm_with_props create_charm_with_props.cpp
./create_charm_with_props d2i/rich.d2i d2i/twi_1240052515.d2i d2i/output.d2i 7 99
```

**Key Feature**: Creates NEW extended fields (guid=0) while copying properties from source file

#### 3. copy_extended_props.cpp
**Purpose**: Copy extended fields + properties from one item to another

**Note**: This tool copies guid/uniqueId, which may change item name. Use create_charm_with_props.cpp instead for better control.

### Python Verification Scripts

#### 1. verify_with_prop.py
**Purpose**: Parse and validate item structure with properties

**Usage**:
```bash
python3 verify_with_prop.py
```

**Output**: Shows extended fields, quality-specific fields, properties, and end marker position

#### 2. diagnose_extended.py
**Purpose**: Bit-by-bit comparison of files

**Usage**: Edit script to specify files to compare
```python
analyze_file('d2i/rich.d2i', 'rich.d2i (non-extended)')
analyze_file('d2i/rich_clean_magic.d2i', 'rich_clean_magic.d2i')
```

#### 3. analyze_extended_fields.py
**Purpose**: Document meaning of each extended field bit

**Features**: Shows decimal values, identifies guid position, explains each field

---

## Working Examples

### Example 1: Rich Clean Magic Charm
**File**: `d2i/rich_clean_magic.d2i`  
**Size**: 26 bytes  
**Quality**: 4 (Magic)  
**Hex**: `1000800065000e5207b4020200000080310100000050fc07`

**Structure**:
```
Bits 0-59:    Header
Bits 60-91:   Item type 'u@+ '
Bits 92-139:  Extended fields (guid=0, ilvl=99, quality=4)
Bits 140-161: Magic prefix/suffix (both 0)
Bits 162-177: Default property (ID=0, value=10)
Bits 178-186: End marker (511)
Bits 187:     Padding
```

**Ready for**: Import as magic charm, add more properties later

### Example 2: Rich Clean Unique Charm
**File**: `d2i/rich_clean_unique.d2i`  
**Size**: 25 bytes  
**Quality**: 7 (Unique)  
**Hex**: `1000800065000e5207b4020200000080f101000014ff01`

**Structure**:
```
Bits 0-59:    Header
Bits 60-91:   Item type 'u@+ '
Bits 92-139:  Extended fields (guid=0, ilvl=99, quality=7)
Bits 140-151: Unique ID (0 = no unique name)
Bits 152-167: Default property (ID=0, value=10)
Bits 168-176: End marker (511)
Bits 177:     Padding
```

**Ready for**: Import as unique charm without unique name, add more properties later

### Example 3: Charm with Gold Find Property
**Goal**: Add Gold Find +100% to clean magic charm

**Process**:
1. Read rich_clean_magic.d2i and convert to bits
2. Find end marker position (bit 178)
3. Insert property BEFORE end marker:
   ```cpp
   string goldFind = numberToBitsLSB(79, 9) +      // Property ID
                    numberToBitsLSB(100, 9);       // Value (9 bits from props.tsv)
   ```
4. Keep end marker at the end
5. Convert bits back to bytes

**Result**: Charm with default property + Gold Find + end marker

### Example 4: Copying Properties from TWI
**Source**: `d2i/twi_1240052515.d2i` (working unique charm with properties)  
**Target**: Create new charm with TWI's properties but no unique name

**Code**:
```cpp
// Read TWI file and convert to bits
string twiBits = bytesToBits(twiBytes);

// TWI properties start at bit 152 (unique quality)
// Find end marker (511) in TWI
size_t endPos = twiBits.find("111111111", 152);

// Extract properties (from bit 152 to end marker + 9)
string twiProperties = twiBits.substr(152, endPos - 152 + 9);

// Create new item with guid=0, uniqueId=0, and TWI properties
string newBits = richHeaderAndType + extendedFields + qualityFields + twiProperties;
```

---

## Best Practices

### 1. Always Verify Bit Positions
```cpp
// Calculate positions based on structure
int headerEnd = 60;
int typeEnd = headerEnd + 32;  // 92
int extendedEnd = typeEnd + 48;  // 140

// For magic quality:
int qualityFieldEnd = extendedEnd + 22;  // 162
int propertyStart = qualityFieldEnd;

// For unique quality:
int qualityFieldEnd = extendedEnd + 12;  // 152
int propertyStart = qualityFieldEnd;
```

### 2. Set guid=0 for Generic Items
```cpp
// Prevent unique name assignment
string guid = string(32, '0');  // 32 zeros
```

### 3. Include End Marker
```cpp
// ALWAYS end properties with marker 511
properties += "111111111";
```

### 4. Test Incrementally
1. Create basic extended item (no properties)
2. Add default property (ID=0, value=10)
3. Test import in MedianXLOfflineTools
4. Add desired properties one at a time
5. Verify each step

### 5. Use Python for Verification
```python
# Quick check without compiling
def verify_structure(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    bits = bytes_to_bits(data)
    
    print(f"Total bits: {len(bits)}")
    print(f"Extended bit (21): {bits[21]}")
    print(f"Quality: {bits_to_number(bits[134:138])}")
    print(f"Property start: {162 if quality==4 else 152}")
```

### 6. Keep Backups
```bash
# Before modifying files
cp d2i/rich.d2i d2i/rich.d2i.backup
```

---

## Appendix: ItemParser Logic (MedianXLOfflineTools)

### Reading Items (Line 80)
```cpp
// ItemParser prepends bytes when READING
itemBitData.prepend(charToHex(itemBytes[j]));
```

### Writing Items (Line 510)
```cpp
// ItemParser also prepends when WRITING
itemBytes.prepend(value);
```

**Important**: The prepend logic is for ItemParser's internal representation. When creating .d2i files directly, use sequential byte order (NO prepend).

### Property Parsing (Line 240+)
```cpp
// Parser expects properties after quality-specific fields
// Magic: after prefix/suffix (bit 162)
// Unique: after uniqueId (bit 152)
int propId = itemBitData.mid(position, 9).toInt(nullptr, 2);
if (propId == 511) break;  // End marker

// Get property definition from props.tsv
int valueBits = propDef.bits;
int paramBits = propDef.paramBits;
int propValue = itemBitData.mid(position + 9, valueBits).toInt(nullptr, 2);
```

---

## Credits & References

- **MedianXL Offline Tools**: Original codebase and ItemParser logic
- **D2 File Format Documentation**: `d2 docs/` directory
- **props.tsv**: Property definitions from MedianXL mod
- **Forum Threads**: Research notes in `forum threads text/`

## Changelog

### 2025-10-22
- Initial comprehensive documentation
- Documented extended item structure
- Explained quality-specific fields
- Detailed bit encoding rules
- Added working examples and tools
- Documented common issues and solutions

---

**Last Updated**: October 22, 2025  
**Version**: 1.0  
**Status**: Complete guide based on successful implementation
