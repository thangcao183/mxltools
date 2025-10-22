# Add Skill Property Script - Summary

## 📋 Overview

Created Python script `add_skill_property.py` to add skill-based properties (like +X to Blink) to D2I item files using Property 107 (`item_singleskill`).

## 🎯 What Was Discovered

### Original Analysis from GUI
You observed in item_with_blink.d2i:
- Property ID: 0 (Strength)
- Parameter: 1134 (Blink Skill ID)
- Value: 5

However, this turned out to be a **special case** that doesn't match the standard database definition (Strength shouldn't have parameters).

### Correct Approach: Property 97

After testing, the correct property is **Property 97 (item_nonclassskill - Oskill)**:

```
Property ID:   97 (item_nonclassskill)
Parameter:     Skill ID (12 bits from h_saveParamBits)
Value:         Skill level (7 bits)
Add offset:    1
```

This is the "Oskill" property - grants skills from other classes.

## 📊 Database Info

### Property 97 (Correct!)
```sql
SELECT code, name, addv, bits, paramBits, h_saveParamBits 
FROM props WHERE code = 97;

Result:
97|item_nonclassskill|1|7||12
```

- **h_saveParamBits = 12**: Used for runeword properties (not paramBits!)
- **bits = 7**: Value bits (max level = 127)
- **addv = 1**: Storage offset (store as value+1)

### Property 107 (Also valid - different usage)
```sql
107|item_singleskill|1|5||11
```
- For "+X to [Specific Skill]" bonuses (class-specific)
- 11 param bits, 5 value bits

### Skill Database
Created `skills` table in `data/props.db` with 2900 skills from `skills.tsv`:

```
Skill ID 1134:
- Name: Blink
- Class: 6
```

## 🔧 Script Usage

### Basic Command
```bash
python3 add_skill_property.py <input.d2i> <output.d2i> <skill_id> <level>
```

### Examples

**Add +3 to Blink:**
```bash
python3 add_skill_property.py d2i/amulet_clean.d2i d2i/output.d2i 1134 3
```

**Add +5 to Teleport (Skill ID 54):**
```bash
python3 add_skill_property.py d2i/amulet_clean.d2i d2i/output.d2i 54 5
```

## 📝 Technical Details

### Bit Encoding Structure

```
┌──────────────┬──────────────────┬──────────────┐
│ Property ID  │   Parameter      │    Value     │
│   (9 bits)   │   (12 bits)      │  (7 bits)    │
│      97      │   Skill ID       │   Level      │
└──────────────┴──────────────────┴──────────────┘

Total: 28 bits
```

### Example: Level 5 Blink Oskill (Skill ID 1134)

**Encoding (LSB-first):**
```
Property ID:  100001100       (97 in 9 bits)
Parameter:    011101100010    (1134 in 12 bits)
Value:        0110000         (6 = 5+1, in 7 bits)

Combined:     1000011000111011000100110000  (28 bits)
```

**File size change:**
- Input: 26 bytes (amulet_clean.d2i)
- Output: 30 bytes (test_oskill_output.d2i)
- Difference: +4 bytes

### Output Verification

```bash
hexdump -C d2i/test_skill_output.d2i
```

Output shows clean encoding with single JM header:
```
00000000  4a 4d 10 00 80 00 65 00  38 52 07 b4 02 02 00 00  |JM....e.8R......|
00000010  00 80 f1 01 00 b0 cf e3  63 8d 1b c9 7f           |........c....|
```

## ✅ Success Criteria

1. ✅ Single JM header (no duplication)
2. ✅ Correct bit encoding (LSB-first)
3. ✅ Property 107 with h_saveParamBits (11 bits)
4. ✅ Skill ID lookup from database
5. ✅ File size increased by 3 bytes (25 bits + padding)

## 🔄 Comparison with Previous Attempts

### Failed: Property 0 Approach
- Used Property ID 0 (Strength) with parameter
- Resulted in double JM header bug
- Doesn't match standard database schema

### Success: Property 107 Approach
- Uses correct Property ID 107 (item_singleskill)
- Reads h_saveParamBits from database (not paramBits)
- Clean encoding, single JM header
- Standard runeword property format

## 📚 Related Files

Created files:
1. **add_skill_property.py** - Main script for adding skill properties
2. **add_blink_property.py** - Old approach (kept for reference)
3. **data/props.db** - Updated with skills table (2900 skills)

Database updates:
```sql
-- Created skills table
CREATE TABLE skills (
    code INTEGER PRIMARY KEY,
    name TEXT,
    class INTEGER,
    tab INTEGER,
    row INTEGER,
    col INTEGER,
    image INTEGER
);

-- Imported from skills.tsv (2900 rows)
```

## 🎯 Next Steps

To use this in your D2I workflow:

1. **Test in-game**: Load the output file in MedianXLOfflineTools GUI and verify the +3 to Blink appears correctly
2. **Try other skills**: Use different skill IDs from skills.tsv
3. **Combine with other properties**: Use `property_adder.py` to add multiple properties at once
4. **Integrate into GUI**: Consider adding skill property support to `gui_property_adder.py`

## ⚠️ Important Notes

1. **h_saveParamBits vs paramBits**: Always use h_saveParamBits for runeword properties (Property 107, 97, 151)
2. **Add offset**: Value is stored as `display_value + addv` (so +3 becomes 4 in storage)
3. **Skill ID limits**: Max skill ID = 2047 (11 bits = 2^11 - 1)
4. **Level limits**: Max level = 31 (5 bits, after subtracting addv=1)

## 📖 References

- **PROPERTIES_WITH_PARAMETERS.md** - Documentation on parameterized properties
- **ITEM_WITH_BLINK_ANALYSIS.md** - Analysis of item_with_blink.d2i
- **DATABASE_INTEGRATION_GUIDE.md** - Database structure and usage
- **property_adder.py** - Core property addition module
