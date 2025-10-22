#!/usr/bin/env python3
"""
Debug tool: Parse extended file theo CHÍNH XÁC ItemParser.cpp logic
để tìm lỗi khi click vào item
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
    if pos + count > len(bits):
        print(f"❌ ERROR: Trying to read past bitstring! pos={pos}, count={count}, total={len(bits)}")
        return None, None, pos
    
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits, pos + count

def parse_extended_item_debug(filename):
    """Parse theo ItemParser.cpp với debug chi tiết"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"\n{'='*70}")
    print(f"DEBUGGING: {filename}")
    print(f"{'='*70}")
    print(f"File size: {len(data)} bytes")
    print(f"Hex: {data.hex()}")
    print()
    
    # Skip JM header
    item_data = data[2:]
    bits = bytes_to_bits(item_data)
    
    print(f"Total bits: {len(bits)}")
    print(f"Bits: {bits}\n")
    
    pos = 0
    
    # Parse header (60 bits)
    print("="*70)
    print("HEADER (60 bits)")
    print("="*70)
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isQuest = {b} → {val}")
    
    pos += 3
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isIdentified = {b} → {val}")
    
    pos += 5
    pos += 1
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isSocketed = {b} → {val}")
    
    pos += 2
    pos += 2
    
    val, b, pos = read_bits(bits, pos, 1)
    is_ear = (val == 1)
    print(f"Bit {pos-1:3d}: isEar = {b} → {is_ear}")
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isStarter = {b} → {val}")
    
    pos += 2
    pos += 1
    
    val, b, pos = read_bits(bits, pos, 1)
    is_extended = (val == 0)  # INVERTED
    print(f"Bit {pos-1:3d}: isExtended bit = {b} → isExtended = {is_extended} *** KEY BIT ***")
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isEthereal = {b} → {val}")
    
    pos += 1
    
    val, b, pos = read_bits(bits, pos, 1)
    is_personalized = (val == 1)
    print(f"Bit {pos-1:3d}: isPersonalized = {b} → {is_personalized}")
    
    pos += 1
    
    val, b, pos = read_bits(bits, pos, 1)
    print(f"Bit {pos-1:3d}: isRW = {b} → {val}")
    
    pos += 5
    pos += 8
    pos += 2
    
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
    
    print(f"\n✓ After header: pos = {pos} (expected 60)")
    
    if is_ear:
        print("\n⚠️  This is an EAR item - different parsing!")
        return
    
    # Item type (32 bits)
    print(f"\n{'='*70}")
    print("ITEM TYPE (32 bits)")
    print("="*70)
    
    item_type = ""
    for i in range(4):
        val, b, pos = read_bits(bits, pos, 8)
        if val and val != 32:
            item_type += chr(val)
        print(f"Bit {pos-8:3d}: char[{i}] = {val:3d} = '{chr(val) if 32 <= val < 127 else '?'}'")
    
    print(f"\n✓ Item type = '{item_type}'")
    print(f"✓ After item type: pos = {pos} (expected 92)")
    
    if not is_extended:
        print(f"\n⚠️  Item is NOT extended - no extended fields or properties to parse")
        print(f"✓ Parsing complete at bit {pos}")
        return
    
    # Extended fields
    print(f"\n{'='*70}")
    print("EXTENDED FIELDS")
    print("="*70)
    
    # socketablesNumber (3 bits)
    val, b, pos = read_bits(bits, pos, 3)
    socketables = val
    print(f"Bit {pos-3:3d}: socketablesNumber (3 bits) = {val}")
    
    # guid (32 bits)
    val, b, pos = read_bits(bits, pos, 32)
    guid = val
    print(f"Bit {pos-32:3d}: guid (32 bits) = {val}")
    
    # ilvl (7 bits)
    val, b, pos = read_bits(bits, pos, 7)
    ilvl = val
    print(f"Bit {pos-7:3d}: ilvl (7 bits) = {val}")
    
    # quality (4 bits)
    val, b, pos = read_bits(bits, pos, 4)
    quality = val
    quality_names = ["", "low", "normal", "superior", "magic", "set", "rare", "unique", "crafted"]
    quality_name = quality_names[val] if val < len(quality_names) else f"unknown({val})"
    print(f"Bit {pos-4:3d}: quality (4 bits) = {val} ({quality_name})")
    
    # hasMultiGraphics (1 bit)
    val, b, pos = read_bits(bits, pos, 1)
    has_multi_graphics = (val == 1)
    print(f"Bit {pos-1:3d}: hasMultiGraphics = {b} → {has_multi_graphics}")
    
    if has_multi_graphics:
        val, b, pos = read_bits(bits, pos, 3)
        print(f"Bit {pos-3:3d}:   graphicId (3 bits) = {val}")
    
    # autoPrefix (1 bit)
    val, b, pos = read_bits(bits, pos, 1)
    auto_prefix = (val == 1)
    print(f"Bit {pos-1:3d}: autoPrefix = {b} → {auto_prefix}")
    
    if auto_prefix:
        val, b, pos = read_bits(bits, pos, 11)
        print(f"Bit {pos-11:3d}:   autoPrefixId (11 bits) = {val}")
    
    print(f"\n✓ After extended fields: pos = {pos}")
    
    # ItemBase checks (simplified - we don't have ItemDataBase here)
    # Assuming charm: no defense, no durability, no quantity
    print(f"\n{'='*70}")
    print("ITEM BASE CHECKS (charm assumed)")
    print("="*70)
    print("  Charm → no defense, no durability, no quantity")
    
    # Quality-specific fields
    print(f"\n{'='*70}")
    print(f"QUALITY-SPECIFIC FIELDS (quality={quality})")
    print("="*70)
    
    if quality == 4:  # Magic
        print("Quality = MAGIC (4)")
        val, b, pos = read_bits(bits, pos, 11)
        print(f"Bit {pos-11:3d}: magicPrefix (11 bits) = {val}")
        
        val, b, pos = read_bits(bits, pos, 11)
        print(f"Bit {pos-11:3d}: magicSuffix (11 bits) = {val}")
    
    elif quality == 5:  # Set
        print("Quality = SET (5)")
        val, b, pos = read_bits(bits, pos, 12)
        print(f"Bit {pos-12:3d}: setId (12 bits) = {val}")
    
    elif quality == 6:  # Rare/Crafted
        print("Quality = RARE (6)")
        
        val, b, pos = read_bits(bits, pos, 8)
        print(f"Bit {pos-8:3d}: rareName1 (8 bits) = {val}")
        
        val, b, pos = read_bits(bits, pos, 8)
        print(f"Bit {pos-8:3d}: rareName2 (8 bits) = {val}")
        
        # 6 prefixes
        for i in range(6):
            val, b, pos = read_bits(bits, pos, 1)
            if val == 1:
                prefix_val, _, pos = read_bits(bits, pos, 11)
                print(f"Bit {pos-12:3d}: rarePrefix{i+1} = {prefix_val}")
            else:
                print(f"Bit {pos-1:3d}: rarePrefix{i+1} = none")
        
        # 6 suffixes
        for i in range(6):
            val, b, pos = read_bits(bits, pos, 1)
            if val == 1:
                suffix_val, _, pos = read_bits(bits, pos, 11)
                print(f"Bit {pos-12:3d}: rareSuffix{i+1} = {suffix_val}")
            else:
                print(f"Bit {pos-1:3d}: rareSuffix{i+1} = none")
    
    elif quality == 7:  # Unique
        print("Quality = UNIQUE (7)")
        val, b, pos = read_bits(bits, pos, 12)
        print(f"Bit {pos-12:3d}: uniqueId (12 bits) = {val}")
    
    # Personalized name
    if is_personalized:
        print(f"\n{'='*70}")
        print("PERSONALIZED NAME")
        print("="*70)
        
        name = ""
        for i in range(16):
            val, b, pos = read_bits(bits, pos, 7)
            if val == 0:
                break
            name += chr(val)
        print(f"Personalized name: '{name}'")
    
    # Properties!
    print(f"\n{'='*70}")
    print(f"PROPERTIES START AT BIT {pos}")
    print("="*70)
    
    prop_num = 0
    max_props = 30
    
    while pos + 9 <= len(bits) and prop_num < max_props:
        val, prop_bits, pos = read_bits(bits, pos, 9)
        
        if val is None:
            print(f"\n❌ ERROR: Cannot read property ID at bit {pos-9}")
            break
        
        if val == 511:
            print(f"\nBit {pos-9:3d}: END MARKER (9 bits) = {prop_bits} → 511")
            print(f"\n✓ Properties parsing complete!")
            break
        
        print(f"\nBit {pos-9:3d}: Property #{prop_num}")
        print(f"           Property ID = {val} (bits: {prop_bits})")
        
        # For debug, assume 9 bits value (need property table for exact)
        if pos + 9 <= len(bits):
            val, value_bits, pos = read_bits(bits, pos, 9)
            if val is None:
                print(f"❌ ERROR: Cannot read property value at bit {pos-9}")
                break
            print(f"Bit {pos-9:3d}:   Value = {val} (bits: {value_bits})")
        else:
            print(f"⚠️  WARNING: Not enough bits for property value!")
            break
        
        prop_num += 1
    
    if prop_num >= max_props:
        print(f"\n⚠️  WARNING: Reached max properties ({max_props}) without finding end marker!")
        print(f"   This could indicate missing or incorrect end marker!")
    
    print(f"\n{'='*70}")
    print("PARSING SUMMARY")
    print("="*70)
    print(f"Final position: {pos} bits")
    print(f"Total bits: {len(bits)} bits")
    print(f"Remaining: {len(bits) - pos} bits (should be padding)")
    
    if pos > len(bits):
        print(f"\n❌ CRITICAL ERROR: Tried to read past end of bitstring!")
        print(f"   This will cause application crash!")
    elif len(bits) - pos > 16:
        print(f"\n⚠️  WARNING: Large amount of unparsed data remaining!")
        print(f"   Unparsed bits: {bits[pos:]}")

# Parse both files for comparison
print("="*70)
print("COMPARING ORIGINAL vs CONVERTED")
print("="*70)

# Parse original working item
print("\n" + "="*70)
print("1. ORIGINAL WORKING ITEM (twi_1240052515.d2i)")
print("="*70)
parse_extended_item_debug('d2i/twi_1240052515.d2i')

# Parse our converted item
print("\n\n" + "="*70)
print("2. CONVERTED ITEM (rich_cpp_extended.d2i)")
print("="*70)
parse_extended_item_debug('d2i/rich_cpp_extended.d2i')
