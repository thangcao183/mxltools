#!/usr/bin/env python3
"""Extract the exact bits from twi_1240052515.d2i for copying"""

def read_d2i_file(filename):
    with open(filename, 'rb') as f:
        header = f.read(2)
        if header != b'JM':
            print(f"Invalid header: {header}")
            return None
        data = f.read()
    
    # Convert to bits (no prepend)
    bits = ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))
    return bits

def main():
    bits = read_d2i_file('d2i/twi_1240052515.d2i')
    if not bits:
        return
    
    print(f"Total bits: {len(bits)}")
    print(f"\nBit positions:")
    print(f"  Header: 0-59 (60 bits)")
    print(f"  Item Type: 60-91 (32 bits)")
    print(f"  Extended Fields: 92-139 (48 bits)")
    print(f"  Quality-specific (unique): 140-151 (12 bits)")
    print(f"  Properties: 152+")
    
    # Extract sections
    header = bits[0:60]
    item_type = bits[60:92]
    extended = bits[92:140]
    unique_id = bits[140:152]
    properties = bits[152:]
    
    print(f"\n=== BIT SECTIONS ===")
    print(f"Header (60 bits):")
    print(f"  {header}")
    
    print(f"\nItem Type (32 bits): {''.join(chr(int(item_type[i:i+8][::-1], 2)) for i in range(0, 32, 8))}")
    print(f"  {item_type}")
    
    print(f"\nExtended Fields (48 bits):")
    print(f"  {extended}")
    
    print(f"\nUnique ID (12 bits): {int(unique_id, 2)}")
    print(f"  {unique_id}")
    
    print(f"\nProperties ({len(properties)} bits):")
    print(f"  {properties}")
    
    # Parse properties
    print(f"\n=== PROPERTIES ===")
    pos = 0
    prop_num = 0
    while pos < len(properties) - 9:
        prop_id_bits = properties[pos:pos+9]
        prop_id = int(prop_id_bits, 2)
        print(f"Property #{prop_num}: ID={prop_id}, bits={prop_id_bits}")
        
        if prop_id == 511:
            print(f"  -> End marker at bit {152 + pos}")
            break
        
        # For simplicity, assume each property has some value bits
        # You'd need props.tsv to know exact bit count
        pos += 9
        # Skip ahead to find next property or end
        # This is simplified - real parsing needs props.tsv
        
        # Try to find next 9-bit boundary that looks like property or end
        found_next = False
        for skip in range(1, 50):
            next_id_bits = properties[pos:pos+9]
            if len(next_id_bits) < 9:
                break
            next_id = int(next_id_bits, 2)
            if next_id == 511 or next_id < 200:  # reasonable property ID
                print(f"  -> Value likely {skip} bits: {properties[pos-skip:pos]}")
                found_next = True
                break
        
        if not found_next:
            pos += 1
        
        prop_num += 1
        if prop_num > 20:  # safety
            break
    
    print(f"\n=== FOR COPYING ===")
    print(f"Extended fields to copy (48 bits):")
    print(f"  {extended}")
    print(f"\nUnique ID to copy (12 bits):")
    print(f"  {unique_id}")
    print(f"\nAll property bits from position 152 to end:")
    print(f"  {properties}")
    print(f"  ({len(properties)} bits total)")

if __name__ == '__main__':
    main()
