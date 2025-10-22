#!/usr/bin/env python3
"""Parse d2i file correctly like ItemParser does"""

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

# Parse
if data[:2] != b'JM':
    print("❌ Invalid file - no JM header")
    exit(1)

bitstring = bytes_to_bitstring(data[2:])
print(f"📊 BitString: {len(bitstring)} bits")
print()

# Parse from END (ReverseBitReader style)
pos = len(bitstring)

print("=== HEADER (60 bits) ===")
pos -= 60
print(f"Skipped header, pos={pos}")
print()

print("=== ITEM TYPE (32 bits) ===")
# Item type is 32 bits BEFORE the header
# After skipping header (pos now points to start of item type section)
# We need to read 32 more bits backwards
if pos < 32:
    print(f"❌ Not enough bits for item type ({pos} bits available)")
else:
    item_type_bits = bitstring[pos-32:pos]
    item_type = ""
    for i in range(0, 32, 8):
        byte_bits = item_type_bits[i:i+8]
        char_val = int(byte_bits, 2)
        item_type += chr(char_val) if 32 <= char_val < 127 else '.'
    print(f"Item type bits: {item_type_bits}")
    print(f"Item type: '{item_type}' (reversed: '{item_type[::-1]}')")
    pos -= 32
print(f"After item type, pos={pos} (properties start here)")
print()

print("=== PROPERTIES ===")
prop_num = 0
while pos > 0:
    if pos < 9:
        print(f"⚠️ Only {pos} bits left, stopping")
        break
    
    # Read property ID
    prop_id_bits = bitstring[pos-9:pos]
    prop_id = int(prop_id_bits, 2)
    
    if prop_id == 511:
        print(f"✅ Property #{prop_num}: END MARKER (ID=511)")
        pos -= 9
        break
    
    print(f"📦 Property #{prop_num}: ID={prop_id}")
    pos -= 9
    
    # Known properties
    if prop_id == 79:  # Gold Find
        if pos >= 9:
            val_bits = bitstring[pos-9:pos]
            raw_val = int(val_bits, 2)
            display_val = raw_val - 100  # subtract 'add'
            print(f"   Type: Gold Find")
            print(f"   Raw value: {raw_val} (bits: {val_bits})")
            print(f"   Display: +{display_val}% Gold Find")
            pos -= 9
        else:
            print(f"   ❌ Not enough bits for value")
            break
    elif prop_id == 80:  # Magic Find
        if pos >= 9:
            val_bits = bitstring[pos-9:pos]
            raw_val = int(val_bits, 2)
            display_val = raw_val - 100
            print(f"   Type: Magic Find")
            print(f"   Raw value: {raw_val}")
            print(f"   Display: +{display_val}% Magic Find")
            pos -= 9
        else:
            print(f"   ❌ Not enough bits for value")
            break
    else:
        print(f"   ⚠️ Unknown property type, cannot parse further")
        break
    
    prop_num += 1

print()
print(f"✅ Parsing complete!")
print(f"   Remaining bits: {pos}")
if pos > 0:
    print(f"   Remaining data: {bitstring[:pos]}")
