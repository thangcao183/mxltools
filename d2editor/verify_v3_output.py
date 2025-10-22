#!/usr/bin/env python3
"""Verify property_adder_v3 output by parsing as ItemParser would"""

def bytes_to_bitstring(data):
    """Convert bytes to bitstring using prepend logic (like ItemParser)"""
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

def read_bits_from_end(bitstring, pos, length):
    """Read bits like ReverseBitReader (from end to beginning)"""
    if pos < length:
        raise ValueError(f"Cannot read {length} bits from position {pos}")
    new_pos = pos - length
    value_str = bitstring[new_pos:pos]
    return int(value_str, 2), new_pos

# Read the file
with open('d2i/rich.d2i.added_v4', 'rb') as f:
    data = f.read()

print(f"File size: {len(data)} bytes")
print(f"Hex: {data.hex()}")
print()

# Skip JM header
item_bytes = data[2:]
bitstring = bytes_to_bitstring(item_bytes)

print(f"BitString length: {len(bitstring)} bits")
print(f"BitString: {bitstring}")
print()

# Parse like ReverseBitReader (from end to beginning)
pos = len(bitstring)

print("=== PARSING HEADER (ReverseBitReader style) ===")

# Simple item header: 60 bits
bit_fields = [
    (10, "isQuest"),
    (4, "unknown1"),  
    (1, "isIdentified"),
    (6, "unknown2"),
    (1, "unknown3"),
    (4, "isSocketed"),
    (1, "unknown4"),
    (4, "isPickedUp"),
    (1, "isEar"),
    (4, "isStarterItem"),
    (1, "unknown5"),
    (3, "isSimpleItem"),
    (1, "isEthereal"),
    (1, "unknown6"),
    (1, "isPersonalized"),
    (1, "unknown7"),
    (1, "isRuneword"),
    (15, "unknown8"),
]

# Skip to after basic header
pos -= 60
print(f"After skipping 60-bit header, pos={pos}")

# Item type: 4 characters * 8 bits = 32 bits
item_type_bits = bitstring[pos-32:pos]
print(f"\nItem type bits (32): {item_type_bits}")

# Convert to characters (MSB-first, 8 bits each)
item_type = ""
for i in range(0, 32, 8):
    char_bits = item_type_bits[i:i+8]
    char_code = int(char_bits, 2)
    item_type += chr(char_code) if 32 <= char_code < 127 else '?'
print(f"Item type: '{item_type}'")

pos -= 32
print(f"After item type, pos={pos} (this is where properties start)")
print()

print("=== PARSING PROPERTIES ===")
prop_count = 0
while pos > 0:
    if pos < 9:
        print(f"Not enough bits for property ID ({pos} bits remaining)")
        break
    
    # Read property ID (9 bits)
    prop_id_bits = bitstring[pos-9:pos]
    prop_id = int(prop_id_bits, 2)
    print(f"Property #{prop_count}: ID={prop_id} ({prop_id_bits}) at pos {pos}")
    
    if prop_id == 511:
        print("  -> END MARKER found!")
        pos -= 9
        break
    
    if prop_id == 79:  # Gold Find
        print("  -> Gold Find property!")
        pos -= 9
        if pos >= 9:
            value_bits = bitstring[pos-9:pos]
            value = int(value_bits, 2)
            display_value = value - 100  # subtract 'add'
            print(f"     Value bits: {value_bits}")
            print(f"     Raw value: {value}")
            print(f"     Display value: {display_value}%")
            pos -= 9
    else:
        print(f"  -> Unknown property, stopping parse")
        break
    
    prop_count += 1

print(f"\nRemaining bits: {pos}")
print(f"Final bitstring section: {bitstring[:pos]}")
