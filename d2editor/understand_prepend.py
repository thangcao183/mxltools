#!/usr/bin/env python3
"""
Understand PREPEND logic and byte positions

When reading a file with PREPEND:
- Read bytes 2,3,4...N (after JM header)
- For each byte: bitstring = new_byte_bits + bitstring (PREPEND)
- Result: LAST byte read becomes FIRST 8 bits of bitstring

Example with 3 bytes [A, B, C]:
- Read A: bitstring = "AAAAAAAA"
- Read B: bitstring = "BBBBBBBB" + "AAAAAAAA"
- Read C: bitstring = "CCCCCCCC" + "BBBBBBBB" + "AAAAAAAA"
- Final: "CCCCCCCCBBBBBBBBAAAAAAAA"

So byte index in FILE vs bit position in BITSTRING:
- File byte 0 (after JM) → last 8 bits of bitstring
- File byte 1 → second-to-last 8 bits
- File byte N-1 → first 8 bits of bitstring

Or: file_byte[i] → bitstring[(N-1-i)*8 : (N-i)*8]
"""

# We have 36 bytes in output file
num_bytes = 36

# Byte 20 changed (0-indexed)
changed_byte_index = 20

# Calculate where this byte appears in the bitstring
# Using PREPEND logic: last byte (35) → bits 0-7
#                      byte 34 → bits 8-15
#                      ...
#                      byte 20 → bits (35-20)*8 to (35-20+1)*8
bit_start_in_bitstring = (num_bytes - 1 - changed_byte_index) * 8
bit_end_in_bitstring = bit_start_in_bitstring + 8

print(f"Total bytes: {num_bytes}")
print(f"Total bits: {num_bytes * 8}")
print()
print(f"Changed byte index in file (after JM): {changed_byte_index}")
print(f"This byte appears in bitstring at bits: {bit_start_in_bitstring}-{bit_end_in_bitstring}")
print()

# Property 42 is at bits 114-130 in the bitstring
prop_start = 114
prop_end = 130

print(f"Property 42 in bitstring: bits {prop_start}-{prop_end}")
print()

if bit_start_in_bitstring <= prop_start < bit_end_in_bitstring or \
   bit_start_in_bitstring < prop_end <= bit_end_in_bitstring or \
   (prop_start <= bit_start_in_bitstring and bit_end_in_bitstring <= prop_end):
    print(f"✅ YES! Byte {changed_byte_index} overlaps with property 42")
    overlap_start = max(prop_start, bit_start_in_bitstring)
    overlap_end = min(prop_end, bit_end_in_bitstring)
    print(f"   Overlap: bits {overlap_start}-{overlap_end}")
else:
    print(f"❌ NO overlap between byte {changed_byte_index} and property 42")

# Let's also check which FILE bytes contain the property
print()
print("Property 42 spans bits 114-130 in bitstring")
print("Which file bytes contain these bits?")

for bit_pos in range(prop_start, prop_end):
    # Which byte in bitstring?
    byte_in_bitstring = bit_pos // 8
    # Which byte in file? (reverse the PREPEND)
    byte_in_file = num_bytes - 1 - byte_in_bitstring
    print(f"  Bit {bit_pos} → bitstring byte {byte_in_bitstring} → file byte {byte_in_file}")
