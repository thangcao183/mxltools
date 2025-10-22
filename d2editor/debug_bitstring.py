#!/usr/bin/env python3
"""Debug: Print exact bitString before and after modification"""

import sys

def byte_to_binary_msb(byte):
    return format(byte, '08b')

def create_bitstring_prepend(data):
    bitstring = ""
    for i in range(2, len(data)):  # Skip JM
        byte_bits = byte_to_binary_msb(data[i])
        bitstring = byte_bits + bitstring  # prepend
    return bitstring

def bitstring_to_bytes_prepend(bitstring):
    """Convert bitString back using prepend (matching ItemParser)"""
    # Pad at end
    extra = len(bitstring) % 8
    if extra != 0:
        bitstring += '0' * (8 - extra)
    
    bytes_list = [0x4A, 0x4D]  # JM header
    for i in range(0, len(bitstring), 8):
        chunk = bitstring[i:i+8]
        byte_val = int(chunk, 2)
        bytes_list.insert(2, byte_val)  # prepend to data section (after JM)
    
    return bytes(bytes_list)

# Read original
with open('d2i/rich.d2i', 'rb') as f:
    orig_data = f.read()

orig_bitstring = create_bitstring_prepend(orig_data)
print(f"Original file: {len(orig_data)} bytes")
print(f"Original bytes: {' '.join(f'{b:02x}' for b in orig_data)}")
print(f"Original bitString: {len(orig_bitstring)} bits")
print(f"Original bitString: {orig_bitstring}\n")

# Read modified
with open('d2i/rich.d2i.added_v3', 'rb') as f:
    mod_data = f.read()

mod_bitstring = create_bitstring_prepend(mod_data)
print(f"Modified file: {len(mod_data)} bytes")
print(f"Modified bytes: {' '.join(f'{b:02x}' for b in mod_data)}")
print(f"Modified bitString: {len(mod_bitstring)} bits")
print(f"Modified bitString: {mod_bitstring}\n")

# What SHOULD the modified bitString be?
# Original: 96 bits
# Add 18 bits (property ID + value) + 9 bits (end marker) + padding
expected_bitstring = orig_bitstring + "111100100011010010" + "111111111" + "00000"
print(f"Expected bitString: {len(expected_bitstring)} bits")
print(f"Expected bitString: {expected_bitstring}\n")

# Convert expected back to bytes
expected_bytes = bitstring_to_bytes_prepend(expected_bitstring)
print(f"Expected bytes: {' '.join(f'{b:02x}' for b in expected_bytes)}")
print(f"Modified bytes: {' '.join(f'{b:02x}' for b in mod_data)}")
print(f"Match: {expected_bytes == mod_data}")
