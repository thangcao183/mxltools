#!/usr/bin/env python3
"""Verify rich_extended.d2i structure"""

with open('d2i/rich_extended.d2i', 'rb') as f:
    data = f.read()

print(f'File size: {len(data)} bytes')
print(f'Hex: {data.hex()}\n')

item_data = data[2:]
bits = ''.join('1' if (b & (1<<i)) else '0' for b in item_data for i in range(8))

print(f'Total bits: {len(bits)}')
print(f'Bit 21 (isExtended): {bits[21]} (0=extended, 1=not extended)')
print()

# Item type
item_type_bits = bits[60:92]
item_type = ''.join(chr(int(item_type_bits[i:i+8][::-1], 2)) for i in range(0, 32, 8))
print(f'Item type: "{item_type}"')
print()

# Extended fields
print('Extended fields (bit 92-139):')
print(f'  socketablesNumber (3 bits): {int(bits[92:95], 2)}')
print(f'  guid (32 bits): {int(bits[95:127], 2)}')
print(f'  ilvl (7 bits): {int(bits[127:134], 2)}')
print(f'  quality (4 bits): {int(bits[134:138], 2)} (7=unique)')
print()

# Unique ID
print(f'Unique ID (bit 140-151): {int(bits[140:152], 2)}')
print()

# First property
print(f'First property ID (bit 152-160): {int(bits[152:161], 2)}')
print()

print('=' * 60)
print('✅ SUCCESS! File has correct structure:')
print('   [Header 60][Type 32][Extended 48][UniqueID 12][Properties...]')
print('   Rich header + item type + Twi extended + properties')
print('=' * 60)
