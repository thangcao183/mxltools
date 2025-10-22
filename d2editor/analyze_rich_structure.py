#!/usr/bin/env python3
"""
Phân tích chi tiết structure của rich.d2i (non-extended item)
"""

def read_bits_lsb(data, pos, count):
    """Read bits LSB-first from byte array"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    
    return value, result_bits, pos + count

def read_bits_msb(data, pos, count):
    """Read bits MSB-first from byte array"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << (count - 1 - i))
    
    return value, result_bits, pos + count

# Read rich.d2i
with open('d2i/rich.d2i', 'rb') as f:
    data = f.read()

print(f"File: rich.d2i")
print(f"Size: {len(data)} bytes")
print(f"Hex: {data.hex()}")
print()

# Skip JM header
item_data = data[2:]

# Convert to bitstring (LSB-first within byte)
bits = ""
for byte in item_data:
    for i in range(8):
        bits += '1' if (byte & (1 << i)) else '0'

print(f"Total bits: {len(bits)}")
print(f"Bits: {bits}")
print()

# Parse according to ItemParser logic
pos = 0

# Read header (similar to ItemParser)
print("=== PARSING STRUCTURE ===")

# isExtended bit (inverted)
val, b, pos = read_bits_lsb(item_data, pos, 1)
is_extended = (val == 0)
print(f"Pos {pos-1:3d}: isExtended bit = {b} → isExtended = {is_extended}")

# socketablesNumber (3 bits)
val, b, pos = read_bits_lsb(item_data, pos, 3)
print(f"Pos {pos-3:3d}: socketablesNumber = {b} → {val}")

# isIdentified (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: isIdentified = {b} → {val}")

# 6 unknown bits
val, b, pos = read_bits_lsb(item_data, pos, 6)
print(f"Pos {pos-6:3d}: unknown (6 bits) = {b}")

# isBroken (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: isBroken = {b} → {val}")

# 1 unknown bit
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: unknown (1 bit) = {b}")

# hasSocket (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: hasSocket = {b} → {val}")

# 1 unknown bit
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: unknown (1 bit) = {b}")

# isNew (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: isNew = {b} → {val}")

# 2 unknown bits
val, b, pos = read_bits_lsb(item_data, pos, 2)
print(f"Pos {pos-2:3d}: unknown (2 bits) = {b}")

# isEar (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
is_ear = (val == 1)
print(f"Pos {pos-1:3d}: isEar = {b} → {is_ear}")

# starterItem (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: starterItem = {b} → {val}")

# 3 unknown bits
val, b, pos = read_bits_lsb(item_data, pos, 3)
print(f"Pos {pos-3:3d}: unknown (3 bits) = {b}")

# simpleItem (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
is_simple = (val == 1)
print(f"Pos {pos-1:3d}: simpleItem = {b} → {is_simple}")

# isEthereal (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: isEthereal = {b} → {val}")

# 1 unknown bit
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: unknown (1 bit) = {b}")

# isPersonalized (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
is_personalized = (val == 1)
print(f"Pos {pos-1:3d}: isPersonalized = {b} → {is_personalized}")

# 1 unknown bit
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: unknown (1 bit) = {b}")

# isRuneword (1 bit)
val, b, pos = read_bits_lsb(item_data, pos, 1)
print(f"Pos {pos-1:3d}: isRuneword = {b} → {val}")

# 5 unknown bits
val, b, pos = read_bits_lsb(item_data, pos, 5)
print(f"Pos {pos-5:3d}: unknown (5 bits) = {b}")

print(f"\nAfter header: pos = {pos}")

# Item type (32 bits, but stored as 4-char string)
# Read as 4 bytes
type_bits = bits[pos:pos+32]
print(f"\nPos {pos:3d}: Item type (32 bits) = {type_bits}")

# Convert to string (8 bits per char, LSB-first per byte)
item_type = ""
for i in range(0, 32, 8):
    char_bits = type_bits[i:i+8]
    char_val = 0
    for j, bit in enumerate(char_bits):
        if bit == '1':
            char_val |= (1 << j)
    if char_val != 32:  # Skip spaces
        item_type += chr(char_val)
pos += 32

print(f"         Item type string = '{item_type}'")

print(f"\nAfter item type: pos = {pos}")
print(f"Remaining bits: {len(bits) - pos}")

# If isExtended, we would read more fields here
if is_extended:
    print("\n=== EXTENDED FIELDS (if it were extended) ===")
    # These fields would be read but rich.d2i doesn't have them
else:
    print("\n=== NOT EXTENDED - No extended fields ===")
    print("To make it extended, we need to:")
    print("1. Flip isExtended bit (pos 0): 1 → 0")
    print("2. Insert extended fields after pos 32 (after header)")
    print("3. Insert properties after extended fields")

print(f"\n=== SUMMARY ===")
print(f"Current structure:")
print(f"  [Header: 32 bits][Item type: 32 bits][Padding: {len(bits)-64} bits]")
print(f"\nTo make extended:")
print(f"  [Header: 32 bits (flip bit 0)][Item type: 32 bits][Extended fields: ~50 bits][Properties: variable][End marker: 9 bits][Padding]")
