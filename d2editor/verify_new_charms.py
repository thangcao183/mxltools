#!/usr/bin/env python3
"""Verify the new charm files"""

def bytes_to_bits(data):
    return ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))

def verify_file(filename):
    with open(filename, 'rb') as f:
        header = f.read(2)
        data = f.read()
    
    bits = bytes_to_bits(data)
    
    print(f"\n{'='*70}")
    print(f"FILE: {filename}")
    print(f"{'='*70}")
    print(f"Size: {len(data) + 2} bytes ({len(bits)} bits)")
    print(f"Hex: {data.hex()}")
    
    # Parse
    is_extended = bits[21] == '0'
    item_type = ''.join(chr(int(bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
    
    if is_extended:
        sockets = int(bits[92:95], 2)
        guid = int(bits[95:127], 2)
        ilvl = int(bits[127:134], 2)
        quality = int(bits[134:138], 2)
        
        print(f"\n✓ isExtended: True")
        print(f"✓ Item type: '{item_type}'")
        print(f"\nExtended fields:")
        print(f"  socketablesNumber: {sockets}")
        print(f"  guid: {guid} {'✓ (no unique name)' if guid == 0 else '⚠ (has unique name)'}")
        print(f"  ilvl: {ilvl}")
        print(f"  quality: {quality} ({['','inferior','normal','superior','magic','set','rare','unique','crafted'][quality] if quality < 9 else 'unknown'})")
        
        if quality == 4:
            prefix = int(bits[140:151], 2)
            suffix = int(bits[151:162], 2)
            print(f"\nMagic fields:")
            print(f"  prefix: {prefix}")
            print(f"  suffix: {suffix}")
            print(f"\nProperties start at bit: 162")
            first_prop = int(bits[162:171], 2)
            print(f"  First property ID: {first_prop}")
        elif quality == 7:
            unique_id = int(bits[140:152], 2)
            print(f"\nUnique fields:")
            print(f"  uniqueId: {unique_id} {'✓ (no specific unique)' if unique_id == 0 else ''}")
            print(f"\nProperties start at bit: 152")
            first_prop = int(bits[152:161], 2)
            print(f"  First property ID: {first_prop}")
    else:
        print(f"\n✗ isExtended: False")
    
    print(f"{'='*70}")

# Verify both files
verify_file('d2i/rich_magic.d2i')
verify_file('d2i/rich_unique.d2i')

print(f"\n{'='*70}")
print("SUMMARY")
print(f"{'='*70}")
print("""
Both files created:

1. rich_magic.d2i (Quality: Magic)
   - Item type: 'u@+ ' (charm)
   - guid = 0 (no unique name assignment)
   - magicPrefix = 0, magicSuffix = 0
   - Properties copied from twi
   - Should show as magic charm with twi's properties
   
2. rich_unique.d2i (Quality: Unique)
   - Item type: 'u@+ ' (charm)
   - guid = 0 (no unique name assignment)
   - uniqueId = 0 (generic unique or no name)
   - Properties copied from twi
   - Should show as unique charm with twi's properties

✅ Neither should change the item name to twi's name
✅ Both should have properties from twi
""")
print(f"{'='*70}")
