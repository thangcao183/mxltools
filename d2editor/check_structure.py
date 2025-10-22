#!/usr/bin/env python3

with open('debug_bitstring.txt', 'r') as f:
    bits = f.read()

print(f'BitString: {len(bits)} bits')
print(bits)
print()

print('Structure after insert at position 4:')
print(f'  Bits 0-3: old padding (4)')
print(f'  Bits 4-30: properties (27)')  
print(f'  Bits 31-62: item type (32)')
print(f'  Bits 63-122: header (60)')
print()

print('1. Old padding (bits 0-3):')
print(f'   {bits[0:4]}')
print()

print('2. Properties (bits 4-30):')
props = bits[4:31]
print(f'   {props} ({len(props)} bits)')
print()

print('3. Item type (bits 31-62):')
item_type_bits = bits[31:63]
print(f'   {item_type_bits}')
for i in range(0, 32, 8):
    byte_bits = item_type_bits[i:i+8]
    c = chr(int(byte_bits, 2))
    print(f'     {byte_bits} = {repr(c)}')
print()

print('4. Header (bits 63-122):')
header = bits[63:123]
print(f'   {header[:40]}... ({len(header)} bits)')
