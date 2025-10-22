#!/usr/bin/env python3
"""
Kiểm tra xem prepend có làm hỏng item type không
So sánh item type giữa file gốc và file đã prepend
"""

def bytes_to_bits_prepend(data):
    """Convert with PREPEND logic (như ItemParser)"""
    bits = ""
    for byte in data:
        byte_bits = ""
        for i in range(8):
            byte_bits += '1' if (byte & (1 << i)) else '0'
        bits = byte_bits + bits  # PREPEND
    return bits

def read_bits(bits, pos, count):
    if pos + count > len(bits):
        return None, None, pos
    result_bits = bits[pos:pos+count]
    value = 0
    for i, bit in enumerate(result_bits):
        if bit == '1':
            value |= (1 << i)
    return value, result_bits, pos + count

def read_item_type(bits):
    """Read item type from bit 60-91"""
    item_type = ""
    pos = 60
    for i in range(4):
        val, b, pos = read_bits(bits, pos, 8)
        if val and val != 32:
            item_type += chr(val)
    return item_type

print("="*70)
print("CHECKING IF PREPEND CORRUPTS ITEM TYPE")
print("="*70)

# Check original file
print("\n1. ORIGINAL FILE (rich.d2i)")
print("-"*70)
with open('d2i/rich.d2i', 'rb') as f:
    data = f.read()

print(f"Size: {len(data)} bytes")
print(f"Hex: {data.hex()}")

item_data = data[2:]
bits = bytes_to_bits_prepend(item_data)
item_type = read_item_type(bits)

print(f"Item type: '{item_type}'")
print(f"Item type should be: 'u@+' (charm code)")

if item_type == 'u@+':
    print("✓ Original item type is CORRECT")
else:
    print(f"❌ Original item type is WRONG! Got '{item_type}'")

# Check prepended file
print("\n2. PREPENDED FILE (rich_prepend.d2i)")
print("-"*70)
with open('d2i/rich_prepend.d2i', 'rb') as f:
    data = f.read()

print(f"Size: {len(data)} bytes")
print(f"Hex: {data.hex()}")

item_data = data[2:]
bits = bytes_to_bits_prepend(item_data)
item_type = read_item_type(bits)

print(f"Item type: '{item_type}'")
print(f"Item type should be: 'u@+' (charm code)")

if item_type == 'u@+':
    print("✓ Prepended item type is CORRECT")
else:
    print(f"❌ Prepended item type is WRONG! Got '{item_type}'")
    print("\n⚠️  PREPEND LOGIC CORRUPTED ITEM TYPE!")
    print("This is why the application crashes!")

# Check working file for comparison
print("\n3. WORKING FILE (twi_1240052515.d2i)")
print("-"*70)
with open('d2i/twi_1240052515.d2i', 'rb') as f:
    data = f.read()

print(f"Size: {len(data)} bytes")
print(f"Hex: {data.hex()[:50]}...")

item_data = data[2:]
bits = bytes_to_bits_prepend(item_data)
item_type = read_item_type(bits)

print(f"Item type: '{item_type}'")
print(f"Item type should be: 'twi' (charm code)")

if item_type == 'twi':
    print("✓ Working file item type is CORRECT")

print("\n" + "="*70)
print("DIAGNOSIS")
print("="*70)
print("""
If prepended file has wrong item type, the problem is:
  We are PREPENDING bytes when writing to file,
  but ItemParser ALREADY expects bytes in prepended order!
  
The original file bytes are ALREADY in prepended order!
So we should NOT prepend again - just write directly!
""")
