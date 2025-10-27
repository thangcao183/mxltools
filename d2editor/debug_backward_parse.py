#!/usr/bin/env python3
"""Debug the backward property parsing"""

import sqlite3
from d2i_full_parser import D2ItemParser

# Load DB
conn = sqlite3.connect('data/props.db')
cursor = conn.cursor()
property_db = {}
cursor.execute('SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0')
for row in cursor.fetchall():
    code, name, addv, bits, param_bits, h_save_param_bits = row
    if param_bits == '': param_bits = None
    if h_save_param_bits == '': h_save_param_bits = None
    property_db[code] = {
        'name': name or f'prop_{code}', 
        'addv': addv if addv is not None else 0,
        'bits': bits, 
        'paramBits': param_bits, 
        'h_saveParamBits': h_save_param_bits
    }
conn.close()

parser = D2ItemParser(property_db)
item = parser.parse_file('d2i/complete/relic_fungus.d2i')

# Try to manually parse backward from end marker
def binary_lsb_to_number(bits: str) -> int:
    """Convert LSB-first binary string to number"""
    return int(bits[::-1], 2)

bitstring = item.bitstring
end_marker = '111111111'
end_marker_pos = bitstring.rfind(end_marker)

print(f"Bitstring length: {len(bitstring)}")
print(f"End marker at position: {end_marker_pos}")
print()

# Start parsing backward
current_pos = end_marker_pos
properties_found = []

print("Parsing backward from end marker:")
for attempt in range(10):  # Try up to 10 properties
    if current_pos < 9:
        print(f"  Not enough bits ({current_pos} < 9)")
        break
    
    # Read last 9 bits
    prop_id_bits = bitstring[current_pos-9:current_pos]
    prop_id = binary_lsb_to_number(prop_id_bits)
    
    print(f"\nAttempt {attempt + 1} at position {current_pos}:")
    print(f"  Prop ID bits: {prop_id_bits}")
    print(f"  Prop ID: {prop_id}")
    
    # Check if valid
    prop_info = property_db.get(prop_id)
    if not prop_info:
        print(f"  ❌ Invalid property ID {prop_id} - stopping")
        break
    
    print(f"  ✅ Valid property: {prop_info['name']}")
    
    # Get total bits
    param_bits = prop_info.get('paramBits') or prop_info.get('h_saveParamBits') or 0
    if param_bits:
        param_bits = int(param_bits)
    value_bits = prop_info['bits']
    total_bits = 9 + param_bits + value_bits
    
    print(f"  Total bits: {total_bits} (ID:9 + Param:{param_bits} + Value:{value_bits})")
    
    if current_pos < total_bits:
        print(f"  ❌ Not enough bits ({current_pos} < {total_bits})")
        break
    
    prop_start = current_pos - total_bits
    print(f"  Property at position {prop_start}-{current_pos}")
    
    properties_found.insert(0, (prop_start, total_bits, prop_id))
    current_pos = prop_start

print(f"\n\nFound {len(properties_found)} properties")
for i, (start, total, pid) in enumerate(properties_found):
    print(f"  [{i}] Property {pid} at {start}-{start+total}")
