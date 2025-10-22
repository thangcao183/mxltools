#!/usr/bin/env python3
"""
Test to understand EXACT prepend logic by analyzing original rich.d2i
"""

def bytes_to_bitstring_prepend(data):
    """Prepend each byte's MSB-first representation"""
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

# Original rich.d2i
orig_bytes = bytes.fromhex('4a4d10 00 a0 00 65 00 0e 52 07 b4 02 02')

print("Original file bytes (12 after JM):")
for i, b in enumerate(orig_bytes[2:]):
    print(f"  Byte {i}: 0x{b:02x} = {format(b, '08b')}")
print()

bitstring = bytes_to_bitstring_prepend(orig_bytes[2:])
print(f"BitString ({len(bitstring)} bits):")
print(bitstring)
print()

# Expected item type: 'u@+ ' (0x75 0x40 0x2b 0x20)
# In binary: 01110101 01000000 00101011 00100000

# Let's search for this pattern in the bitString
item_type_pattern = '01110101010000000010101100100000'
if item_type_pattern in bitstring:
    pos = bitstring.find(item_type_pattern)
    print(f"✅ Found item type pattern at position {pos}")
    print(f"   Context: ...{bitstring[max(0,pos-10):pos+42]}...")
else:
    print(f"❌ Item type pattern NOT found!")
    print(f"   Looking for: {item_type_pattern}")
    
    # Try reversed
    reversed_pattern = item_type_pattern[::-1]
    if reversed_pattern in bitstring:
        pos = bitstring.find(reversed_pattern)
        print(f"✅ Found REVERSED item type pattern at position {pos}")
    else:
        print(f"❌ Reversed pattern also NOT found")

# Let's check what ItemParser would actually read
# From our analysis: header is 60 bits from END
print(f"\nReading from END (ReverseBitReader style):")
pos = len(bitstring)
print(f"  Total bits: {pos}")

# Skip header
pos -= 60
print(f"  After header (60 bits): pos = {pos}")
print(f"  Header bits: {bitstring[pos:pos+60][:40]}...")

# Item type: next 32 bits backwards
if pos >= 32:
    item_type_bits = bitstring[pos-32:pos]
    print(f"  Item type (32 bits from pos {pos-32} to {pos}):")
    print(f"    Bits: {item_type_bits}")
    for i in range(0, 32, 8):
        bb = item_type_bits[i:i+8]
        val = int(bb, 2)
        ch = chr(val) if 32 <= val < 127 else '?'
        print(f"      {bb} = 0x{val:02x} = '{ch}'")
