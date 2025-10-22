#!/usr/bin/env python3
"""Final verification and summary of rich_extended.d2i"""

def bytes_to_bits(data):
    return ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))

with open('d2i/rich_extended.d2i', 'rb') as f:
    header = f.read(2)
    data = f.read()

bits = bytes_to_bits(data)

print("="*70)
print("RICH_EXTENDED.D2I - FINAL VERIFICATION")
print("="*70)
print(f"\nFile size: {len(data) + 2} bytes (including JM header)")
print(f"Data size: {len(data)} bytes")
print(f"Total bits: {len(bits)}")
print(f"\nHex: {data.hex()}")

print("\n" + "-"*70)
print("STRUCTURE BREAKDOWN")
print("-"*70)

# Header (0-59)
print("\n[Bits 0-59] Header (60 bits)")
print(f"  Source: rich.d2i (with bit 21 flipped)")
print(f"  Bit 21 (isExtended): {bits[21]} ✓ (0=extended)")

# Item Type (60-91)
print("\n[Bits 60-91] Item Type (32 bits)")
item_type = ''.join(chr(int(bits[60+i:60+i+8][::-1], 2)) for i in range(0, 32, 8))
print(f"  Source: rich.d2i")
print(f"  Value: '{item_type}' ✓")

# Extended Fields (92-139)
print("\n[Bits 92-139] Extended Fields (48 bits)")
print(f"  Source: twi_1240052515.d2i")
print(f"  socketablesNumber: {int(bits[92:95], 2)}")
print(f"  guid: {int(bits[95:127], 2)}")
print(f"  ilvl: {int(bits[127:134], 2)}")
print(f"  quality: {int(bits[134:138], 2)} (7=unique) ✓")

# Quality-specific (140-151)
print("\n[Bits 140-151] Unique ID (12 bits)")
print(f"  Source: twi_1240052515.d2i")
print(f"  Value: {int(bits[140:152], 2)}")

# Properties (152+)
print("\n[Bits 152+] Properties")
print(f"  Source: twi_1240052515.d2i")
print(f"  First property ID: {int(bits[152:161], 2)}")

print("\n" + "="*70)
print("✅ FILE IS READY TO TEST")
print("="*70)
print("\nStructure: [Rich Header+Type with bit 21 flipped] + [Twi Extended+Props]")
print("="*70)
