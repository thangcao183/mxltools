#!/usr/bin/env python3
"""
Parse item_with_blink.d2i using ItemParser's prepend logic
This matches how MedianXLOfflineTools reads items
"""

import sqlite3
from pathlib import Path

def bytes_to_bits_prepend(data):
    """
    Convert bytes to bit string using ItemParser's prepend logic
    ItemParser reads bytes and prepends them, creating MSB-first-per-byte representation
    """
    bits = ""
    # Skip JM header (2 bytes)
    for i in range(2, len(data)):
        byte = data[i]
        byte_bits = ""
        for j in range(7, -1, -1):  # MSB first for each byte
            byte_bits += '1' if (byte >> j) & 1 else '0'
        bits = byte_bits + bits  # PREPEND (like ItemParser)
    return bits

def bits_to_number_msb(bits):
    """Convert bit string to number (MSB-first, standard binary)"""
    result = 0
    for i, bit in enumerate(bits):
        if bit == '1':
            result = (result << 1) | 1
        else:
            result = result << 1
    return result

def load_property_info(prop_id):
    """Load property info from database"""
    db_path = Path(__file__).parent / 'data' / 'props.db'
    if not db_path.exists():
        return None
    
    conn = sqlite3.connect(str(db_path))
    cursor = conn.cursor()
    cursor.execute("SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props WHERE code=?", (prop_id,))
    row = cursor.fetchone()
    conn.close()
    
    if row:
        # Use h_saveParamBits if available (for runeword properties)
        param_bits = 0
        if row[5] is not None and str(row[5]).strip():
            try:
                param_bits = int(row[5])
            except:
                pass
        elif row[4] is not None:
            try:
                param_bits = int(row[4])
            except:
                pass
        
        return {
            'code': row[0],
            'name': row[1],
            'add': row[2] if row[2] is not None else 0,
            'bits': row[3] if row[3] is not None else 0,
            'paramBits': param_bits
        }
    return None

def parse_item_prepend(filename):
    """Parse item using ItemParser's prepend logic"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"\n{'='*70}")
    print(f"ANALYZING (ItemParser prepend logic): {filename}")
    print(f"{'='*70}")
    print(f"File size: {len(data)} bytes\n")
    
    # Use prepend logic
    bits = bytes_to_bits_prepend(data)
    print(f"Total bits (ItemParser prepend): {len(bits)}")
    print(f"First 100 bits: {bits[:100]}")
    print(f"Last 100 bits: {bits[-100:]}\n")
    
    # Parse header
    print(f"--- HEADER ---")
    print(f"Bits 0-15 (unknown): {bits[0:16]}")
    print(f"Bits 16-20 (location): {bits[16:21]} = {bits_to_number_msb(bits[16:21])}")
    print(f"Bit 21 (extended): {bits[21]} = {'Extended' if bits[21] == '1' else 'Non-extended'}")
    
    # Item type (32 bits)
    item_type_bits = bits[60:92]
    item_type = ""
    for i in range(0, 32, 8):
        byte_bits = item_type_bits[i:i+8]
        byte_val = bits_to_number_msb(byte_bits)
        item_type += chr(byte_val) if 32 <= byte_val < 127 else '?'
    print(f"Bits 60-91 (item type): \"{item_type}\"")
    
    # Extended fields
    if bits[21] == '1':
        print(f"\n--- EXTENDED FIELDS ---")
        socketables = bits_to_number_msb(bits[92:95])
        guid = bits_to_number_msb(bits[95:127])
        ilvl = bits_to_number_msb(bits[127:134])
        quality = bits_to_number_msb(bits[134:138])
        
        print(f"Socketables: {socketables}")
        print(f"GUID: {guid}")
        print(f"iLvl: {ilvl}")
        print(f"Quality: {quality}")
        
        # Quality-specific fields
        property_start = 140
        if quality == 4:  # Magic
            magic_prefix = bits_to_number_msb(bits[140:151])
            magic_suffix = bits_to_number_msb(bits[151:162])
            print(f"Magic Prefix: {magic_prefix}")
            print(f"Magic Suffix: {magic_suffix}")
            property_start = 162
        elif quality == 7:  # Unique
            unique_id = bits_to_number_msb(bits[140:152])
            print(f"Unique ID: {unique_id}")
            property_start = 152
        
        # Parse properties
        print(f"\n--- PROPERTIES (starting at bit {property_start}) ---")
        pos = property_start
        prop_num = 1
        
        while pos < len(bits) - 9:
            # Property ID (9 bits)
            prop_id_bits = bits[pos:pos+9]
            prop_id = bits_to_number_msb(prop_id_bits)
            
            print(f"\n#{prop_num} at bit {pos}:")
            print(f"  ID: {prop_id_bits} = {prop_id}")
            
            if prop_id == 511:
                print(f"  → END MARKER")
                break
            
            prop_info = load_property_info(prop_id)
            if prop_info:
                print(f"  Name: {prop_info['name']}")
                print(f"  Bits: {prop_info['bits']}, ParamBits: {prop_info['paramBits']}, Add: {prop_info['add']}")
                
                pos += 9
                
                # Read parameter
                if prop_info['paramBits'] > 0:
                    param_bits = bits[pos:pos+prop_info['paramBits']]
                    param_value = bits_to_number_msb(param_bits)
                    print(f"  Parameter ({prop_info['paramBits']} bits): {param_bits} = {param_value}")
                    if 'skill' in prop_info['name'].lower() or 'aura' in prop_info['name'].lower():
                        print(f"    → Skill/Aura ID: {param_value}")
                    pos += prop_info['paramBits']
                
                # Read value
                if prop_info['bits'] > 0:
                    value_bits = bits[pos:pos+prop_info['bits']]
                    value = bits_to_number_msb(value_bits)
                    actual = value - prop_info['add']
                    print(f"  Value ({prop_info['bits']} bits): {value_bits} = {value} (display: {actual})")
                    pos += prop_info['bits']
                
                prop_num += 1
                if prop_num > 50:
                    print(f"\n⚠️ Too many properties, stopping...")
                    break
            else:
                print(f"  → UNKNOWN PROPERTY")
                break
    
    print(f"\n{'='*70}\n")

if __name__ == '__main__':
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python3 parse_itemparser_style.py <file.d2i>")
        sys.exit(1)
    
    parse_item_prepend(sys.argv[1])
