#!/usr/bin/env python3
"""
Verify the correct_order implementation
"""

def parse_lsb_properties(data):
    """Parse property bits (LSB-first)"""
    # Skip JM header (2 bytes)
    item_data = data[2:]
    
    # Convert to bits (LSB-first within byte)
    bits = ""
    for byte in item_data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    
    print(f"Total bits: {len(bits)}")
    print(f"Bits: {bits}")
    
    # Parse properties from the END (where we appended them)
    # Original file was 14 bytes = 12 bytes item data = 96 bits
    # We appended 27 bits (property + end marker)
    
    original_bits = 96
    appended_bits = bits[original_bits:]
    
    print(f"\nOriginal bits: {bits[:original_bits]}")
    print(f"Appended bits: {appended_bits} (length={len(appended_bits)})")
    
    # Parse appended bits
    pos = 0
    prop_num = 0
    
    while pos + 9 <= len(appended_bits):
        # Read property ID (9 bits LSB-first)
        prop_id_bits = appended_bits[pos:pos+9]
        prop_id = 0
        for i, bit in enumerate(prop_id_bits):
            if bit == '1':
                prop_id |= (1 << i)
        
        print(f"\nProperty #{prop_num}:")
        print(f"  Bits: {prop_id_bits}")
        print(f"  ID: {prop_id}")
        
        if prop_id == 511:
            print("  → END MARKER found!")
            break
        
        # Read value (9 bits for property 79)
        pos += 9
        if pos + 9 <= len(appended_bits):
            prop_value_bits = appended_bits[pos:pos+9]
            prop_value = 0
            for i, bit in enumerate(prop_value_bits):
                if bit == '1':
                    prop_value |= (1 << i)
            print(f"  Value bits: {prop_value_bits}")
            print(f"  Value: {prop_value}")
            pos += 9
        
        prop_num += 1

# Read the file
with open('d2i/rich.d2i.correct_order', 'rb') as f:
    data = f.read()

print(f"File: d2i/rich.d2i.correct_order")
print(f"Size: {len(data)} bytes")
print(f"Hex: {data.hex()}")
print("\n" + "="*60)

parse_lsb_properties(data)
