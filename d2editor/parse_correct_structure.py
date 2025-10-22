#!/usr/bin/env python3
"""
Parser chính xác theo ItemParser.cpp
"""

def bytes_to_bits(data):
    """Convert bytes to bit string (LSB-first within byte)"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    return bits

def read_bits(bits, pos, count):
    """Read bits LSB-first"""
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits, pos + count

def parse_item_correct(filename):
    """Parse D2I item theo đúng ItemParser.cpp"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"\n{'='*70}")
    print(f"File: {filename}")
    print(f"Size: {len(data)} bytes")
    print(f"Hex: {data.hex()}")
    print(f"{'='*70}")
    
    # Skip JM header
    item_data = data[2:]
    bits = bytes_to_bits(item_data)
    
    print(f"Total bits: {len(bits)}\n")
    
    pos = 0
    
    # Parse theo ItemParser.cpp line by line
    print("--- HEADER (60 bits) ---")
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isQuest = {b} → {val}")
    
    pos += 3  # skip 3
    
    val, b, pos = read_bits(bits, pos, 1)
    is_identified = (val == 1)
    print(f"Bit {pos-1:3d}: isIdentified = {b} → {is_identified}")
    
    pos += 5  # skip 5
    pos += 1  # skip (is duped)
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isSocketed = {b} → {val}")
    
    pos += 2  # skip 2
    pos += 2  # skip 2 (illegal equip + unk)
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isEar = {b} → {val}")
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isStarter = {b} → {val}")
    
    pos += 2  # skip 2
    pos += 1  # skip 1
    
    val, b, pos = read_bits(bits, pos, 1)
    is_extended = (val == 0)  # INVERTED!
    print(f"Bit {pos-1:3d}: isExtended bit = {b} → isExtended = {is_extended} (INVERTED)")
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isEthereal = {b} → {val}")
    
    pos += 1  # skip 1
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isPersonalized = {b} → {val}")
    
    pos += 1  # skip 1
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isRW = {b} → {val}")
    
    pos += 5  # skip 5
    pos += 8  # skip 8 (version)
    pos += 2  # skip 2
    
    val, b, pos = read_bits(bits, pos, 3)
    print(f"Bit {pos-3:3d}: location (3 bits) = {val}")
    
    val, b, pos = read_bits(bits, pos, 4)
    print(f"Bit {pos-4:3d}: whereEquipped (4 bits) = {val}")
    
    val, b, pos = read_bits(bits, pos, 4)
    print(f"Bit {pos-4:3d}: column (4 bits) = {val}")
    
    val, b, pos = read_bits(bits, pos, 4)
    print(f"Bit {pos-4:3d}: row (4 bits) = {val}")
    
    val, b, pos = read_bits(bits, pos, 3)
    print(f"Bit {pos-3:3d}: storage (3 bits) = {val}")
    
    print(f"\nAfter header: pos = {pos} (should be 60)")
    
    # Item type (32 bits = 4 chars, 8 bits each)
    print(f"\n--- ITEM TYPE (32 bits) ---")
    item_type = ""
    for i in range(4):
        val, b, pos = read_bits(bits, pos, 8)
        if val != 32:  # Not space
            item_type += chr(val)
        print(f"Bit {pos-8:3d}: char {i} = {val} ('{chr(val) if 32 <= val < 127 else '?'}')")
    
    print(f"         Item type = '{item_type}'")
    print(f"\nAfter item type: pos = {pos} (should be 92)")
    
    # Extended fields (only if isExtended)
    if is_extended:
        print(f"\n--- EXTENDED FIELDS ---")
        
        # socketablesNumber (3 bits)
        val, b, pos = read_bits(bits, pos, 3)
        print(f"Bit {pos-3:3d}: socketablesNumber (3 bits) = {val}")
        
        # guid (32 bits)
        val, b, pos = read_bits(bits, pos, 32)
        print(f"Bit {pos-32:3d}: guid (32 bits) = {val}")
        
        # ilvl (7 bits)
        val, b, pos = read_bits(bits, pos, 7)
        print(f"Bit {pos-7:3d}: ilvl (7 bits) = {val}")
        
        # quality (4 bits)
        val, b, pos = read_bits(bits, pos, 4)
        quality_names = ["", "low", "normal", "superior", "magic", "set", "rare", "unique", "crafted"]
        quality_name = quality_names[val] if val < len(quality_names) else f"unknown({val})"
        print(f"Bit {pos-4:3d}: quality (4 bits) = {val} ({quality_name})")
        quality = val
        
        # hasMultiGraphics (1 bit)
        val, b, pos = read_bits(bits, pos, 1)
        has_multi_graphics = (val == 1)
        print(f"Bit {pos-1:3d}: hasMultiGraphics = {b} → {has_multi_graphics}")
        
        if has_multi_graphics:
            val, b, pos = read_bits(bits, pos, 3)
            print(f"Bit {pos-3:3d}: graphicId (3 bits) = {val}")
        
        # autoPrefix (1 bit)
        val, b, pos = read_bits(bits, pos, 1)
        auto_prefix = (val == 1)
        print(f"Bit {pos-1:3d}: autoPrefix = {b} → {auto_prefix}")
        
        if auto_prefix:
            pos += 11  # skip autoPrefixId
            print(f"Bit {pos-11:3d}: autoPrefixId (11 bits) - skipped")
        
        print(f"\nAfter extended fields: pos = {pos}")
        print(f"→ PROPERTIES START HERE AT BIT {pos}")
        
        # Parse properties
        print(f"\n--- PROPERTIES ---")
        prop_num = 0
        while pos + 9 <= len(bits):
            val, prop_id_bits, new_pos = read_bits(bits, pos, 9)
            
            if val == 511:  # End marker
                print(f"Bit {pos:3d}: END MARKER (9 bits) = {prop_id_bits} → 511")
                pos = new_pos
                break
            
            print(f"\nBit {pos:3d}: Property #{prop_num} ID = {val}")
            pos = new_pos
            
            # Assume 9 bits for value (simplified)
            if pos + 9 <= len(bits):
                val, value_bits, pos = read_bits(bits, pos, 9)
                print(f"Bit {pos-9:3d}: Property #{prop_num} Value = {val}")
            
            prop_num += 1
            
            if prop_num > 25:
                print("... (stopping after 25 properties)")
                break
        
        print(f"\nAfter properties: pos = {pos}")
    
    else:
        print(f"\n--- NOT EXTENDED ---")
        print(f"No extended fields, no properties")
    
    print(f"\nFinal position: {pos} bits")
    print(f"Remaining: {len(bits) - pos} bits (padding)")
    
    return {
        'is_extended': is_extended,
        'item_type': item_type,
        'header_end': 60,
        'item_type_end': 92,
        'properties_start': pos if is_extended else None,
        'total_bits': len(bits)
    }

