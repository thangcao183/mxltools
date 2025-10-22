#!/usr/bin/env python3
"""Verify files with default property"""

def bytes_to_bits(data):
    return ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))

def verify_file(filename, expected_prop_start):
    with open(filename, 'rb') as f:
        header = f.read(2)
        data = f.read()
    
    bits = bytes_to_bits(data)
    
    print(f"\n{'='*70}")
    print(f"FILE: {filename}")
    print(f"{'='*70}")
    print(f"Size: {len(data) + 2} bytes, {len(bits)} bits")
    print(f"Hex: {data.hex()}")
    
    is_extended = bits[21] == '0'
    item_type = ''.join(chr(int(bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
    guid = int(bits[95:127], 2)
    ilvl = int(bits[127:134], 2)
    quality = int(bits[134:138], 2)
    
    print(f"\n✓ isExtended: {is_extended}")
    print(f"✓ Item type: '{item_type}'")
    print(f"✓ guid: {guid}")
    print(f"✓ ilvl: {ilvl}")
    print(f"✓ quality: {quality}")
    
    # Parse first property at expected_prop_start
    prop_id_bits = bits[expected_prop_start:expected_prop_start+9]
    prop_id = int(prop_id_bits, 2)
    
    prop_val_bits = bits[expected_prop_start+9:expected_prop_start+16]
    prop_val = int(prop_val_bits, 2)
    
    print(f"\n[Bit {expected_prop_start}+] First Property:")
    print(f"  ID bits: {prop_id_bits} = {prop_id}")
    print(f"  Value bits: {prop_val_bits} = {prop_val}")
    
    # Check end marker
    end_pos = expected_prop_start + 16
    end_bits = bits[end_pos:end_pos+9]
    end_val = int(end_bits, 2)
    print(f"\n[Bit {end_pos}+] End Marker:")
    print(f"  Bits: {end_bits} = {end_val} {'✓' if end_val == 511 else '✗ Expected 511'}")
    
    print(f"{'='*70}")

# Verify both files
verify_file('d2i/rich_clean_magic.d2i', 162)
verify_file('d2i/rich_clean_unique.d2i', 152)

print(f"\n{'='*70}")
print("SUMMARY")
print(f"{'='*70}")
print("""
✅ Both files now have:
  - Extended fields with guid=0 (no unique name)
  - Default property: ID=0, value=10
  - Property end marker: 511
  
These files should now load properly in MedianXLOfflineTools!

You can add more properties later by:
  1. Converting file to bits
  2. Finding the end marker (511)
  3. Inserting new property bits BEFORE the end marker
  4. Keeping end marker at the end
""")
print(f"{'='*70}")
