#!/usr/bin/env python3
"""
Debug: Tại sao item type bị hỏng khi insert extended fields?
"""

def bytes_to_bits_prepend(data):
    bits = ""
    for byte in data:
        byte_bits = ""
        for i in range(8):
            byte_bits += '1' if (byte & (1 << i)) else '0'
        bits = byte_bits + bits
    return bits

def read_item_type(bits):
    item_type = ""
    for i in range(4):
        val = 0
        for j in range(8):
            if bits[60 + i*8 + j] == '1':
                val |= (1 << j)
        if val != 32 and val != 0:
            item_type += chr(val) if 32 <= val < 127 else f'[{val}]'
    return item_type

# Read original file
with open('d2i/rich.d2i', 'rb') as f:
    orig_data = f.read()[2:]

orig_bits = bytes_to_bits_prepend(orig_data)

print("="*70)
print("ORIGINAL FILE")
print("="*70)
print(f"Total bits: {len(orig_bits)}")
print(f"Bits 0-96: {orig_bits[:96]}\n")

print("Item type at bits 60-91:")
for i in range(4):
    char_bits = orig_bits[60 + i*8:60 + (i+1)*8]
    val = 0
    for j, bit in enumerate(char_bits):
        if bit == '1':
            val |= (1 << j)
    print(f"  Char {i}: {char_bits} = {val} = '{chr(val) if 32 <= val < 127 else '?'}'")

item_type = read_item_type(orig_bits)
print(f"\nItem type: '{item_type}'")

# Simulate what our code does
print("\n" + "="*70)
print("SIMULATING OUR CODE")
print("="*70)

# Step 1: Flip bit 21
bits = list(orig_bits)
bits[21] = '0'
bits = ''.join(bits)
print(f"After flipping bit 21:")
print(f"  Item type: '{read_item_type(bits)}'")

# Step 2: Insert 48 bits after bit 92
extended_fields = '0' * 48
bits = bits[:92] + extended_fields + bits[92:]
print(f"\nAfter inserting 48 bits at position 92:")
print(f"  Total bits: {len(bits)}")
print(f"  Item type now at bits 60-91:")
item_type = read_item_type(bits)
print(f"  Item type: '{item_type}'")

# The problem: item type is STILL at bits 60-91
# But we just inserted stuff AFTER bit 92!
# So item type shouldn't change...

print("\n" + "="*70)
print("DIAGNOSIS")
print("="*70)
print(f"""
Original bitString (96 bits):
  [Header 60][Item Type 32][Padding 4]
  
After inserting 48 bits at position 92:
  [Header 60][Item Type 32][INSERTED 48][Padding 4]
  Total: 144 bits
  
Item type is at bits 60-91, we insert at bit 92.
So item type should NOT change!

But in our output, item type IS corrupted.
This means the problem is somewhere else...
""")

# Check the actual bytes
print("Let's check the actual file bytes:")
with open('d2i/rich_final.d2i', 'rb') as f:
    final_data = f.read()

print(f"\nOriginal: {orig_data.hex()}")
print(f"Final:    {final_data[2:].hex()}")

print("\nBytes comparison:")
for i in range(min(len(orig_data), len(final_data)-2)):
    o = orig_data[i] if i < len(orig_data) else 0
    f = final_data[2+i]
    match = "✓" if o == f else "✗"
    print(f"  Byte {i:2d}: {o:02x} → {f:02x} {match}")
