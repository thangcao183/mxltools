# Properties với Parameters - Complete Guide

## 🎯 Overview

Một số properties trong D2I format có **parameters** - các giá trị bổ sung đi kèm với property value. Parameters thường được dùng để chỉ định **skill ID**, **aura ID**, hoặc các thông tin cụ thể khác.

## 📊 Properties có Parameters

### Database Fields

Properties có parameters được định nghĩa trong `data/props.db` với field **`h_saveParamBits`** (không phải `paramBits`):

```sql
SELECT code, name, addv, bits, h_saveParamBits 
FROM props 
WHERE h_saveParamBits IS NOT NULL AND h_saveParamBits != '';
```

### Common Properties với Parameters

| ID | Name | Value Bits | Param Bits | Description |
|----|------|------------|------------|-------------|
| 97 | item_nonclassskill | 7 | 12 | Oskill (non-class skill) |
| 107 | item_singleskill | 5 | 11 | +Skills to specific skill |
| 151 | item_aura | 7 | 11 | Aura when equipped |

## 🔧 Property Structure với Parameters

### Standard Property (no param)
```
[Property ID: 9 bits][Property Value: N bits]
```

Example: Gold Find +50%
```
Property ID 79: 9 bits
Value: 9 bits
Total: 18 bits
```

### Property với Parameter
```
[Property ID: 9 bits][Parameter: M bits][Property Value: N bits]
```

Example: +1 to Skill ID 54 (Blink)
```
Property ID 107: 9 bits
Parameter (Skill ID): 11 bits  ← PARAMETER HERE
Value (skill level): 5 bits
Total: 25 bits
```

## 📖 Chi Tiết Từng Property

### 1. Property 97 - item_nonclassskill (Oskill)

**Purpose**: Adds oskill (non-class skill) to item  
**Structure**:
- Property ID: 97 (9 bits)
- Parameter: Skill ID (12 bits)
- Value: Skill level (7 bits, add=1)

**Example**: Level 10 Blink Oskill
```python
property_id = 97
param = 54  # Blink skill ID
value = 10  # Level 10

# Encoding (MSB-first):
prop_id_bits = format(97, '09b')     # "001100001"
param_bits = format(54, '012b')      # "000000110110"
value_bits = format(10+1, '07b')     # "0001011" (add=1)

total_bits = prop_id_bits + param_bits + value_bits  # 28 bits total
```

### 2. Property 107 - item_singleskill (+Skills)

**Purpose**: +Skills to specific skill  
**Structure**:
- Property ID: 107 (9 bits)
- Parameter: Skill ID (11 bits)
- Value: Bonus levels (5 bits, add=1)

**Example**: +3 to Teleport (Skill ID 54)
```python
property_id = 107
param = 54  # Teleport skill ID
value = 3   # +3 levels

# Encoding (MSB-first):
prop_id_bits = format(107, '09b')    # "001101011"
param_bits = format(54, '011b')      # "00000110110"
value_bits = format(3+1, '05b')      # "00100" (add=1)

total_bits = prop_id_bits + param_bits + value_bits  # 25 bits total
```

### 3. Property 151 - item_aura (Aura When Equipped)

**Purpose**: Grants aura when item is equipped  
**Structure**:
- Property ID: 151 (9 bits)
- Parameter: Aura/Skill ID (11 bits)
- Value: Aura level (7 bits, add=1)

**Example**: Level 5 Conviction Aura
```python
property_id = 151
param = 123  # Conviction aura ID (example)
value = 5    # Level 5

# Encoding (MSB-first):
prop_id_bits = format(151, '09b')    # "010010111"
param_bits = format(123, '011b')     # "00001111011"
value_bits = format(5+1, '07b')      # "0000110" (add=1)

total_bits = prop_id_bits + param_bits + value_bits  # 27 bits total
```

## 🔍 Finding Skill IDs

### Method 1: Database Query
```bash
# If there's a skills database
sqlite3 data/skills.db "SELECT skillId, name FROM skills WHERE name LIKE '%blink%'"
```

### Method 2: MedianXL Documentation
Skill IDs are documented in MedianXL skill tables and documentation.

### Method 3: Reverse Engineering
Parse existing items with skills to extract skill IDs.

## 💻 Implementation

### Python - Adding Property với Parameter

