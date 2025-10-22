# Property Adder Update - Database Integration

## 🎉 Major Update: Full Property Database Support

### What's New

**Before**: Chỉ hỗ trợ 9 properties hardcoded
```python
PROPERTIES = {
    0: "Strength",
    1: "Energy",
    ...
    127: "All Skills"
}
```

**Now**: Hỗ trợ **387 properties** từ database!
```python
# Auto-loaded from data/props.db
✅ Loaded 387 properties from database
```

## 📊 Database Schema

File: `data/props.db` (SQLite database)

### Table: props
```sql
code INTEGER PRIMARY KEY     -- Property ID (0-397)
name TEXT                    -- Property internal name
addv INTEGER                 -- Add value (offset)
bits INTEGER                 -- Number of bits for value encoding
paramBits INTEGER            -- Additional parameter bits
... (+ many other fields)
```

### Property Selection Criteria
Properties được load nếu:
- `bits > 0` (có thể encode được)
- Có tất cả 387/398 properties thỏa điều kiện

## 🔧 Using Property Adder

### Command Line

#### List available properties
```bash
# Show common properties
python3 property_adder.py

# Show all 387 properties
python3 property_adder.py --list-all

# Show first 50 properties
python3 property_adder.py --list-all | head -60
```

#### Add properties by ID
```bash
# Single property
python3 property_adder.py file.d2i <prop_id> <value>

# Multiple properties
python3 property_adder.py file.d2i <id1> <val1> <id2> <val2> ...

# Examples:
python3 property_adder.py d2i/rich.d2i 7 10 79 50 80 30
python3 property_adder.py d2i/rich.d2i 200 5 300 10
```

### GUI Application

GUI đã tự động load tất cả properties từ database:
```bash
python3 gui_property_adder.py
```

Features:
- **Search/Filter**: Tìm property theo tên hoặc ID
- **Browse all 387 properties**: Danh sách đầy đủ
- **Batch add**: Thêm nhiều properties cùng lúc
- **Preview**: Xem trước size change

## 📚 Common Property IDs

### Stats (ID 0-10)
```
  0 = strength           (+200 base)
  1 = energy             (+200 base)
  2 = dexterity          (+200 base)
  3 = vitality           (+200 base)
  7 = maxhp              (+500 base)
  9 = maxmana            (+500 base)
```

### Resistances (ID 39-45)
```
 39 = fireresist        (+100 base)
 41 = lightresist       (+100 base)
 43 = coldresist        (+100 base)
 45 = poisonresist      (+100 base)
```

### Item Properties (ID 79-127)
```
 79 = item_goldbonus            (Gold Find %)
 80 = item_magicbonus           (Magic Find %)
 89 = item_fastergethitrate     (Hit Recovery %)
 93 = item_fasterattackrate     (Attack Speed %)
 96 = item_fastercastrate       (Cast Rate %)
 99 = item_fastermovevelocity   (Run/Walk %)
127 = item_allskills            (+All Skills)
```

### Damage (ID 17-22)
```
 17 = mindamage         (Min Damage)
 18 = maxdamage         (Max Damage)
 19 = secondary_mindamage
 20 = secondary_maxdamage
 21 = damagepercent     (Enhanced Damage %)
```

### Attack (ID 27-32)
```
 27 = tohit             (Attack Rating)
 28 = tohitpercent      (Attack Rating %)
 32 = item_openwounds   (Open Wounds %)
```

### Magic Properties (ID 200+)
```
200 = item_megaimpact
300 = dexterity_bonus_wpn
... (total 387 properties available)
```

## 🔍 Finding Property IDs

### Method 1: Command Line Search
```bash
# List all and grep
python3 property_adder.py --list-all | grep -i "attack"
python3 property_adder.py --list-all | grep -i "resist"
python3 property_adder.py --list-all | grep -i "damage"
```

### Method 2: Direct Database Query
```bash
sqlite3 data/props.db "SELECT code, name, addv, bits FROM props WHERE name LIKE '%attack%'"
sqlite3 data/props.db "SELECT code, name FROM props WHERE code BETWEEN 100 AND 200"
```

### Method 3: GUI Search
1. Open GUI: `python3 gui_property_adder.py`
2. Type in Search box
3. Browse filtered results

### Method 4: Python Script
```python
import property_adder

# Search by name
for pid, prop in property_adder.PROPERTIES.items():
    if 'attack' in prop['name'].lower():
        print(f"ID {pid}: {prop['name']}")

# Get specific property
prop = property_adder.PROPERTIES.get(79)
print(f"Property 79: {prop}")
```

