#!/usr/bin/env python3
"""Parse with LSB-first property values"""

def bytes_to_bitstring(data):
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

with open('d2i/rich.d2i.added_v4', 'rb') as f:
    data = f.read()

print(f"File: {len(data)} bytes - {data.hex()}")

bitstring = bytes_to_bitstring(data[2:])
print(f"BitString: {len(bitstring)} bits\n")

# Structure: [padding 4][properties 27][item type 32][header 60]
pos = len(bitstring)

# Skip header
pos -= 60
print(f"Header: 60 bits, pos now = {pos}")

# Skip item type  
item_type_bits = bitstring[pos-32:pos]
print(f"Item type: {bitstring[pos-32:pos]}")
for i in range(0, 32, 8):
    bb = item_type_bits[i:i+8]
    print(f"  {bb} = '{chr(int(bb, 2))}'")
pos -= 32
print(f"After item type, pos = {pos}\n")

# Parse properties (LSB-first!)
print("Properties (LSB-first):")
prop_num = 0
while pos > 4:  # Leave 4 bits padding
    if pos < 9:
        print(f"  Not enough bits ({pos})")
        break
    
    # Read 9 bits LSB-first
    prop_id_bits = bitstring[pos-9:pos]
    # REVERSE to get actual value (because stored as LSB)
    prop_id = int(prop_id_bits[::-1], 2)
    print(f"  Property #{prop_num}: bits={prop_id_bits}, reversed={prop_id_bits[::-1]}, ID={prop_id}")
    pos -= 9
    
    if prop_id == 511:
        print(f"    -> END MARKER")
        break
    
    if prop_id == 79:  # Gold Find
        if pos >= 9:
            val_bits = bitstring[pos-9:pos]
            raw_val = int(val_bits[::-1], 2)  # Reverse for LSB
            display = raw_val - 100
            print(f"    -> Gold Find: bits={val_bits}, reversed={val_bits[::-1]}, raw={raw_val}, display=+{display}%")
            pos -= 9
        else:
            print(f"    -> Not enough bits for value")
            break
    else:
        print(f"    -> Unknown property")
        break
    
    prop_num += 1

print(f"\nRemaining: {pos} bits")
print(f"Padding: {bitstring[:pos]}")
