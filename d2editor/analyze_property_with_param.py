#!/usr/bin/env python3
"""
Analyze item_with_blink.d2i to understand property with parameters
"""

import sqlite3
from pathlib import Path

def bytes_to_bits(data):
    """Convert bytes to bit string (LSB-first within each byte)"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte >> i) & 1 else '0'
    return bits

def bits_to_number(bits):
    """Convert bit string to number (LSB-first)"""
    result = 0
    for i, bit in enumerate(bits):
        if bit == '1':
            result |= (1 << i)
    return result

def load_property_info(prop_id):
    """Load property info from database"""
    db_path = Path(__file__).parent / 'data' / 'props.db'
    if not db_path.exists():
        return None
    
    conn = sqlite3.connect(str(db_path))
    cursor = conn.cursor()
    # Load both paramBits and h_saveParamBits (for runeword properties)
    cursor.execute("SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props WHERE code=?", (prop_id,))
    row = cursor.fetchone()
    conn.close()
    
    if row:
        # Use h_saveParamBits if available (for runeword properties), otherwise use paramBits
        param_bits = 0
        if row[5] is not None and str(row[5]).strip():  # h_saveParamBits
            try:
                param_bits = int(row[5])
            except:
                pass
        elif row[4] is not None:  # paramBits
            try:
                param_bits = int(row[4])
            except:
                pass
        
        return {
            'code': row[0],
            'name': row[1],
            'add': row[2] if row[2] is not None else 0,
            'bits': row[3] if row[3] is not None else 0,
            'paramBits': param_bits,
            'h_saveParamBits': row[5] if row[5] is not None else None
        }
    return None

def parse_item(filename):
    """Parse item file and extract properties"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"\n{'='*70}")
    print(f"ANALYZING: {filename}")
    print(f"{'='*70}")
    print(f"File size: {len(data)} bytes")
    print(f"Hex: {data.hex()}")
    
    # Skip JM header
    bits = bytes_to_bits(data[2:])
    print(f"\nTotal bits (without JM header): {len(bits)}")
    
    # Parse header fields
    print(f"\n--- HEADER FIELDS ---")
    print(f"Bits 0-15 (unknown): {bits[0:16]}")
    print(f"Bits 16-20 (location): {bits[16:21]} = {bits_to_number(bits[16:21])}")
    print(f"Bits 21 (extended): {bits[21]} = {'Extended' if bits[21] == '1' else 'Non-extended'}")
    
    # Parse item type (32 bits after bit 60)
    item_type_bits = bits[60:92]
    item_type_bytes = []
    for i in range(0, 32, 8):
        byte_bits = item_type_bits[i:i+8]
        byte_val = bits_to_number(byte_bits)
        item_type_bytes.append(chr(byte_val) if 32 <= byte_val < 127 else '?')
    item_type = ''.join(item_type_bytes)
    print(f"\nBits 60-91 (item type): {''.join(item_type_bytes)} = \"{item_type}\"")
    
    # Check if extended
    if bits[21] == '1':
        print(f"\n--- EXTENDED FIELDS (bits 92-139) ---")
        socketables = bits_to_number(bits[92:95])
        guid = bits_to_number(bits[95:127])
        ilvl = bits_to_number(bits[127:134])
        quality = bits_to_number(bits[134:138])
        has_multi = bits[138]
        auto_prefix = bits[139]
        
        print(f"Bits 92-94 (socketables): {socketables}")
        print(f"Bits 95-126 (guid): {guid}")
        print(f"Bits 127-133 (ilvl): {ilvl}")
        print(f"Bits 134-137 (quality): {quality}")
        print(f"Bits 138 (hasMultiGraphics): {has_multi}")
        print(f"Bits 139 (autoPrefix): {auto_prefix}")
        
        # Parse quality-specific fields
        property_start = 140
        if quality == 4:  # Magic
            print(f"\n--- MAGIC QUALITY FIELDS ---")
            magic_prefix = bits_to_number(bits[140:151])
            magic_suffix = bits_to_number(bits[151:162])
            print(f"Bits 140-150 (magicPrefix): {magic_prefix}")
            print(f"Bits 151-161 (magicSuffix): {magic_suffix}")
            property_start = 162
        elif quality == 7:  # Unique
            print(f"\n--- UNIQUE QUALITY FIELDS ---")
            unique_id = bits_to_number(bits[140:152])
            print(f"Bits 140-151 (uniqueId): {unique_id}")
            property_start = 152
        
        # Parse properties
        print(f"\n--- PROPERTIES (starting at bit {property_start}) ---")
        pos = property_start
        prop_num = 1
        
        while pos < len(bits) - 9:
            # Read property ID (9 bits)
            if pos + 9 > len(bits):
                break
            
            prop_id_bits = bits[pos:pos+9]
            prop_id = bits_to_number(prop_id_bits)
            
            print(f"\nProperty #{prop_num}:")
            print(f"  Position: bit {pos}")
            print(f"  ID bits (9): {prop_id_bits}")
            print(f"  ID value: {prop_id}")
            
            # Check for end marker
            if prop_id == 511:
                print(f"  → END MARKER (511)")
                print(f"\n--- END OF PROPERTIES ---")
                print(f"End marker at bit {pos}")
                padding_bits = len(bits) - pos - 9
                if padding_bits > 0:
                    print(f"Padding: {padding_bits} bits = {bits[pos+9:]}")
                break
            
            # Load property info from database
            prop_info = load_property_info(prop_id)
            if prop_info:
                print(f"  Name: {prop_info['name']}")
                print(f"  Add: {prop_info['add']}")
                print(f"  Bits: {prop_info['bits']}")
                print(f"  ParamBits: {prop_info['paramBits']}")
                if prop_info['h_saveParamBits']:
                    print(f"  h_saveParamBits: {prop_info['h_saveParamBits']} (runeword property)")
                
                pos += 9  # Skip property ID
                
                # Read parameter if exists
                if prop_info['paramBits'] > 0:
                    param_bits = bits[pos:pos+prop_info['paramBits']]
                    param_value = bits_to_number(param_bits)
                    print(f"  → PARAMETER ({prop_info['paramBits']} bits): {param_bits} = {param_value}")
                    
                    # Try to interpret parameter (for skills, it's usually skill ID)
                    if 'skill' in prop_info['name'].lower() or 'aura' in prop_info['name'].lower():
                        print(f"     (Skill/Aura ID: {param_value})")
                    
                    pos += prop_info['paramBits']
                
                # Read property value
                if prop_info['bits'] > 0:
                    value_bits = bits[pos:pos+prop_info['bits']]
                    value = bits_to_number(value_bits)
                    actual_value = value - prop_info['add']
                    print(f"  → VALUE ({prop_info['bits']} bits): {value_bits} = {value} (displayed as {actual_value})")
                    pos += prop_info['bits']
                
            else:
                print(f"  → UNKNOWN PROPERTY (not in database)")
                break
            
            prop_num += 1
            
            # Safety check
            if prop_num > 50:
                print(f"\n⚠️ Too many properties, stopping...")
                break
    
    print(f"\n{'='*70}\n")

if __name__ == '__main__':
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_property_with_param.py <file.d2i>")
        print("\nExample:")
        print("  python3 analyze_property_with_param.py d2i/item_with_blink.d2i")
        sys.exit(1)
    
    filename = sys.argv[1]
    parse_item(filename)
