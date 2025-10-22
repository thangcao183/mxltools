#!/usr/bin/env python3
"""
So sánh 2 charm files: rich.d2i (non-extended) vs twi_1240052515.d2i (extended)
Để hiểu cần thêm gì để convert non-extended → extended
"""

def bytes_to_bits(data):
    """Convert bytes to bit string (LSB-first within byte)"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    return bits

def read_bits(bits, pos, count):
    """Read bits and return value (LSB-first)"""
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits

def read_string(bits, pos, length):
    """Read string (4 chars = 32 bits)"""
    result = ""
    for i in range(length):
        val, _ = read_bits(bits, pos + i*8, 8)
        if val != 32:  # Not space
            result += chr(val)
    return result, bits[pos:pos+length*8]

def parse_item(filename):
    """Parse D2I item structure"""
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
    
    print(f"Total bits: {len(bits)}")
    
    pos = 0
    
    # Header flags (32 bits)
    print(f"\n--- HEADER FLAGS (32 bits) ---")
    
    val, b = read_bits(bits, pos, 1)
    is_extended = (val == 0)  # Inverted!
    print(f"Bit {pos:3d}: isExtended = {b} → {is_extended} (inverted bit)")
    pos += 1
    
    val, b = read_bits(bits, pos, 3)
    print(f"Bit {pos:3d}: socketablesNumber = {b} → {val}")
    pos += 3
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isIdentified = {b} → {val}")
    pos += 1
    
    # Skip 6 unknown
    pos += 6
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isBroken = {b} → {val}")
    pos += 1
    
    # Skip 1
    pos += 1
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: hasSocket = {b} → {val}")
    pos += 1
    
    # Skip 1
    pos += 1
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isNew = {b} → {val}")
    pos += 1
    
    # Skip 2
    pos += 2
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isEar = {b} → {val}")
    pos += 1
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: starterItem = {b} → {val}")
    pos += 1
    
    # Skip 3
    pos += 3
    
    val, b = read_bits(bits, pos, 1)
    is_simple = (val == 1)
    print(f"Bit {pos:3d}: simpleItem = {b} → {is_simple}")
    pos += 1
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isEthereal = {b} → {val}")
    pos += 1
    
    # Skip 1
    pos += 1
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isPersonalized = {b} → {val}")
    pos += 1
    
    # Skip 1
    pos += 1
    
    val, b = read_bits(bits, pos, 1)
    print(f"Bit {pos:3d}: isRuneword = {b} → {val}")
    pos += 1
    
    # Skip 5
    pos += 5
    
    print(f"\nAfter header: pos = {pos} (should be 32)")
    
    # Item type (32 bits = 4 chars)
    print(f"\n--- ITEM TYPE (32 bits) ---")
    item_type, type_bits = read_string(bits, pos, 4)
    print(f"Bit {pos:3d}: Item type = '{item_type}'")
    print(f"         Type bits = {type_bits}")
    pos += 32
    
    print(f"\nAfter item type: pos = {pos} (should be 64)")
    
    # Extended fields (only if isExtended)
    if is_extended:
        print(f"\n--- EXTENDED FIELDS ---")
        
        # guid (32 bits)
        val, b = read_bits(bits, pos, 32)
        print(f"Bit {pos:3d}: guid (32 bits) = {val}")
        pos += 32
        
        # ilvl (7 bits)
        val, b = read_bits(bits, pos, 7)
        print(f"Bit {pos:3d}: ilvl (7 bits) = {val}")
        pos += 7
        
        # quality (4 bits)
        val, b = read_bits(bits, pos, 4)
        quality_names = ["", "low", "normal", "superior", "magic", "set", "rare", "unique", "crafted"]
        quality_name = quality_names[val] if val < len(quality_names) else f"unknown({val})"
        print(f"Bit {pos:3d}: quality (4 bits) = {val} ({quality_name})")
        quality = val
        pos += 4
        
        # hasMultiGraphics (1 bit)
        val, b = read_bits(bits, pos, 1)
        has_multi_graphics = (val == 1)
        print(f"Bit {pos:3d}: hasMultiGraphics = {val}")
        pos += 1
        
        if has_multi_graphics:
            val, b = read_bits(bits, pos, 3)
            print(f"Bit {pos:3d}: graphicId (3 bits) = {val}")
            pos += 3
        
        # autoPrefix (1 bit)
        val, b = read_bits(bits, pos, 1)
        auto_prefix = (val == 1)
        print(f"Bit {pos:3d}: autoPrefix = {val}")
        pos += 1
        
        if auto_prefix:
            val, b = read_bits(bits, pos, 11)
            print(f"Bit {pos:3d}: autoPrefixId (11 bits) = {val}")
            pos += 11
        
        # Quality-specific fields
        if quality == 4:  # Magic
            print(f"\n--- MAGIC ITEM FIELDS ---")
            val, b = read_bits(bits, pos, 11)
            print(f"Bit {pos:3d}: magicPrefix (11 bits) = {val}")
            pos += 11
            
            val, b = read_bits(bits, pos, 11)
            print(f"Bit {pos:3d}: magicSuffix (11 bits) = {val}")
            pos += 11
        
        elif quality == 6:  # Rare
            print(f"\n--- RARE ITEM FIELDS ---")
            # rareName1 (8 bits) + rareName2 (8 bits)
            val, b = read_bits(bits, pos, 8)
            print(f"Bit {pos:3d}: rareName1 (8 bits) = {val}")
            pos += 8
            
            val, b = read_bits(bits, pos, 8)
            print(f"Bit {pos:3d}: rareName2 (8 bits) = {val}")
            pos += 8
            
            # 6 prefixes (11 bits each, 1 = has, 0 = no)
            for i in range(6):
                val, b = read_bits(bits, pos, 1)
                if val == 1:
                    prefix_val, _ = read_bits(bits, pos + 1, 11)
                    print(f"Bit {pos:3d}: rarePrefix{i+1} (11 bits) = {prefix_val}")
                    pos += 11
                pos += 1
            
            # 6 suffixes (11 bits each, 1 = has, 0 = no)
            for i in range(6):
                val, b = read_bits(bits, pos, 1)
                if val == 1:
                    suffix_val, _ = read_bits(bits, pos + 1, 11)
                    print(f"Bit {pos:3d}: rareSuffix{i+1} (11 bits) = {suffix_val}")
                    pos += 11
                pos += 1
        
        print(f"\nAfter extended fields: pos = {pos}")
        
        # Properties start here!
        print(f"\n--- PROPERTIES START AT BIT {pos} ---")
        
        # Parse properties
        prop_num = 0
        while pos + 9 <= len(bits):
            val, prop_bits = read_bits(bits, pos, 9)
            
            if val == 511:  # End marker
                print(f"Bit {pos:3d}: Property end marker (9 bits) = {prop_bits} → 511")
                pos += 9
                break
            
            print(f"\nBit {pos:3d}: Property #{prop_num} ID (9 bits) = {val}")
            pos += 9
            
            # For simplicity, assume 9 bits value (need property table for exact)
            if pos + 9 <= len(bits):
                val, value_bits = read_bits(bits, pos, 9)
                print(f"Bit {pos:3d}: Property #{prop_num} Value (9 bits) = {val}")
                pos += 9
            
            prop_num += 1
            
            if prop_num > 20:  # Safety limit
                print("... (stopping after 20 properties)")
                break
    
    else:
        print(f"\n--- NOT EXTENDED (SIMPLE ITEM) ---")
        print(f"No extended fields")
        print(f"No properties")
    
    print(f"\n--- END ---")
    print(f"Final position: {pos} bits")
    print(f"Remaining bits: {len(bits) - pos} bits (padding)")
    
    return {
        'is_extended': is_extended,
        'is_simple': is_simple,
        'item_type': item_type,
        'total_bits': len(bits),
        'header_end': 64,
        'properties_start': pos if is_extended else None
    }

# Parse both files
print("\n" + "="*70)
print("COMPARING TWO CHARM FILES")
print("="*70)

rich = parse_item('d2i/rich.d2i')
twi = parse_item('d2i/twi_1240052515.d2i')

# Summary
print("\n" + "="*70)
print("COMPARISON SUMMARY")
print("="*70)

print(f"\nrich.d2i:")
print(f"  - isExtended: {rich['is_extended']}")
print(f"  - isSimple: {rich['is_simple']}")
print(f"  - Item type: '{rich['item_type']}'")
print(f"  - Total bits: {rich['total_bits']}")
print(f"  - Structure: [Header 32][Type 32][Padding {rich['total_bits']-64}]")

print(f"\ntwi_1240052515.d2i:")
print(f"  - isExtended: {twi['is_extended']}")
print(f"  - isSimple: {twi['is_simple']}")
print(f"  - Item type: '{twi['item_type']}'")
print(f"  - Total bits: {twi['total_bits']}")
if twi['properties_start']:
    print(f"  - Properties start: bit {twi['properties_start']}")
    extended_fields_size = twi['properties_start'] - 64
    print(f"  - Structure: [Header 32][Type 32][Extended {extended_fields_size}][Properties ...][End marker 9][Padding]")

print(f"\n{'='*70}")
print("TO CONVERT rich.d2i → extended:")
print("="*70)
print("1. Flip bit 0 (isExtended): 1 → 0")
print("2. Insert extended fields after bit 64 (after item type):")
print("   - guid (32 bits)")
print("   - ilvl (7 bits)")
print("   - quality (4 bits)")
print("   - hasMultiGraphics (1 bit) → 0")
print("   - autoPrefix (1 bit) → 0")
print("   - Total: ~45 bits for basic extended")
print("3. Insert properties after extended fields")
print("4. Add end marker (511 = 9 bits of 1s)")
print("5. Byte-align with padding")
