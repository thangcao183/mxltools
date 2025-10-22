#!/usr/bin/env python3

def bytes_to_bitstring(data):
    """Convert bytes to bitstring using prepend logic (like ItemParser)"""
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

# Original rich.d2i
original_bytes = bytes([0x4a, 0x4d, 0x10, 0x00, 0xa0, 0x00, 0x65, 0x00, 0x0e, 0x52, 0x07, 0xb4, 0x02, 0x02])
original_bits = bytes_to_bitstring(original_bytes[2:])  # Skip JM

# Modified rich.d2i.added_v4
modified_bytes = bytes([0x4a, 0x4d, 0x00, 0x02, 0x00, 0x14, 0xa0, 0x0c, 0xc0, 0x41, 0xea, 0x80, 0x56, 0x40, 0xfe, 0x4b, 0x23, 0x0f])
modified_bits = bytes_to_bitstring(modified_bytes[2:])  # Skip JM

print("Original bitString (96 bits):")
print(original_bits)
print()

print("Modified bitString (128 bits):")
print(modified_bits)
print()

print("Expected modification:")
print("  - Original: 96 bits total, last 4 bits are padding")
print("  - Should insert 27 bits (property 18 + end marker 9) at logical offset 92")
print("  - This means: keep first 92 bits, insert 27 new bits, keep remaining 4 bits")
print("  - Expected new length: 92 + 27 + 4 = 123 bits, padded to 128")
print()

print("Comparing first 92 bits (header + item type):")
orig_92 = original_bits[-92:]  # Last 92 bits from END (because reversed)
mod_92 = modified_bits[-92:]
print(f"Original (last 92): {orig_92}")
print(f"Modified (last 92): {mod_92}")
print(f"Match: {orig_92 == mod_92}")
print()

# Check bit by bit
for i in range(min(len(original_bits), len(modified_bits))):
    orig_pos = len(original_bits) - 1 - i
    mod_pos = len(modified_bits) - 1 - i
    
    if orig_pos >= 0 and mod_pos >= 0:
        if original_bits[orig_pos] != modified_bits[mod_pos]:
            print(f"❌ First difference at position {i} from end:")
            print(f"   Original[{orig_pos}] = {original_bits[orig_pos]}")
            print(f"   Modified[{mod_pos}] = {modified_bits[mod_pos]}")
            print(f"   Context (original): {original_bits[max(0,orig_pos-5):orig_pos+6]}")
            print(f"   Context (modified): {modified_bits[max(0,mod_pos-5):mod_pos+6]}")
            break
