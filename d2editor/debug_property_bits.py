#!/usr/bin/env python3
"""
Debug: Tại sao property bị đọc sai?
Parse chi tiết từng bit để tìm lỗi
"""

def bytes_to_bits(data):
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    return bits

def read_bits(bits, pos, count):
    if pos + count > len(bits):
        print(f"❌ ERROR: pos={pos}, count={count}, total={len(bits)}")
        return None, None, pos
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits, pos + count

with open('d2i/rich_fixed.d2i', 'rb') as f:
    data = f.read()

print("="*70)
print("DETAILED BIT-BY-BIT ANALYSIS")
print("="*70)
print(f"File: rich_fixed.d2i ({len(data)} bytes)")
print(f"Hex: {data.hex()}\n")

item_data = data[2:]
bits = bytes_to_bits(item_data)

print(f"Total bits: {len(bits)}")
print(f"Bitstring:\n{bits}\n")

# Jump to properties section
pos = 162  # Properties start at bit 162

print("="*70)
print("PROPERTY PARSING (starting at bit 162)")
print("="*70)

# Show context around bit 162
start = max(0, pos - 20)
end = min(len(bits), pos + 60)
print(f"Bits [{start}:{end}]:")
print(bits[start:end])
print(" " * (pos - start) + "^" * 9 + " (ID: 9 bits)")
print()

# Read property ID (9 bits)
prop_id, id_bits, pos = read_bits(bits, pos, 9)
print(f"Bit 162-170: Property ID (9 bits)")
print(f"  Bits: {id_bits}")
print(f"  Binary string: {id_bits}")
print(f"  Decimal value: {prop_id}")

# Convert to verify
print(f"\n  Verification:")
for i, bit in enumerate(id_bits):
    if bit == '1':
        print(f"    Bit {i}: 1 × 2^{i} = {1 << i}")

print(f"  → Total = {prop_id}")

if prop_id == 79:
    print(f"  ✓ Correct! ID = 79")
else:
    print(f"  ❌ WRONG! Expected 79, got {prop_id}")

print()

# Read property value (9 bits according to props.tsv)
prop_value, value_bits, pos = read_bits(bits, pos, 9)
print(f"Bit 171-179: Property Value (9 bits)")
print(f"  Bits: {value_bits}")
print(f"  Decimal value: {prop_value}")
print(f"  After subtracting add (100): {prop_value - 100}")

if prop_value == 150:
    print(f"  ✓ Correct! Raw value = 150 → Display value = 50")
else:
    print(f"  ❌ WRONG! Expected 150, got {prop_value}")

print()

# Read end marker (9 bits)
end_marker, end_bits, pos = read_bits(bits, pos, 9)
print(f"Bit 180-188: End Marker (9 bits)")
print(f"  Bits: {end_bits}")
print(f"  Decimal value: {end_marker}")

if end_marker == 511:
    print(f"  ✓ Correct! End marker = 511")
else:
    print(f"  ❌ WRONG! Expected 511, got {end_marker}")

print()
print("="*70)

# Now check what ItemParser ACTUALLY reads
print("\nWHAT ITEMPARSER SEES:")
print("="*70)

# ItemParser reads from END of bitstring (ReverseBitReader!)
print("\n⚠️  CRITICAL: ItemParser uses ReverseBitReader!")
print("ReverseBitReader reads from END to BEGINNING!\n")

# Reverse analysis
print("Let's check if prepend logic affects property reading...")
print(f"\nOriginal file bytes after JM header:")
for i, byte in enumerate(item_data):
    print(f"  Byte {i:2d}: 0x{byte:02x} = {byte:3d} = {bits[i*8:(i+1)*8]}")

print("\n" + "="*70)
print("HYPOTHESIS: Properties are read from wrong position")
print("="*70)

# Maybe properties need to be inserted at different position?
# Or maybe the prepend logic means we need to reverse property bits?

# Let's see what happens if we read from a different position
for test_pos in [140, 151, 162, 170]:
    val, b, _ = read_bits(bits, test_pos, 9)
    print(f"Reading 9 bits from position {test_pos}: {b} → {val}")

print("\n" + "="*70)
print("CHECKING REVERSE BIT ORDER")
print("="*70)

# Check if property bits should be in different order
pos = 162
prop_bits_forward = bits[pos:pos+27]
print(f"Property bits (forward, 27 bits): {prop_bits_forward}")

# Check what value we'd get if bits were reversed
prop_id_reversed = bits[pos:pos+9][::-1]
val_reversed = 0
for i, bit in enumerate(prop_id_reversed):
    if bit == '1':
        val_reversed |= (1 << i)
print(f"\nIf ID bits were REVERSED: {prop_id_reversed} → {val_reversed}")

# Check MSB-first interpretation
val_msb = 0
for i, bit in enumerate(id_bits):
    if bit == '1':
        val_msb |= (1 << (8 - i))
print(f"If ID bits were MSB-first: {id_bits} → {val_msb}")
