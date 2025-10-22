#!/usr/bin/env python3
"""Diagnose rich_extended.d2i issues"""

def bytes_to_bits(data):
    return ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))

def read_d2i(filename):
    with open(filename, 'rb') as f:
        header = f.read(2)
        if header != b'JM':
            print(f"ERROR: {filename} has invalid header")
            return None, None
        data = f.read()
    return data, bytes_to_bits(data)

print("="*70)
print("COMPARING FILES")
print("="*70)

# Read all three files
rich_data, rich_bits = read_d2i('d2i/rich.d2i')
twi_data, twi_bits = read_d2i('d2i/twi_1240052515.d2i')
extended_data, extended_bits = read_d2i('d2i/rich_extended.d2i')

if not all([rich_bits, twi_bits, extended_bits]):
    print("ERROR: Could not read files")
    exit(1)

print(f"\n{'Filename':<25} {'Size':<10} {'Total Bits':<12}")
print("-"*70)
print(f"{'rich.d2i':<25} {len(rich_data):<10} {len(rich_bits):<12}")
print(f"{'twi_1240052515.d2i':<25} {len(twi_data):<10} {len(twi_bits):<12}")
print(f"{'rich_extended.d2i':<25} {len(extended_data):<10} {len(extended_bits):<12}")

print("\n" + "="*70)
print("RICH.D2I (Original non-extended)")
print("="*70)
print(f"Bit 21 (isExtended): {rich_bits[21]} (1=not extended)")
item_type = ''.join(chr(int(rich_bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
print(f"Item type: '{item_type}'")
print(f"Hex: {rich_data.hex()}")

print("\n" + "="*70)
print("TWI_1240052515.D2I (Working extended unique)")
print("="*70)
print(f"Bit 21 (isExtended): {twi_bits[21]} (0=extended)")
item_type = ''.join(chr(int(twi_bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
print(f"Item type: '{item_type}'")
print(f"Quality (bit 134-137): {int(twi_bits[134:138], 2)} (7=unique)")
print(f"Unique ID (bit 140-151): {int(twi_bits[140:152], 2)}")
print(f"First prop ID (bit 152-160): {int(twi_bits[152:161], 2)}")

print("\n" + "="*70)
print("RICH_EXTENDED.D2I (Our created file)")
print("="*70)
print(f"Bit 21 (isExtended): {extended_bits[21]} (should be 0)")
item_type = ''.join(chr(int(extended_bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
print(f"Item type: '{item_type}'")
print(f"Quality (bit 134-137): {int(extended_bits[134:138], 2)}")
print(f"Unique ID (bit 140-151): {int(extended_bits[140:152], 2)}")
print(f"First prop ID (bit 152-160): {int(extended_bits[152:161], 2)}")
print(f"Hex: {extended_data.hex()}")

print("\n" + "="*70)
print("BIT-BY-BIT COMPARISON")
print("="*70)

# Compare header (0-59) - should be from rich
print("\nHeader (bits 0-59):")
header_match = rich_bits[0:60] == extended_bits[0:60]
print(f"  Rich == Extended: {header_match}")
if not header_match:
    for i in range(60):
        if rich_bits[i] != extended_bits[i]:
            print(f"  Bit {i}: rich={rich_bits[i]}, extended={extended_bits[i]}")

# Compare item type (60-91) - should be from rich
print("\nItem Type (bits 60-91):")
type_match = rich_bits[60:92] == extended_bits[60:92]
print(f"  Rich == Extended: {type_match}")
if not type_match:
    print(f"  Rich: {rich_bits[60:92]}")
    print(f"  Extended: {extended_bits[60:92]}")

# Compare extended onwards (92+) - should be from twi
print("\nExtended+Properties (bits 92+):")
ext_match = twi_bits[92:] == extended_bits[92:]
print(f"  Twi == Extended: {ext_match}")
if not ext_match:
    print(f"  Twi length: {len(twi_bits[92:])}")
    print(f"  Extended length: {len(extended_bits[92:])}")
    # Find first difference
    for i in range(min(len(twi_bits[92:]), len(extended_bits[92:]))):
        if twi_bits[92+i] != extended_bits[92+i]:
            print(f"  First diff at bit {92+i}: twi={twi_bits[92+i]}, extended={extended_bits[92+i]}")
            break

print("\n" + "="*70)
print("POTENTIAL ISSUES")
print("="*70)

issues = []

# Check isExtended bit
if extended_bits[21] != '0':
    issues.append(f"❌ Bit 21 (isExtended) = {extended_bits[21]}, should be 0")
else:
    print("✓ Bit 21 (isExtended) is correct: 0")

# Check if we kept rich's non-extended header
if rich_bits[21] == '1' and extended_bits[21] == '1':
    issues.append("❌ We copied rich's isExtended=1 (not extended) but added extended fields!")
    issues.append("   Solution: Need to flip bit 21 from 1 to 0")

if issues:
    print("\nFOUND ISSUES:")
    for issue in issues:
        print(issue)
else:
    print("\n✓ No obvious issues found")

print("\n" + "="*70)
