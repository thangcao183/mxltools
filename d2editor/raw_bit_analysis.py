#!/usr/bin/env python3
"""
Raw bit analysis of item_with_blink.d2i
"""

def bytes_to_bits(data):
    """Convert bytes to bit string (LSB-first within each byte)"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte >> i) & 1 else '0'
    return bits

def bits_to_number(bits):
    """Convert bit string to number (LSB-first)"""
    result = 0
    for i, bit in enumerate(bits):
        if bit == '1':
            result |= (1 << i)
    return result

with open('d2i/item_with_blink.d2i', 'rb') as f:
    data = f.read()

print(f"File size: {len(data)} bytes")
print(f"\nHex dump:")
print(data.hex())

print(f"\n{'='*70}")
print("BIT ANALYSIS (LSB-first within each byte)")
print(f"{'='*70}")

bits = bytes_to_bits(data[2:])  # Skip JM header
print(f"\nTotal bits: {len(bits)}")

# Show bits in groups of 50
for i in range(0, min(len(bits), 500), 50):
    end = min(i + 50, len(bits))
    print(f"Bits {i:3d}-{end-1:3d}: {bits[i:end]}")

# Try to find all occurrences of "111111111" (end marker)
print(f"\n{'='*70}")
print("SEARCHING FOR END MARKERS (111111111)")
print(f"{'='*70}")

end_marker = "111111111"
pos = 0
found_positions = []
while True:
    pos = bits.find(end_marker, pos)
    if pos == -1:
        break
    found_positions.append(pos)
    print(f"Found at bit {pos}: ...{bits[max(0,pos-10):pos]}[{end_marker}]{bits[pos+9:min(len(bits),pos+19)]}...")
    pos += 1

print(f"\nTotal end markers found: {len(found_positions)}")

# Check specific bit positions
print(f"\n{'='*70}")
print("KEY BIT POSITIONS")
print(f"{'='*70}")
print(f"Bit 21 (extended): {bits[21]}")
print(f"Bits 92-139 (would be extended fields if extended=1):")
print(f"  {bits[92:140]}")

# Try to identify where properties might start
# In a typical file, properties come after quality fields
# Let's check common starting positions
print(f"\n{'='*70}")
print("POTENTIAL PROPERTY START POSITIONS")
print(f"{'='*70}")

for start_pos in [140, 152, 162, 200, 250]:
    if start_pos + 9 < len(bits):
        prop_id_bits = bits[start_pos:start_pos+9]
        prop_id = bits_to_number(prop_id_bits)
        print(f"At bit {start_pos}: {prop_id_bits} = ID {prop_id}")
