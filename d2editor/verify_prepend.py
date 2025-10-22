#!/usr/bin/env python3
"""
Verify rich_prepend.d2i with PREPEND logic
Mimics ItemParser's byte reading: bytes are prepended to bitString
"""

def bytes_to_bits_prepend(data):
    """
    Convert bytes to bitString with PREPEND logic
    (mimics ItemParser::parseItem line 80)
    """
    bits = ""
    for byte in data:
        byte_bits = ""
        for i in range(8):
            byte_bits += '1' if (byte & (1 << i)) else '0'
        # PREPEND this byte to the beginning
        bits = byte_bits + bits
    return bits

def read_bits(bits, pos, count):
    """Read bits LSB-first"""
    if pos + count > len(bits):
        return None, None, pos
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits, pos + count

print("="*70)
print("VERIFYING rich_prepend.d2i WITH PREPEND LOGIC")
print("="*70)

with open('d2i/rich_prepend.d2i', 'rb') as f:
    data = f.read()

print(f"\nFile: rich_prepend.d2i ({len(data)} bytes)")
print(f"Hex: {data.hex()}\n")

# Skip JM header
item_data = data[2:]

print(f"Item bytes (after JM):")
for i, byte in enumerate(item_data):
    print(f"  Byte {i:2d}: 0x{byte:02x}")

# Convert with PREPEND logic
bits = bytes_to_bits_prepend(item_data)
print(f"\nBitString (prepended): {len(bits)} bits")
print(f"Bits: {bits[:100]}...\n")

# Parse structure
pos = 21
val, b, pos = read_bits(bits, pos, 1)
print(f"Bit 21: isExtended bit = {b} → isExtended = {val == 0}")

pos = 92
print(f"\n--- EXTENDED FIELDS (starting at bit 92) ---")

val, b, pos = read_bits(bits, pos, 3)
print(f"Bit 92:  socketablesNumber = {val}")

val, b, pos = read_bits(bits, pos, 32)
print(f"Bit 95:  guid = {val}")

val, b, pos = read_bits(bits, pos, 7)
print(f"Bit 127: ilvl = {val}")

val, b, pos = read_bits(bits, pos, 4)
quality = val
print(f"Bit 134: quality = {val}")

pos += 2  # skip hasMultiGraphics + autoPrefix

print(f"\n--- QUALITY FIELDS (quality={quality}) ---")

if quality == 4:  # Magic
    val, b, pos = read_bits(bits, pos, 11)
    print(f"Bit 140: magicPrefix = {val}")
    
    val, b, pos = read_bits(bits, pos, 11)
    print(f"Bit 151: magicSuffix = {val}")

print(f"\n--- PROPERTIES (starting at bit {pos}) ---")

prop_num = 0
while pos + 9 <= len(bits) and prop_num < 10:
    val, prop_bits, pos = read_bits(bits, pos, 9)
    
    if val is None:
        print(f"❌ ERROR: Cannot read property ID")
        break
    
    if val == 511:
        print(f"Bit {pos-9}: END MARKER (9 bits) = {val}")
        break
    
    print(f"\nBit {pos-9}: Property #{prop_num} ID = {val}")
    print(f"         Bits: {prop_bits}")
    
    if val == 79:
        print(f"         ✓ Correct! Gold Find property")
    else:
        print(f"         ❌ WRONG! Expected 79, got {val}")
    
    # Read value
    val, value_bits, pos = read_bits(bits, pos, 9)
    if val is None:
        print(f"❌ ERROR: Cannot read property value")
        break
    
    print(f"Bit {pos-9}:   Value = {val} (raw)")
    print(f"         Bits: {value_bits}")
    print(f"         Display value: {val - 100}")
    
    if val == 150:
        print(f"         ✓ Correct! 150 - 100 = 50")
    else:
        print(f"         ❌ WRONG! Expected 150, got {val}")
    
    prop_num += 1

print(f"\n{'='*70}")
print(f"Final position: {pos} bits")
print(f"Total bits: {len(bits)} bits")
print(f"Remaining: {len(bits) - pos} bits")

if prop_num == 1:
    print(f"\n✅ SUCCESS! Found 1 property with correct ID and value")
else:
    print(f"\n⚠️  WARNING: Found {prop_num} properties")
