#!/usr/bin/env python3
"""
Debug: So sánh bitString được tạo từ file gốc vs file mới
"""

def bytes_to_bits_no_prepend(data):
    """Normal conversion: byte 0 → bits 0-7"""
    bits = ""
    for byte in data:
        for i in range(8):
            bits += '1' if (byte & (1 << i)) else '0'
    return bits

def bytes_to_bits_prepend(data):
    """With PREPEND: byte 0 → bits at end"""
    bits = ""
    for byte in data:
        byte_bits = ""
        for i in range(8):
            byte_bits += '1' if (byte & (1 << i)) else '0'
        bits = byte_bits + bits
    return bits

print("="*70)
print("COMPARING BITSTRING GENERATION METHODS")
print("="*70)

# Original file
with open('d2i/rich.d2i', 'rb') as f:
    orig_data = f.read()

print("\n1. ORIGINAL FILE (rich.d2i)")
print("-"*70)
print(f"Bytes: {orig_data[2:].hex()}")

orig_bits_no_prepend = bytes_to_bits_no_prepend(orig_data[2:])
orig_bits_prepend = bytes_to_bits_prepend(orig_data[2:])

print(f"\nBitString (NO prepend):")
print(f"  Length: {len(orig_bits_no_prepend)} bits")
print(f"  First 96 bits: {orig_bits_no_prepend[:96]}")

print(f"\nBitString (WITH prepend):")
print(f"  Length: {len(orig_bits_prepend)} bits")
print(f"  First 96 bits: {orig_bits_prepend[:96]}")

print(f"\nAre they the same? {orig_bits_no_prepend == orig_bits_prepend}")

# New file
with open('d2i/rich_prepend.d2i', 'rb') as f:
    new_data = f.read()

print("\n2. NEW FILE (rich_prepend.d2i)")
print("-"*70)
print(f"Bytes: {new_data[2:].hex()}")

new_bits_no_prepend = bytes_to_bits_no_prepend(new_data[2:])
new_bits_prepend = bytes_to_bits_prepend(new_data[2:])

print(f"\nBitString (NO prepend):")
print(f"  Length: {len(new_bits_no_prepend)} bits")
print(f"  First 100 bits: {new_bits_no_prepend[:100]}")

print(f"\nBitString (WITH prepend):")
print(f"  Length: {len(new_bits_prepend)} bits")
print(f"  First 100 bits: {new_bits_prepend[:100]}")

# Check which method gives correct structure
print("\n" + "="*70)
print("CHECKING WHICH METHOD IS CORRECT")
print("="*70)

def check_structure(bits, label):
    print(f"\n{label}:")
    # Check isExtended bit at position 21
    is_extended_bit = bits[21]
    is_extended = (is_extended_bit == '0')
    print(f"  Bit 21 (isExtended): {is_extended_bit} → isExtended = {is_extended}")
    
    # Check item type at bits 60-91
    item_type = ""
    for i in range(4):
        char_bits = bits[60 + i*8:60 + (i+1)*8]
        val = 0
        for j, bit in enumerate(char_bits):
            if bit == '1':
                val |= (1 << j)
        if val != 32 and val != 0:
            item_type += chr(val) if 32 <= val < 127 else f'[{val}]'
    print(f"  Item type (bits 60-91): '{item_type}'")
    
    return is_extended, item_type

# Test original file
check_structure(orig_bits_no_prepend, "Original (NO prepend)")
check_structure(orig_bits_prepend, "Original (WITH prepend)")

# Test new file  
check_structure(new_bits_no_prepend, "New (NO prepend)")
check_structure(new_bits_prepend, "New (WITH prepend)")

print("\n" + "="*70)
print("CONCLUSION")
print("="*70)
print("""
ItemParser uses PREPEND when READING files (line 80):
  itemBitData.prepend(binaryStringFromNumber(aByte));

So when reading file bytes [0, 1, 2, ...], it creates bitString:
  [...bits from byte 2...][...bits from byte 1...][...bits from byte 0...]

When WRITING files (line 510), it reverses this:
  itemBytes.prepend(item->bitString.mid(i, 8).toShort(0, 2));

So bitString [0-7] becomes last byte, [8-15] becomes second-to-last, etc.

Our tool should:
1. Read file with NO prepend (straight bytes → bits)
2. Modify bitString
3. Write file with PREPEND (to reverse the bitString)

But this creates bytes in prepended order!
ItemParser will then PREPEND again when reading, reversing it back!
""")
