#!/usr/bin/env python3
"""
Simple parser - assume properties are at the END of file
"""

with open('d2i/rich.d2i.added_simple', 'rb') as f:
    data = f.read()

print(f"File: {len(data)} bytes")
print(f"Hex: {data.hex()}")
print()

# Original file was 14 bytes, new is 18 bytes
# So last 4 bytes are property bytes
property_bytes = data[14:]
print(f"Property bytes: {property_bytes.hex()}")

# Convert to bits (LSB-first within each byte)
property_bits = ""
for b in property_bytes:
    for i in range(8):
        property_bits += '1' if (b & (1 << i)) else '0'

print(f"Property bits ({len(property_bits)}): {property_bits}")
print()

# Parse properties (LSB-first)
pos = 0
prop_num = 0

while pos < len(property_bits) - 8:
    if pos + 9 > len(property_bits):
        break
    
    # Read 9 bits for property ID (LSB-first, already in correct order)
    prop_id_bits = property_bits[pos:pos+9]
    prop_id = int(prop_id_bits, 2)
    
    print(f"Property #{prop_num}:")
    print(f"  Bits {pos}-{pos+8}: {prop_id_bits} = {prop_id}")
    
    if prop_id == 511:
        print(f"  -> END MARKER ✅")
        break
    
    pos += 9
    
    if prop_id == 79:  # Gold Find
        if pos + 9 <= len(property_bits):
            val_bits = property_bits[pos:pos+9]
            raw_val = int(val_bits, 2)
            display_val = raw_val - 100
            print(f"  -> Gold Find")
            print(f"     Value bits {pos}-{pos+8}: {val_bits} = {raw_val}")
            print(f"     Display: +{display_val}% Gold Find ✅")
            pos += 9
    
    prop_num += 1

print(f"\n✅ Parse complete! Found {prop_num} properties")
