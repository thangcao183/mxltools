#!/usr/bin/env python3
"""Test C++ encoding: LSB-first with APPEND (không phải prepend)"""

def byteToBinary_LSB_APPEND(byte):
    """C++ byteToBinary logic: LSB-first"""
    result = ""
    for i in range(8):  # i = 0,1,2,...,7
        result += '1' if (byte >> i) & 1 else '0'
    return result

def binaryToByte_LSB_APPEND(binary):
    """C++ binaryToByte logic: LSB-first"""
    result = 0
    for i in range(min(8, len(binary))):
        if binary[i] == '1':
            result |= (1 << i)
    return result

def create_bit_string_APPEND(bytes_arr):
    """C++ createBitString: APPEND (không reverse!)"""
    bitstring = ""
    for byte in bytes_arr[2:]:  # Skip JM
        bitstring += byteToBinary_LSB_APPEND(byte)  # APPEND!
    return bitstring

def bit_string_to_bytes_APPEND(bitstring):
    """C++ bitStringToBytes: APPEND (không reverse!)"""
    bytes_arr = [ord('J'), ord('M')]
    
    # Convert theo chunks 8 bits
    for i in range(0, len(bitstring), 8):
        chunk = bitstring[i:i+8]
        if len(chunk) < 8:
            chunk += '0' * (8 - len(chunk))  # Pad
        byte_val = binaryToByte_LSB_APPEND(chunk)
        bytes_arr.append(byte_val)  # APPEND!
    
    return bytes(bytes_arr)

# Test with file twi_1240052515.d2i
import sys
if len(sys.argv) > 1:
    with open(sys.argv[1], 'rb') as f:
        original_bytes = f.read()
    
    print(f"File: {sys.argv[1]}")
    print(f"Size: {len(original_bytes)} bytes")
    print(f"First 10 bytes: {' '.join(f'{b:02x}' for b in original_bytes[:10])}")
    
    # Create bitstring
    bitstring = create_bit_string_APPEND(original_bytes)
    print(f"\nBitstring length: {len(bitstring)} bits")
    print(f"First 50 bits: {bitstring[:50]}")
    print(f"Last 50 bits: {bitstring[-50:]}")
    
    # Convert back
    converted_bytes = bit_string_to_bytes_APPEND(bitstring)
    print(f"\nConverted back: {len(converted_bytes)} bytes")
    print(f"First 10 bytes: {' '.join(f'{b:02x}' for b in converted_bytes[:10])}")
    
    # Compare
    if original_bytes == converted_bytes:
        print("\n✅ PERFECT MATCH! Encoding is correct!")
    else:
        print("\n❌ MISMATCH!")
        print(f"Original byte 2: {original_bytes[2]:02x} = {original_bytes[2]:08b}")
        print(f"Converted byte 2: {converted_bytes[2]:02x} = {converted_bytes[2]:08b}")
        
        # Show LSB encoding
        print(f"\nLSB encoding of byte 2 (0x{original_bytes[2]:02x}):")
        lsb_bits = byteToBinary_LSB_APPEND(original_bytes[2])
        print(f"  Bits: {lsb_bits}")
        print(f"  Bit 0 (LSB): {lsb_bits[0]}")
        print(f"  Bit 7 (MSB): {lsb_bits[7]}")
else:
    # Test basic conversion
    test_byte = 0x10  # Byte 2 from original file
    print(f"Test byte: 0x{test_byte:02x} = {test_byte:08b} (MSB-first)")
    
    lsb_bits = byteToBinary_LSB_APPEND(test_byte)
    print(f"LSB-first: {lsb_bits}")
    print(f"  Bit 0: {lsb_bits[0]}")
    print(f"  Bit 7: {lsb_bits[7]}")
    
    converted_back = binaryToByte_LSB_APPEND(lsb_bits)
    print(f"Converted back: 0x{converted_back:02x}")
    print(f"Match: {converted_back == test_byte}")