```python
def add_property_with_param(property_id, param_value, skill_value):
    """
    Add a property with parameter to item
    
    Args:
        property_id: Property code (97, 107, or 151)
        param_value: Parameter (skill ID, aura ID)
        skill_value: Property value (skill level)
    """
    import property_adder
    
    # Load property info
    prop = property_adder.PROPERTIES.get(property_id)
    if not prop:
        raise ValueError(f"Unknown property: {property_id}")
    
    # Check if property has parameters
    if prop['paramBits'] == 0:
        raise ValueError(f"Property {property_id} does not have parameters")
    
    # Encode property ID (9 bits, LSB-first)
    prop_id_bits = property_adder.number_to_binary_lsb(property_id, 9)
    
    # Encode parameter (paramBits, LSB-first)
    param_bits = property_adder.number_to_binary_lsb(param_value, prop['paramBits'])
    
    # Encode value (bits, LSB-first)
    raw_value = skill_value + prop['add']
    value_bits = property_adder.number_to_binary_lsb(raw_value, prop['bits'])
    
    # Combine
    property_bits = prop_id_bits + param_bits + value_bits
    
    return property_bits
```

### Usage Example

```python
# Add Level 10 Blink Oskill
prop_bits = add_property_with_param(
    property_id=97,      # item_nonclassskill
    param_value=54,      # Blink skill ID
    skill_value=10       # Level 10
)

# Add to item
# (insert prop_bits before end marker in item bitstring)
```

## ⚠️ Important Notes

### 1. Bit Encoding Order

**For ItemParser (MedianXLOfflineTools)**:
- Uses **prepend logic** when reading
- Bytes are MSB-first within each byte
- Requires specific bit ordering

**For property_adder.py**:
- Uses **LSB-first** encoding
- Sequential byte order (no prepend)
- Matches create_clean_extended format

### 2. Add Value Offset

Properties có parameters vẫn có `add` offset:
```python
# Property 107: add=1
input_value = 3  # User wants +3
encoded_value = 3 + 1 = 4  # Stored as 4
displayed_value = 4 - 1 = 3  # Game shows +3
```

### 3. Parameter vs Value

- **Parameter**: Identifies WHICH skill/aura (skill ID)
- **Value**: Specifies HOW MUCH (level, bonus)

Example: "+3 to Teleport"
- Parameter = 54 (Teleport skill ID)
- Value = 3 (bonus levels)

## 🧪 Testing Properties với Parameters

### Test File Creation

```bash
# Create clean template
./create_clean_extended d2i/rich.d2i d2i/test_param.d2i 7 99

# Would need special tool to add property with param
# property_adder.py currently doesn't support parameters
```

### Verification

1. Create item with property+parameter
2. Import into MedianXLOfflineTools
3. Check item displays correct skill
4. Test in-game functionality

## 📝 TODO: Update property_adder.py

Current `property_adder.py` needs enhancement to support parameters:

### Required Changes

1. **Load h_saveParamBits from database**
```python
# In load_properties_from_db()
cursor.execute("SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props")
# Store h_saveParamBits in PROPERTIES dict
```

2. **Support parameter input**
```python
# New function signature
def add_property_with_param(filename, prop_id, param, value):
    # Encode: [prop_id][param][value]
    pass
```

3. **CLI interface update**
```bash
# New syntax for properties with params
python3 property_adder.py file.d2i 107:54:3
# Format: property_id:parameter:value
```

## 🎯 Common Use Cases

### 1. Oskills (Cross-Class Skills)

```python
# Add Teleport oskill to any item
property_id = 97  # item_nonclassskill
skill_id = 54     # Teleport
level = 1         # Level 1

# Result: "Level 1 Teleport (X charges)" or similar
```

### 2. Skill Bonuses

```python
# +3 to specific skill
property_id = 107  # item_singleskill
skill_id = 42      # Some skill
bonus = 3          # +3 levels

# Result: "+3 to [Skill Name]"
```

### 3. Auras

```python
# Level 10 Conviction Aura when equipped
property_id = 151  # item_aura
aura_id = 123      # Conviction (example)
level = 10         # Level 10

# Result: "Level 10 Conviction Aura When Equipped"
```

## 📚 References

- **props.db**: Full property database with h_saveParamBits
- **MedianXL Documentation**: Skill IDs and aura IDs
- **ItemParser.cpp**: Reference implementation for parsing
- **D2I_FORMAT_COMPLETE_GUIDE.md**: Base format documentation

## 🔄 Future Enhancements

1. ✅ Document properties with parameters
2. ⏳ Update property_adder.py to support parameters
3. ⏳ Create skill ID database/lookup table
4. ⏳ Add GUI support for parameter input
5. ⏳ Validate skill IDs against game data

---

**Version**: 1.0  
**Last Updated**: October 22, 2025  
**Status**: Documentation Complete, Implementation Pending