# Parse both files
print("="*70)
print("CORRECT PARSING COMPARISON")
print("="*70)

rich = parse_item_correct('d2i/rich.d2i')
twi = parse_item_correct('d2i/twi_1240052515.d2i')

# Summary
print("\n" + "="*70)
print("SUMMARY")
print("="*70)

print(f"\nrich.d2i:")
print(f"  isExtended: {rich['is_extended']}")
print(f"  Item type: '{rich['item_type']}'")
print(f"  Total: {rich['total_bits']} bits")
print(f"  Structure: [Header 60][Type 32][{'Extended fields + Properties' if rich['is_extended'] else 'Padding'}]")
if rich['properties_start']:
    print(f"  Properties start: bit {rich['properties_start']}")

print(f"\ntwi_1240052515.d2i:")
print(f"  isExtended: {twi['is_extended']}")
print(f"  Item type: '{twi['item_type']}'")
print(f"  Total: {twi['total_bits']} bits")
print(f"  Structure: [Header 60][Type 32][{'Extended fields + Properties' if twi['is_extended'] else 'Padding'}]")
if twi['properties_start']:
    print(f"  Properties start: bit {twi['properties_start']}")

print(f"\n{'='*70}")
print("TO CONVERT rich.d2i TO EXTENDED WITH PROPERTIES:")
print("="*70)
if not rich['is_extended']:
    print("1. Flip isExtended bit (bit 23): 1 → 0")
    print("2. Insert extended fields after bit 92 (after item type)")
    print("3. Insert properties after extended fields")
    print("4. Add end marker (511)")
    print("5. Byte-align")
else:
    print("rich.d2i is ALREADY extended!")
    print(f"Just need to insert properties at bit {rich['properties_start']}")
