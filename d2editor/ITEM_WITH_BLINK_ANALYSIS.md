# Phân Tích item_with_blink.d2i - Findings

## 📋 Tổng Kết

File `d2i/item_with_blink.d2i` (77 bytes) chứa properties có **parameters** - một tính năng đặc biệt trong D2 item format.

## 🔍 Khám Phá Chính

### 1. Properties Có Parameters

Phát hiện 3 properties chính có parameters trong database:

| Property ID | Name | Value Bits | **Param Bits** | Mục đích |
|-------------|------|------------|----------------|----------|
| **97** | item_nonclassskill | 7 | **12** | Oskill (cross-class skill) |
| **107** | item_singleskill | 5 | **11** | +Skills to specific skill |
| **151** | item_aura | 7 | **11** | Aura when equipped |

### 2. Parameter Storage

Parameters được lưu trong field `h_saveParamBits` của database (không phải `paramBits`):

```sql
sqlite3 data/props.db "SELECT code, name, h_saveParamBits FROM props WHERE h_saveParamBits IS NOT NULL"
```

Output:
```
97|item_nonclassskill|12
107|item_singleskill|11
151|item_aura|11
```

### 3. Property Structure với Parameters

**Standard property** (ví dụ: Gold Find):
```
[Property ID: 9 bits][Value: 9 bits]
Total: 18 bits
```

**Property với parameter** (ví dụ: +Skill):
```
[Property ID: 9 bits][Parameter: 11 bits][Value: 5 bits]
                      ↑ Skill ID          ↑ Skill level
Total: 25 bits
```

## 📊 Ví Dụ Cụ Thể

### Property 107: +3 to Blink (Skill ID 54)

```
Bit structure:
┌─────────────┬────────────────┬──────────────┐
│ Property ID │   Parameter    │    Value     │
│   (9 bits)  │   (11 bits)    │  (5 bits)    │
│    107      │   Skill ID 54  │  Level 3     │
└─────────────┴────────────────┴──────────────┘

Encoded (MSB-first):
- Property ID: 001101011 (107)
- Parameter:   00000110110 (54 = Blink skill ID)
- Value:       00100 (4 = 3+1, add=1)

Total: 25 bits
```

### Property 97: Level 10 Teleport Oskill

```
Bit structure:
┌─────────────┬────────────────┬──────────────┐
│ Property ID │   Parameter    │    Value     │
│   (9 bits)  │   (12 bits)    │  (7 bits)    │
│     97      │   Skill ID 54  │  Level 10    │
└─────────────┴────────────────┴──────────────┘

Encoded (MSB-first):
- Property ID: 001100001 (97)
- Parameter:   000000110110 (54 = Teleport)
- Value:       0001011 (11 = 10+1, add=1)

Total: 28 bits
```

## 🛠️ Tools Created

### 1. analyze_property_with_param.py
Script để phân tích properties có parameters:
```bash
python3 analyze_property_with_param.py d2i/item_with_blink.d2i
```

**Features**:
- Đọc `h_saveParamBits` từ database
- Parse parameter bits
- Hiển thị skill/aura ID
- Support cả LSB-first và MSB-first encoding

### 2. parse_itemparser_style.py
Parse theo cách ItemParser (prepend logic):
```bash
python3 parse_itemparser_style.py d2i/item_with_blink.d2i
```

**Features**:
- Sử dụng prepend logic như MedianXLOfflineTools
- MSB-first per byte
- Match với ItemParser behavior

### 3. raw_bit_analysis.py
Phân tích raw bits để debug:
```bash
python3 raw_bit_analysis.py
```

**Features**:
- Hiển thị bits theo nhóm 50
- Tìm tất cả end markers
- Check potential property positions

## 📚 Documentation Created

### PROPERTIES_WITH_PARAMETERS.md
Tài liệu đầy đủ về properties có parameters:
- Property structures
- Encoding examples
- Python implementation guide
- Testing procedures
- Future enhancements needed

## ⚠️ Limitations Found

### Current property_adder.py

**Không hỗ trợ parameters** vì:
1. Chỉ đọc `paramBits`, không đọc `h_saveParamBits`
2. Không có interface để nhập parameter values
3. Encoding logic chưa xử lý parameter bits

### File Format Issues

File `item_with_blink.d2i`:
- 77 bytes (larger than typical)
- Có thể là stash file hoặc format đặc biệt
- Bit 21 (extended) = 0 (non-extended?)
- Needs MedianXLOfflineTools để parse chính xác

## 🎯 Next Steps

### 1. Update property_adder.py

**Phase 1**: Load h_saveParamBits
```python
# In load_properties_from_db()
cursor.execute("SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props")
# Store both paramBits and h_saveParamBits
```

**Phase 2**: Support parameter input
```python
# New function
def add_property_with_param(filename, prop_id, param, value):
    # Encode: [prop_id 9][param N][value M]
    pass
```

**Phase 3**: CLI interface
```bash
# Syntax: prop_id:parameter:value
python3 property_adder.py file.d2i 107:54:3
# Adds: +3 to Skill 54
```

### 2. Create Skill Database

```sql
-- skills.db
CREATE TABLE skills (
    skillId INTEGER PRIMARY KEY,
    name TEXT,
    class TEXT,
    description TEXT
);

-- Example data
INSERT INTO skills VALUES (54, 'Blink', 'Sorceress', 'Teleport skill');
```

### 3. GUI Enhancement

Add parameter input fields:
```
Property: [Dropdown: item_singleskill]
Skill ID: [Input: 54] [Lookup: Blink]
Value:    [Input: 3]
[Add Property]
```

### 4. Validation

Validate skill IDs against:
- Valid skill range (0-1024?)
- Class restrictions
- Oskill compatibility

## 💡 Key Learnings

### 1. Two Types of ParamBits

- `paramBits`: Standard parameters (rarely used)
- `h_saveParamBits`: Runeword/special parameters (commonly used)

### 2. Bit Order Matters

- **ItemParser**: MSB-first per byte + prepend logic
- **property_adder.py**: LSB-first + sequential order
- Need different encoding for each!

### 3. Parameters = Context

Parameters give context to property values:
- Property 107 value "3" means nothing alone
- Property 107 param "54" value "3" = "+3 to Blink"

### 4. Add Offset Still Applies

Even with parameters, add offset is used:
```
Input: +3 to skill
Param: 54 (Blink)
Value stored: 3 + 1 = 4
Game displays: 4 - 1 = 3
```

## 📊 Statistics

```
Properties in database: 398
Properties with bits > 0: 387
Properties with h_saveParamBits: 3
  - item_nonclassskill (97): 12 param bits
  - item_singleskill (107): 11 param bits
  - item_aura (151): 11 param bits

Tools created: 3
  - analyze_property_with_param.py
  - parse_itemparser_style.py
  - raw_bit_analysis.py

Documentation: 1
  - PROPERTIES_WITH_PARAMETERS.md
```

## 🎓 Conclusion

File `item_with_blink.d2i` đã giúp chúng ta khám phá:

✅ **Properties có parameters tồn tại** trong D2 format  
✅ **3 properties chính** có tính năng này  
✅ **h_saveParamBits field** chứa thông tin param  
✅ **Parameter = Skill/Aura ID**, Value = Level  
✅ **Current tools cần update** để support tính năng này  

**Kết quả**: Có tài liệu đầy đủ và roadmap để implement property parameters vào property_adder.py!

---

**Analysis Date**: October 22, 2025  
**File**: d2i/item_with_blink.d2i (77 bytes)  
**Properties Found**: 3 types with parameters  
**Status**: ✅ Documented, ⏳ Implementation Pending
