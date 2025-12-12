
import sys
import os
from d2i_full_parser import D2ItemParser, ParsedItem, ItemProperty
import sqlite3

def main():
    # Load DB
    property_db = {}
    conn = sqlite3.connect('data/props.db')
    cursor = conn.cursor()
    cursor.execute("SELECT code, name, h_descNegative, h_descStringAdd, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0")
    for row in cursor.fetchall():
        code, name, h_descNegative, h_descStringAdd, addv, bits, paramBits, h_save_param_bits = row
        if paramBits == '': paramBits = None
        if h_save_param_bits == '': h_save_param_bits = None
        property_db[code] = {
            'name': (name or '') + " " + (h_descNegative or '') + " " + (h_descStringAdd or '') or f'prop_{code}',
            'addv': addv if addv is not None else 0,
            'bits': bits,
            'h_saveParamBits': h_save_param_bits
        }
    conn.close()

    parser = D2ItemParser(property_db)
    
    # Create a dummy item or load one
    # Let's load a file if available, or mock one
    # I'll try to load 'amu_1018642689.d2i' which is in the file list
    filename = 'd2i/amulet_clean.d2i'
    if not os.path.exists(filename):
        print(f"File {filename} not found")
        return

    item = parser.parse_file(filename)
    print(f"Initial properties: {len(item.properties)}")
    print(f"End marker pos: {item.end_marker_position}")
    
    # Add 2 properties
    # Prop 0 (Strength) usually has bits=9, value_bits=8?
    # Let's check prop 0
    if 0 in property_db:
        print(f"Prop 0 bits: {property_db[0]['bits']}")
    
    props_to_add = [
        (0, 10, 0), # Strength +10
        (1, 10, 0)  # Energy +10 (assuming 1 is Energy)
    ]
    
    for pid, val, par in props_to_add:
        print(f"Adding prop {pid}...")
        parser.add_property_to_item(item, pid, val, par)
        print(f"End marker pos: {item.end_marker_position}")
        print(f"Properties count: {len(item.properties)}")
        print(f"Top property offset: {item.properties[0].bit_offset}")
        
    print(f"Final properties: {len(item.properties)}")
    
    # Verify offsets
    for i, p in enumerate(item.properties):
        print(f"Prop {i}: ID={p.prop_id} Offset={p.bit_offset}")

if __name__ == "__main__":
    main()
