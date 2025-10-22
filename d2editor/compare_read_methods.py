#!/usr/bin/env python3
"""
So sánh 2 cách đọc file gốc
"""

def no_prepend(data):
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    return bits

def with_prepend(data):
    bits = ""
    for byte in data:
        byte_bits = ""
        for i in range(8):
            byte_bits += '1' if (byte & (1 << i)) else '0'
        bits = byte_bits + bits
    return bits

def check_item_type(bits, label):
    print(f"\n{label}:")
    item_type = ""
    for i in range(4):
        val = 0
        for j in range(8):
            if bits[60 + i*8 + j] == '1':
                val |= (1 << j)
        if val != 32 and val != 0:
            item_type += chr(val) if 32 <= val < 127 else f'[{val}]'
        print(f"  Char {i}: bits[{60+i*8}:{60+(i+1)*8}] = {bits[60+i*8:60+(i+1)*8]} = {val} = '{chr(val) if 32 <= val < 127 else '?'}'")
    print(f"  Item type: '{item_type}'")
    return item_type

with open('d2i/rich.d2i', 'rb') as f:
    data = f.read()[2:]

print(f"File bytes: {data.hex()}")

bits_no = no_prepend(data)
bits_with = with_prepend(data)

check_item_type(bits_no, "NO prepend")
check_item_type(bits_with, "WITH prepend")

print("\n" + "="*70)
print("CONCLUSION:")
print("="*70)
print("""
The original rich.d2i file was created WITHOUT prepend!
So we should:
1. Read with NO prepend
2. Modify bitString
3. Write with NO prepend

NOT use prepend at all for this file!
""")
