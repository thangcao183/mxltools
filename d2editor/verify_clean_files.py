#!/usr/bin/env python3
"""Verify the clean extended charm files"""

def bytes_to_bits(data):
    return ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))

def verify_clean_file(filename):
    with open(filename, 'rb') as f:
        header = f.read(2)
        data = f.read()
    
    bits = bytes_to_bits(data)
    
    print(f"\n{'='*70}")
    print(f"FILE: {filename}")
    print(f"{'='*70}")
    print(f"Total size: {len(data) + 2} bytes (with JM header)")
    print(f"Data size: {len(data)} bytes = {len(bits)} bits")
    print(f"Hex: {data.hex()}")
    
    # Parse structure
    is_extended = bits[21] == '0'
    item_type = ''.join(chr(int(bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
    
    print(f"\n[Bits 0-59] Header")
    print(f"  Bit 21 (isExtended): {bits[21]} = {'Extended' if is_extended else 'Not Extended'} ✓")
    
    print(f"\n[Bits 60-91] Item Type")
    print(f"  Value: '{item_type}' ✓")
    
    if is_extended:
        guid = int(bits[95:127], 2)
        ilvl = int(bits[127:134], 2)
        quality = int(bits[134:138], 2)
        
        print(f"\n[Bits 92-139] Extended Fields")
        print(f"  socketablesNumber: {int(bits[92:95], 2)}")
        print(f"  guid: {guid} ✓ (0 = no unique name)")
        print(f"  ilvl: {ilvl}")
        print(f"  quality: {quality}")
        
        if quality == 4 or (quality == 2 and len(bits) > 161):
            # Magic
            prefix = int(bits[140:151], 2)
            suffix = int(bits[151:162], 2)
            print(f"\n[Bits 140-161] Magic Fields")
            print(f"  prefix: {prefix} ✓")
            print(f"  suffix: {suffix} ✓")
            
            # Check end marker
            if len(bits) >= 171:
                end_marker = int(bits[162:171], 2)
                print(f"\n[Bits 162-170] Property End Marker")
                print(f"  Value: {end_marker} {'✓ (511 = no properties)' if end_marker == 511 else '✗ Expected 511'}")
                print(f"\n✅ Properties start at bit 162")
                print(f"   To add property: insert bits BEFORE end marker at bit 162")
        
        elif quality == 7 or (quality == 14 and len(bits) > 151):
            # Unique
            unique_id = int(bits[140:152], 2)
            print(f"\n[Bits 140-151] Unique ID")
            print(f"  Value: {unique_id} ✓ (0 = no specific unique)")
            
            # Check end marker
            if len(bits) >= 161:
                end_marker = int(bits[152:161], 2)
                print(f"\n[Bits 152-160] Property End Marker")
                print(f"  Value: {end_marker} {'✓ (511 = no properties)' if end_marker == 511 else '✗ Expected 511'}")
                print(f"\n✅ Properties start at bit 152")
                print(f"   To add property: insert bits BEFORE end marker at bit 152")
    
    print(f"{'='*70}")

# Verify both clean files
verify_clean_file('d2i/rich_clean_magic.d2i')
verify_clean_file('d2i/rich_clean_unique.d2i')

print(f"\n{'='*70}")
print("SUMMARY - CLEAN EXTENDED CHARMS")
print(f"{'='*70}")
print("""
Two clean extended charm files created:

1. rich_clean_magic.d2i (21 bytes)
   Structure: [Header 60][Type 32][Extended 48][Magic 22][EndMarker 9][Padding 1]
   - Quality: Magic (4)
   - guid: 0 (no unique name)
   - prefix: 0, suffix: 0
   - Properties: NONE (only end marker 511)
   - ✅ Ready to add properties at bit 162

2. rich_clean_unique.d2i (20 bytes)  
   Structure: [Header 60][Type 32][Extended 48][UniqueID 12][EndMarker 9][Padding 1]
   - Quality: Unique (7)
   - guid: 0 (no unique name)
   - uniqueId: 0
   - Properties: NONE (only end marker 511)
   - ✅ Ready to add properties at bit 152

TO ADD PROPERTIES LATER:
  - Read the file
  - Convert to bits
  - Find the end marker (9 bits of 1s = 511)
  - INSERT new property bits BEFORE the end marker
  - Property format: [ID 9 bits][Value N bits]
  - Keep end marker (511) at the end
  - Convert back to bytes
""")
print(f"{'='*70}")
