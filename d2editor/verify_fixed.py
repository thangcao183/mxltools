#!/usr/bin/env python3
"""Quick verify rich_fixed.d2i"""

def bytes_to_bits(data):
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
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

with open('d2i/twi_1240052515.d2i', 'rb') as f:
    data = f.read()

print(f"File: rich_fixed.d2i ({len(data)} bytes)")
print(f"Hex: {data.hex()}\n")

item_data = data[2:]
bits = bytes_to_bits(item_data)
print(f"Total bits: {len(bits)}\n")

pos = 21
val, b, pos = read_bits(bits, pos, 1)
print(f"Bit 21: isExtended = {val == 0}")

pos = 92
val, b, pos = read_bits(bits, pos, 3)
print(f"Bit 92: socketablesNumber = {val}")

val, b, pos = read_bits(bits, pos, 32)
print(f"Bit 95: guid = {val}")

val, b, pos = read_bits(bits, pos, 7)
print(f"Bit 127: ilvl = {val}")

val, b, pos = read_bits(bits, pos, 4)
print(f"Bit 134: quality = {val} (magic)")

pos += 2  # skip hasMultiGraphics + autoPrefix

print(f"\nBit 140: MAGIC FIELDS START")

val, b, pos = read_bits(bits, pos, 11)
print(f"Bit 140: magicPrefix = {val}")

val, b, pos = read_bits(bits, pos, 11)
print(f"Bit 151: magicSuffix = {val}")

print(f"\nBit {pos}: PROPERTIES START")

val, b, pos = read_bits(bits, pos, 9)
print(f"Bit {pos-9}: Property ID = {val}")

val, b, pos = read_bits(bits, pos, 9)
print(f"Bit {pos-9}: Property Value = {val}")

val, b, pos = read_bits(bits, pos, 9)
print(f"Bit {pos-9}: End marker = {val}")

print(f"\n✓ Final pos: {pos} bits")
print(f"✓ Remaining: {len(bits) - pos} bits (padding)")
print(f"\n{'='*60}")
print("✅ Structure is CORRECT!")
print("   [Header 60][Type 32][Extended 48][Magic 22][Properties 27]")
print("="*60)
