#!/usr/bin/env python3
"""So sánh structure của working file vs new file"""

def parse_structure(filename):
    with open(filename, 'rb') as f:
        data = f.read()[2:]
    
    bits = ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))
    
    print(f"\n{'='*60}")
    print(f"{filename}")
    print(f"{'='*60}")
    print(f"Total bits: {len(bits)}")
    
    # Header
    is_extended = bits[21] == '0'
    print(f"Bit 21: isExtended = {is_extended}")
    
    # Item type
    item_type = ''
    for i in range(4):
        v = sum((bits[60+i*8+j]=='1')<<j for j in range(8))
        if v not in [0, 32]:
            item_type += chr(v)
    print(f"Bit 60-91: Item type = '{item_type}'")
    
    if is_extended:
        # Extended fields
        quality = sum((bits[134+j]=='1')<<j for j in range(4))
        quality_names = ["?", "low", "normal", "superior", "magic", "set", "rare", "unique", "crafted"]
        print(f"Bit 134-137: Quality = {quality} ({quality_names[quality] if quality < len(quality_names) else '?'})")
        
        # Calculate where properties should start
        pos = 140  # After extended base fields
        
        if quality == 4:  # Magic
            print(f"  → MAGIC needs 22 bits (prefix 11 + suffix 11)")
            pos += 22
        elif quality == 5:  # Set
            print(f"  → SET needs 12 bits (setId)")
            pos += 12
        elif quality == 7:  # Unique
            print(f"  → UNIQUE needs 12 bits (uniqueId)")
            pos += 12
        
        print(f"\nProperties should start at bit: {pos}")
        
        # Read first property
        if pos + 9 <= len(bits):
            prop_id = sum((bits[pos+j]=='1')<<j for j in range(9))
            print(f"Bit {pos}: First property ID = {prop_id}")
            if prop_id == 511:
                print("  → END MARKER (no properties)")
            else:
                print(f"  → Property exists")

parse_structure('d2i/twi_1240052515.d2i')
parse_structure('d2i/rich_no_prepend.d2i')

print("\n" + "="*60)
print("COMPARISON")
print("="*60)
print("""
Both files should have same structure:
- isExtended = True
- Extended fields at bit 92-139
- Quality-specific fields after bit 140
- Properties after quality fields
- End marker (511)
""")