## 💡 Understanding Property Values

### Add Value (Base Offset)
Nhiều properties có `add` value để offset:

```python
# Property 7 (maxhp): add=500, bits=12
# Input value: 10
# Encoded value: 10 + 500 = 510
# In-game display: +10 Life (game subtracts the base back)

# Property 79 (gold find): add=100, bits=9
# Input value: 50
# Encoded value: 50 + 100 = 150
# In-game display: 50% Gold Find
```

### Bits (Encoding Size)
Number of bits used to store property value:
- More bits = larger max value
- Common sizes: 5, 7, 9, 11, 12 bits

```
5 bits  = max value 31    (e.g., item_allskills)
7 bits  = max value 127   
9 bits  = max value 511   (e.g., gold find, magic find)
11 bits = max value 2047  (e.g., strength, dexterity)
12 bits = max value 4095  (e.g., life, mana)
```

## ⚠️ Important Notes

### 1. Properties Without Bits
Some properties in database have `bits=NULL` or `bits=0`:
- These are **NOT loaded** by property_adder
- Total in DB: 398 properties
- Loaded: 387 properties (with bits > 0)

### 2. Value Limits
Respect the `bits` limit for each property:
```python
prop = PROPERTIES[79]  # gold find
max_value = (2 ** prop['bits']) - 1 - prop['add']
# For gold find: (2^9) - 1 - 100 = 411 max input value
```

### 3. Property Combinations
Some properties don't work together or override each other:
- Test in game after adding
- Check MedianXL documentation for valid combinations

### 4. File Compatibility
Property adder works with:
- ✅ Extended items (created with create_clean_extended)
- ✅ Items with existing properties
- ❌ Non-extended items (need to convert first)

## 🧪 Testing Examples

### Example 1: All Stats +10
```bash
python3 property_adder.py d2i/rich_clean_magic.d2i 0 10 1 10 2 10 3 10
```
Output: Charm with +10 Str, +10 Energy, +10 Dex, +10 Vit

### Example 2: MF + GF Charm
```bash
python3 property_adder.py d2i/rich_clean_unique.d2i 79 30 80 25
```
Output: Charm with 30% GF, 25% MF

### Example 3: Resist All
```bash
python3 property_adder.py d2i/rich.d2i 39 20 41 20 43 20 45 20
```
Output: Charm with +20% to all resistances

### Example 4: Using High Property IDs
```bash
python3 property_adder.py d2i/rich.d2i 200 5 300 10
```
Output: Charm with item_megaimpact and dexterity_bonus_wpn

## 📦 Integration with GUI

The GUI automatically uses the database:

1. **Startup**: Loads all 387 properties from props.db
2. **Search**: Filters by property name or code
3. **Display**: Shows property name from database
4. **Add**: Uses correct bits/add values from database

No configuration needed - just run:
```bash
python3 gui_property_adder.py
```

## 🔄 Database Updates

If you need to update the property database:

### Check current properties
```bash
sqlite3 data/props.db "SELECT COUNT(*) FROM props"
sqlite3 data/props.db "SELECT COUNT(*) FROM props WHERE bits > 0"
```

### Add new property
```sql
INSERT INTO props (code, name, addv, bits, paramBits) 
VALUES (999, 'custom_property', 0, 8, 0);
```

### Reload in Python
```python
import property_adder
property_adder.load_properties_from_db()  # Reload
print(len(property_adder.PROPERTIES))
```

## 🐛 Troubleshooting

### "props.db not found"
```
⚠️ Warning: props.db not found at /path/to/data/props.db
Using fallback hardcoded properties
```
**Solution**: Ensure data/props.db exists in d2editor directory

### "Property ID not found"
```
❌ Unknown property ID: 999
```
**Solution**: Use `--list-all` to see available IDs, or check database

### "Bits value is 0 or NULL"
Some properties can't be encoded (no bits field in database)
**Solution**: Choose a different property with valid bits

## 📊 Statistics

```
Total properties in database: 398
Properties with bits > 0:     387
Properties loaded:            387
Success rate:                 97.2%
```

## 🎯 Next Steps

1. **Explore properties**: `python3 property_adder.py --list-all`
2. **Find interesting ones**: Use grep or GUI search
3. **Test on clean files**: Start with rich_clean_magic.d2i
4. **Verify in MedianXL Tools**: Import and check properties
5. **Create your custom items**: Combine properties as needed

---

**Version**: 2.0 (Database Integration)  
**Date**: October 22, 2025  
**Properties Available**: 387
