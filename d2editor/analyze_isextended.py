#!/usr/bin/env python3
"""
Analyze isExtended bit position in rich.d2i
"""

def analyze_d2i_structure(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"File: {filename}")
    print(f"Size: {len(data)} bytes")
    print(f"Hex: {data.hex()}")
    
    # Skip JM header
    item_data = data[2:]
    
    # Convert to bits (ReverseBitReader reads from END, so we need to understand the structure)
    # But for finding isExtended, let's look at the ACTUAL byte order in file
    
    print("\n" + "="*60)
    print("Byte-by-byte analysis (MSB-first within byte):")
    print("="*60)
    
    for i, byte in enumerate(item_data):
        bits_msb = format(byte, '08b')
        print(f"Byte {i}: 0x{byte:02x} = {bits_msb} (MSB-first)")
    
    print("\n" + "="*60)
    print("Looking for isExtended bit (according to ItemParser.cpp line 98):")
    print("="*60)
    
    # ItemParser reads in this order:
    # skip() - duped
    # readBool() - isSocketed
    # skip(2)
    # skip(2) - illegal equip + unk
    # readBool() - isEar
    # readBool() - isStarter
    # skip(2)
    # skip()
    # readBool() - THIS IS THE BIT FOR isExtended! (inverted)
    
    # That's 1 + 1 + 2 + 2 + 1 + 1 + 2 + 1 = 11 bits from the start
    # Bit position 11 (0-indexed) is the isExtended bit
    
    # But wait - ReverseBitReader reads from END!
    # So we need to convert from END position
    
    total_bits = len(item_data) * 8
    print(f"\nTotal bits: {total_bits}")
    
    # Convert to bitString (prepend logic - last byte first)
    bitString = ""
    for byte in reversed(item_data):
        for i in range(7, -1, -1):
            bitString += '1' if (byte & (1 << i)) else '0'
    
    print(f"BitString (after prepend): {bitString}")
    print(f"BitString length: {len(bitString)}")
    
    # ReverseBitReader starts from END of bitString
    # Position 0 in ReverseBitReader = bit at (length - 1)
    # After reading 11 bits, we're at position 11
    # The bit at position 11 from END is the isExtended bit
    
    isExtended_pos_from_end = 11
    isExtended_pos_in_bitString = len(bitString) - 1 - isExtended_pos_from_end
    
    print(f"\nisExtended bit is at position {isExtended_pos_from_end} from END")
    print(f"Which is position {isExtended_pos_in_bitString} in bitString (0-indexed)")
    print(f"isExtended bit value: {bitString[isExtended_pos_in_bitString]}")
    print(f"isExtended = !bit = {not bool(int(bitString[isExtended_pos_in_bitString]))}")
    
    # Show context around this bit
    start = max(0, isExtended_pos_in_bitString - 20)
    end = min(len(bitString), isExtended_pos_in_bitString + 20)
    context = bitString[start:end]
    marker_pos = isExtended_pos_in_bitString - start
    
    print(f"\nContext (±20 bits):")
    print(f"  {context}")
    print(f"  {' ' * marker_pos}^ isExtended bit")

# Analyze both files
print("ORIGINAL FILE:")
print("="*60)
analyze_d2i_structure('d2i/rich.d2i')

print("\n\n")
print("FILE WITH PROPERTY ADDED:")
print("="*60)
analyze_d2i_structure('d2i/rich.d2i.correct_order')
