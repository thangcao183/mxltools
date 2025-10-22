#!/usr/bin/env python3
"""Analyze which extended fields to copy vs keep from rich"""

def bytes_to_bits(data):
    return ''.join('1' if (b & (1<<i)) else '0' for b in data for i in range(8))

def read_d2i(filename):
    with open(filename, 'rb') as f:
        header = f.read(2)
        data = f.read()
    return bytes_to_bits(data)

rich_bits = read_d2i('d2i/rich.d2i')
twi_bits = read_d2i('d2i/twi_1240052515.d2i')

print("="*70)
print("EXTENDED FIELDS BREAKDOWN")
print("="*70)

print("\nAccording to ItemParser.cpp:")
print("  Bit 92-94:   socketablesNumber (3 bits)")
print("  Bit 95-126:  guid (32 bits) - determines UNIQUE NAME")
print("  Bit 127-133: ilvl (7 bits)")
print("  Bit 134-137: quality (4 bits)")
print("  Bit 138:     hasMultiGraphics (1 bit)")
print("  Bit 139:     autoPrefix (1 bit)")
print("  ---")
print("  Bit 140-151: uniqueId (12 bits) - for quality=7 (unique)")

print("\n" + "="*70)
print("RICH.D2I (non-extended)")
print("="*70)
print(f"Total bits: {len(rich_bits)}")
print(f"isExtended: {rich_bits[21]} (1 = not extended)")
print("Extended fields: NOT PRESENT (only 96 bits total)")

print("\n" + "="*70)
print("TWI_1240052515.D2I (extended unique)")
print("="*70)
print(f"Total bits: {len(twi_bits)}")
print(f"isExtended: {twi_bits[21]} (0 = extended)")
print(f"\nExtended fields:")
print(f"  socketablesNumber (92-94):   {twi_bits[92:95]} = {int(twi_bits[92:95], 2)}")
print(f"  guid (95-126):               {twi_bits[95:127]} = {int(twi_bits[95:127], 2)}")
print(f"  ilvl (127-133):              {twi_bits[127:134]} = {int(twi_bits[127:134], 2)}")
print(f"  quality (134-137):           {twi_bits[134:138]} = {int(twi_bits[134:138], 2)} (14 LSB = 7 MSB)")
print(f"  hasMultiGraphics (138):      {twi_bits[138]}")
print(f"  autoPrefix (139):            {twi_bits[139]}")
print(f"\nUnique ID (140-151):           {twi_bits[140:152]} = {int(twi_bits[140:152], 2)}")

print("\n" + "="*70)
print("RECOMMENDATION")
print("="*70)
print("""
For creating a charm with properties but NO UNIQUE NAME:

KEEP FROM RICH:
  - Header (bits 0-59) but flip bit 21 to 0
  - Item type (bits 60-91) 'u@+ '

CREATE NEW:
  - socketablesNumber = 0 (3 bits)
  - guid = 0 or random (32 bits) - DON'T copy from twi!
  - ilvl = 99 (7 bits)
  - quality = 4 (magic) NOT 7 (unique) - so no unique name!
  - hasMultiGraphics = 0 (1 bit)
  - autoPrefix = 0 (1 bit)
  
  For magic items (quality=4):
  - magicPrefix = 0 (11 bits)
  - magicSuffix = 0 (11 bits)

COPY FROM TWI:
  - Properties only (from bit 152+)

This way:
  ✓ Item is extended (has properties)
  ✓ Item type is 'u@+ ' (charm)
  ✓ Quality is MAGIC (4), not UNIQUE (7)
  ✓ Properties are copied from twi
  ✗ No unique name (because quality=magic, not unique)
""")

print("="*70)
print("ALTERNATIVE: Use quality=7 but guid=0 and uniqueId=0")
print("="*70)
print("""
If you want UNIQUE quality but without a specific unique name:

CREATE NEW:
  - socketablesNumber = 0
  - guid = 0 (32 bits)
  - ilvl = 99
  - quality = 7 (unique)
  - hasMultiGraphics = 0
  - autoPrefix = 0
  - uniqueId = 0 (12 bits) - might show generic unique name

COPY FROM TWI:
  - Properties only
""")
print("="*70)
