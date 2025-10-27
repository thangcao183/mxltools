#!/usr/bin/env python3
"""Trace the entire modification process step by step"""

import sqlite3
from d2i_full_parser import D2ItemParser
from bitutils import number_to_binary_lsb

# Load DB
conn = sqlite3.connect('data/props.db')
cursor = conn.cursor()
property_db = {}
cursor.execute('SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0')
for row in cursor.fetchall():
    code, name, addv, bits, param_bits, h_save_param_bits = row
    if param_bits == '': param_bits = None
    if h_save_param_bits == '': h_save_param_bits = None
    property_db[code] = {
        'name': name or f'prop_{code}', 
        'addv': addv if addv is not None else 0,
        'bits': bits, 
        'paramBits': param_bits, 
        'h_saveParamBits': h_save_param_bits
    }
conn.close()

parser = D2ItemParser(property_db)

print("="*70)
print("STEP 1: Parse original file")
print("="*70)
item = parser.parse_file('d2i/complete/relic_fungus.d2i')

print(f"Original bitstring length: {len(item.bitstring)}")
print(f"Original bitstring (first 150 chars): {item.bitstring[:150]}")
print()

print("="*70)
print("STEP 2: Find property position")
print("="*70)

# Find property 42 pattern
orig_pattern = number_to_binary_lsb(42, 9) + number_to_binary_lsb(21, 7)
pos = item.bitstring.find(orig_pattern)

print(f"Looking for pattern: {orig_pattern}")
print(f"Found at position: {pos}")
print(f"Bits at that position: {item.bitstring[pos:pos+16]}")
print()

print("="*70)
print("STEP 3: Build new property bits")
print("="*70)

new_pattern = number_to_binary_lsb(42, 9) + number_to_binary_lsb(25, 7)
print(f"New pattern: {new_pattern}")
print()

print("="*70)
print("STEP 4: Modify bitstring")
print("="*70)

bitstring_before = item.bitstring
item.bitstring = item.bitstring[:pos] + new_pattern + item.bitstring[pos+16:]

print(f"Bitstring before modification (at pos {pos}): {bitstring_before[pos:pos+16]}")
print(f"Bitstring after modification  (at pos {pos}): {item.bitstring[pos:pos+16]}")
print(f"Modified bitstring length: {len(item.bitstring)}")
print()

# Verify the change
if item.bitstring[pos:pos+16] == new_pattern:
    print("✅ Bitstring modification successful")
else:
    print("❌ Bitstring modification FAILED!")
print()

print("="*70)
print("STEP 5: Convert to bytes")
print("="*70)

output_bytes = parser.bitstring_to_bytes(item.bitstring)
print(f"Output bytes length: {len(output_bytes)}")
print(f"Output bytes (hex): {output_bytes.hex()}")
print()

print("="*70)
print("STEP 6: Convert bytes BACK to bitstring (simulate re-parse)")
print("="*70)

# Simulate what happens when we read the file back
re_bitstring = ''
for byte in output_bytes:
    re_bitstring = format(byte, '08b') + re_bitstring  # PREPEND

print(f"Re-created bitstring length: {len(re_bitstring)}")
print(f"Re-created bitstring (first 150): {re_bitstring[:150]}")
print()

print("="*70)
print("STEP 7: Compare bitstrings")
print("="*70)

if item.bitstring == re_bitstring:
    print("✅ Bitstrings match!")
else:
    print("❌ Bitstrings DON'T match!")
    print()
    print("Finding differences...")
    for i in range(min(len(item.bitstring), len(re_bitstring))):
        if item.bitstring[i] != re_bitstring[i]:
            print(f"  Bit {i}: '{item.bitstring[i]}' → '{re_bitstring[i]}'")
            if i > 5:  # Show context
                print(f"    Context (original):   ...{item.bitstring[max(0,i-5):i+6]}...")
                print(f"    Context (re-created): ...{re_bitstring[max(0,i-5):i+6]}...")
                break

print()
print(f"Property location in modified bitstring: {item.bitstring[pos:pos+16]}")
print(f"Property location in re-created bitstring: {re_bitstring[pos:pos+16]}")
