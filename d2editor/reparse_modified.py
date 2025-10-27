#!/usr/bin/env python3
"""Re-parse the modified file to see what went wrong"""

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

print("="*70)
print("PARSING MODIFIED FILE")
print("="*70)

item = parser.parse_file('d2i/test_modified.d2i')

if item:
    print(f"\nItem type: {item.item_type}")
    print(f"Quality: {item.quality}")
    print(f"Bitstring length: {len(item.bitstring)}")
    print()
    
    # Check property at position 114
    from bitutils import number_to_binary_lsb

    expected_pattern = number_to_binary_lsb(42, 9) + number_to_binary_lsb(25, 7)
    actual_bits = item.bitstring[114:130]
    
    print(f"Bits at position 114-130 (where property 42 should be):")
    print(f"  Expected: {expected_pattern}")
    print(f"  Actual:   {actual_bits}")
    
    if actual_bits == expected_pattern:
        print("  ✅ MATCH!")
    else:
        print("  ❌ MISMATCH!")
        
        # Decode the actual bits
        def lsb_to_number(bits):
            return int(bits[::-1], 2)
        
        actual_id = lsb_to_number(actual_bits[:9])
        actual_value = lsb_to_number(actual_bits[9:16])
        
        print(f"  Actual decodes to: ID={actual_id}, raw_value={actual_value}")
    
    print()
    print(f"Properties parsed: {len(item.properties)}")
    for i, prop in enumerate(item.properties):
        prop_info = parser.property_db.get(prop.prop_id, {})
        print(f"  [{i}] Property {prop.prop_id} ({prop_info.get('name', 'unknown')}): "
              f"value={prop.value}, param={prop.param}")
