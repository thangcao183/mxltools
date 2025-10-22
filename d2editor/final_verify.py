#!/usr/bin/env python3
"""
CORRECT parser that matches ItemParser logic
"""

def bytes_to_bitstring(data):
    """Convert bytes to bitstring using prepend logic"""
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

# Read file
with open('d2i/rich.d2i.added_v4', 'rb') as f:
    data = f.read()

print(f"✅ File: d2i/rich.d2i.added_v4")
print(f"   Size: {len(data)} bytes")
print(f"   Hex: {data.hex()}")
print()

if data[:2] != b'JM':
    print("❌ Invalid JM header")
    exit(1)

bitstring = bytes_to_bitstring(data[2:])
print(f"📊 BitString: {len(bitstring)} bits")
print()

# BitString structure (from start to end):
# [padding/properties] [item type 32 bits] [header 60 bits]
#
# Parse from END (ReverseBitReader reads backwards):

pos = len(bitstring)

print("=== PARSING (ReverseBitReader style - from END to START) ===")
print()

# 1. Skip header (60 bits from end)
print(f"1. Header: 60 bits")
header_bits = bitstring[pos-60:pos]
pos -= 60
print(f"   Remaining position: {pos}")
print()

# 2. Item type (32 bits before header)
print(f"2. Item Type: 32 bits")
if pos < 32:
    print(f"   ❌ Not enough bits ({pos} available)")
else:
    item_type_bits = bitstring[pos-32:pos]
    print(f"   Bits: {item_type_bits}")
    
    # Decode item type (4 characters, MSB-first per byte)
    item_type = ""
    for i in range(0, 32, 8):
        byte_bits = item_type_bits[i:i+8]
        char_val = int(byte_bits, 2)
        item_type += chr(char_val) if 32 <= char_val < 127 else '?'
    
    # Item type is stored in reverse order in the game
    item_type_display = ''.join(reversed(item_type))
    print(f"   Raw: '{item_type}'")
    print(f"   Display: '{item_type_display}'")
    pos -= 32
    print(f"   Remaining position: {pos}")
print()

# 3. Properties (remaining bits before item type)
print(f"3. Properties: starting at position {pos}")
prop_count = 0
while pos > 0:
    if pos < 9:
        print(f"   ⚠️ Only {pos} bits left (padding)")
        break
    
    # Read property ID (9 bits, MSB-first)
    prop_id_bits = bitstring[pos-9:pos]
    prop_id = int(prop_id_bits, 2)
    
    if prop_id == 511:
        print(f"   ✅ Property #{prop_count}: END MARKER (ID=511)")
        pos -= 9
        break
    
    print(f"   📦 Property #{prop_count}: ID={prop_id} ({prop_id_bits})")
    pos -= 9
    
    # Parse known properties
    if prop_id == 79:  # Gold Find (9 bits value)
        if pos >= 9:
            val_bits = bitstring[pos-9:pos]
            raw_val = int(val_bits, 2)
            display_val = raw_val - 100  # subtract 'add' offset
            print(f"      Type: Gold Find")
            print(f"      Value bits: {val_bits}")
            print(f"      Raw value: {raw_val}")
            print(f"      Display: +{display_val}% Gold Find")
            pos -= 9
        else:
            print(f"      ❌ Not enough bits for value")
            break
    elif prop_id == 80:  # Magic Find
        if pos >= 9:
            val_bits = bitstring[pos-9:pos]
            raw_val = int(val_bits, 2)
            display_val = raw_val - 100
            print(f"      Type: Magic Find")
            print(f"      Display: +{display_val}% Magic Find")
            pos -= 9
        else:
            print(f"      ❌ Not enough bits for value")
            break
    else:
        print(f"      ⚠️ Unknown property type")
        break
    
    prop_count += 1
    print()

print(f"\n✅ Parsing complete!")
print(f"   Remaining bits: {pos}")
if pos > 0:
    print(f"   Remaining data: {bitstring[:pos]} (padding)")
