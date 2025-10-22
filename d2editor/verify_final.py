#!/usr/bin/env python3
"""Verify rich_final.d2i với đúng prepend logic"""

def bytes_to_bits_prepend(data):
    bits = ""
    for byte in data:
        byte_bits = ""
        for i in range(8):
            byte_bits += '1' if (byte & (1 << i)) else '0'
        bits = byte_bits + bits
    return bits

def read_bits(bits, pos, count):
    if pos + count > len(bits):
        return None, None, pos
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits, pos + count

with open('d2i/rich_final.d2i', 'rb') as f:
    data = f.read()

print(f"File: rich_final.d2i ({len(data)} bytes)")
print(f"Hex: {data.hex()}\n")

item_data = data[2:]
bits = bytes_to_bits_prepend(item_data)

print(f"BitString (with prepend): {len(bits)} bits\n")

pos = 21
val, b, pos = read_bits(bits, pos, 1)
print(f"Bit 21: isExtended = {val == 0}")

# Item type
item_type = ""
pos = 60
for i in range(4):
    val, b, pos = read_bits(bits, pos, 8)
    if val != 32 and val != 0:
        item_type += chr(val)
print(f"Bit 60-91: Item type = '{item_type}'")

pos = 92
val, b, pos = read_bits(bits, pos, 3)
print(f"Bit 92: socketablesNumber = {val}")

val, b, pos = read_bits(bits, pos, 32)
print(f"Bit 95: guid = {val}")

val, b, pos = read_bits(bits, pos, 7)
print(f"Bit 127: ilvl = {val}")

val, b, pos = read_bits(bits, pos, 4)
print(f"Bit 134: quality = {val}")

pos += 2

val, b, pos = read_bits(bits, pos, 11)
print(f"Bit 140: magicPrefix = {val}")

val, b, pos = read_bits(bits, pos, 11)
print(f"Bit 151: magicSuffix = {val}")

print(f"\nBit {pos}: PROPERTIES")

val, b, pos = read_bits(bits, pos, 9)
print(f"Bit {pos-9}: Property ID = {val} {'✓' if val == 79 else '❌'}")

val, b, pos = read_bits(bits, pos, 9)
print(f"Bit {pos-9}: Property Value = {val} → {val-100} {'✓' if val == 150 else '❌'}")

val, b, pos = read_bits(bits, pos, 9)
print(f"Bit {pos-9}: End marker = {val} {'✓' if val == 511 else '❌'}")

print(f"\n{'='*60}")
if item_type == 'u@+':
    print("✅ Item type CORRECT!")
else:
    print(f"❌ Item type WRONG! Got '{item_type}', expected 'u@+'")
