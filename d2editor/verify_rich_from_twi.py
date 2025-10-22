#!/usr/bin/env python3
"""Verify rich_from_twi.d2i has correct structure"""

def read_d2i_file(filename):
    with open(filename, 'rb') as f:
        header = f.read(2)
        if header != b'JM':
            return None
        data = f.read()
    bits = ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))
    return bits

def main():
    print("=" * 60)
    print("Verifying: d2i/rich_from_twi.d2i")
    print("=" * 60)
    
    bits = read_d2i_file('d2i/rich_from_twi.d2i')
    if not bits:
        print("ERROR: Cannot read file")
        return
    
    print(f"Total bits: {len(bits)}")
    
    # Check isExtended (bit 21, inverted)
    is_extended = bits[21] == '0'
    print(f"Bit 21: isExtended = {is_extended}")
    
    # Check item type (bits 60-91)
    item_type_bits = bits[60:92]
    item_type = ''.join(chr(int(item_type_bits[i:i+8][::-1], 2)) for i in range(0, 32, 8))
    print(f"Bit 60-91: Item type = '{item_type}'")
    
    # Check quality (bits 134-137)
    quality_bits = bits[134:138]
    quality = int(quality_bits, 2)
    quality_names = {1:'inferior', 2:'normal', 3:'superior', 4:'magic', 5:'set', 6:'rare', 7:'unique', 8:'crafted'}
    print(f"Bit 134-137: Quality = {quality} ({quality_names.get(quality, 'unknown')})")
    
    # Properties start at 152 for unique items
    props_start = 152
    print(f"\nProperties should start at bit: {props_start}")
    
    # Check first property
    if len(bits) > props_start + 9:
        first_prop_bits = bits[props_start:props_start+9]
        first_prop_id = int(first_prop_bits, 2)
        print(f"Bit {props_start}: First property ID = {first_prop_id}")
        
        if first_prop_id == 511:
            print(f"  → End marker (no properties)")
        elif first_prop_id < 200:
            print(f"  → Property exists")
        else:
            print(f"  → WARNING: Unusual property ID")
    
    # Parse all properties
    print(f"\n=== ALL PROPERTIES ===")
    pos = props_start
    prop_num = 0
    while pos < len(bits) - 9:
        prop_id_bits = bits[pos:pos+9]
        prop_id = int(prop_id_bits, 2)
        print(f"Property #{prop_num} at bit {pos}: ID={prop_id}")
        
        if prop_id == 511:
            print(f"  → End marker")
            break
        
        pos += 9
        prop_num += 1
        
        # Skip some bits for value (simplified - would need props.tsv)
        if pos + 20 < len(bits):
            pos += 9  # guess
        else:
            break
        
        if prop_num > 20:
            print("  ... (stopping at 20 properties for safety)")
            break
    
    print(f"\n{'='*60}")
    print(f"File appears to be: {'VALID' if is_extended and item_type.strip() == 'u@+' else 'INVALID'}")
    print(f"{'='*60}")

if __name__ == '__main__':
    main()
