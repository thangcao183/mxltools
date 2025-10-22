#!/usr/bin/env python3
"""
Convert rich.d2i (non-extended) to extended with properties
Demo implementation in Python first
"""

def bytes_to_bits(data):
    """Convert bytes to bit string (LSB-first within byte)"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    return bits

def bits_to_bytes(bits):
    """Convert bit string to bytes (LSB-first within byte)"""
    # Pad to byte boundary
    while len(bits) % 8 != 0:
        bits += '0'
    
    result = []
    for i in range(0, len(bits), 8):
        byte = 0
        for j in range(8):
            if bits[i + j] == '1':
                byte |= (1 << j)
        result.append(byte)
    
    return bytes(result)

def number_to_bits_msb(num, width):
    """Convert number to MSB-first binary string"""
    result = ""
    for i in range(width - 1, -1, -1):
        result += '1' if (num & (1 << i)) else '0'
    return result

def number_to_bits_lsb(num, width):
    """Convert number to LSB-first binary string (MSB → reverse → LSB)"""
    msb = number_to_bits_msb(num, width)
    return msb[::-1]  # Reverse

# Read rich.d2i
with open('d2i/rich.d2i', 'rb') as f:
    data = f.read()

print("="*70)
print("CONVERTING rich.d2i TO EXTENDED WITH PROPERTIES")
print("="*70)

# Split JM header and item data
jm_header = data[:2]
item_data = data[2:]

print(f"Original file: {len(data)} bytes")
print(f"Hex: {data.hex()}")

# Convert to bits
bits = bytes_to_bits(item_data)
print(f"\nOriginal bits: {len(bits)} bits")
print(f"Bits: {bits}\n")

# Step 1: Flip isExtended bit (bit 21)
print("Step 1: Flip isExtended bit (bit 21): 1 → 0")
bits_list = list(bits)
bits_list[21] = '0'  # Set to 0 (isExtended = True)
bits = ''.join(bits_list)
print(f"After flip: bit 21 = {bits[21]}")

# Step 2: Insert extended fields after bit 92
print("\nStep 2: Insert extended fields after bit 92")

# Extended fields to insert:
# - socketablesNumber: 3 bits = 0
# - guid: 32 bits = some random value (e.g., 999999999)
# - ilvl: 7 bits = 50
# - quality: 4 bits = 4 (magic)
# - hasMultiGraphics: 1 bit = 0
# - autoPrefix: 1 bit = 0
# Total: 48 bits

extended_fields = ""
extended_fields += number_to_bits_lsb(0, 3)         # socketablesNumber = 0
extended_fields += number_to_bits_lsb(999999999, 32) # guid
extended_fields += number_to_bits_lsb(50, 7)        # ilvl = 50
extended_fields += number_to_bits_lsb(4, 4)         # quality = 4 (magic)
extended_fields += number_to_bits_lsb(0, 1)         # hasMultiGraphics = 0
extended_fields += number_to_bits_lsb(0, 1)         # autoPrefix = 0

print(f"Extended fields ({len(extended_fields)} bits):")
print(f"  socketablesNumber = 0")
print(f"  guid = 999999999")
print(f"  ilvl = 50")
print(f"  quality = 4 (magic)")
print(f"  hasMultiGraphics = 0")
print(f"  autoPrefix = 0")

# Insert after bit 92
bits = bits[:92] + extended_fields + bits[92:]
print(f"\nAfter inserting extended fields: {len(bits)} bits")

# Step 3: Insert property (Gold Find +50)
print("\nStep 3: Insert property (Gold Find, ID=79, value=50)")

# Property: ID=79 (9 bits), add=100, bits=9
# Raw value = 50 + 100 = 150
property_id = 79
property_value = 150

# Create property bits (MSB → reverse → LSB)
prop_id_bits = number_to_bits_lsb(property_id, 9)
prop_value_bits = number_to_bits_lsb(property_value, 9)
end_marker_bits = number_to_bits_lsb(511, 9)  # 9 bits of 1s

property_bits = prop_id_bits + prop_value_bits + end_marker_bits

print(f"Property bits ({len(property_bits)} bits):")
print(f"  ID {property_id} (LSB): {prop_id_bits}")
print(f"  Value {property_value} (LSB): {prop_value_bits}")
print(f"  End marker (LSB): {end_marker_bits}")

# Insert after extended fields (at bit 92 + 48 = 140)
insert_pos = 92 + len(extended_fields)
bits = bits[:insert_pos] + property_bits + bits[insert_pos:]

print(f"\nAfter inserting property: {len(bits)} bits")
print(f"Properties start at bit {insert_pos}")

# Step 4: Byte-align (padding already done in bits_to_bytes)
print("\nStep 4: Byte-align")
print(f"Current bits: {len(bits)}")
padding_needed = (8 - (len(bits) % 8)) % 8
print(f"Padding needed: {padding_needed} bits")

# Convert to bytes (with auto-padding)
new_item_data = bits_to_bytes(bits)

print(f"New item data: {len(new_item_data)} bytes")

# Add JM header
new_file_data = jm_header + new_item_data

print(f"\nFinal file: {len(new_file_data)} bytes")
print(f"Hex: {new_file_data.hex()}")

# Save output
output_file = 'd2i/rich_extended.d2i'
with open(output_file, 'wb') as f:
    f.write(new_file_data)

print(f"\n✅ Saved: {output_file}")

# Verify by parsing
print("\n" + "="*70)
print("VERIFICATION - Parsing new file")
print("="*70)

bits_verify = bytes_to_bits(new_item_data)

def read_bits_verify(bits, pos, count):
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, pos + count

pos = 0
pos += 21  # Skip to isExtended bit
val, pos = read_bits_verify(bits_verify, pos, 1)
print(f"isExtended bit = {val} → isExtended = {val == 0}")

pos = 92  # Jump to extended fields
val, pos = read_bits_verify(bits_verify, pos, 3)
print(f"socketablesNumber = {val}")

val, pos = read_bits_verify(bits_verify, pos, 32)
print(f"guid = {val}")

val, pos = read_bits_verify(bits_verify, pos, 7)
print(f"ilvl = {val}")

val, pos = read_bits_verify(bits_verify, pos, 4)
print(f"quality = {val}")

pos += 2  # Skip hasMultiGraphics + autoPrefix

print(f"\nProperties start at bit {pos}")

val, pos = read_bits_verify(bits_verify, pos, 9)
print(f"Property ID = {val}")

val, pos = read_bits_verify(bits_verify, pos, 9)
print(f"Property Value = {val}")

val, pos = read_bits_verify(bits_verify, pos, 9)
print(f"End marker = {val}")

print("\n✅ Conversion complete!")
