#!/usr/bin/env python3
"""Test the exact bitString from C++ output"""

def bitstring_to_bytes(bitstring):
    """Convert bitstring back to bytes using reverse prepend (like C++ bitStringToBytes)"""
    # Pad to byte boundary at END
    padded = bitstring
    extra = len(bitstring) % 8
    if extra != 0:
        padded += '0' * (8 - extra)
    
    print(f"Input bitstring: {len(bitstring)} bits")
    print(f"Padded bitstring: {len(padded)} bits")
    print(f"Padded: {padded}")
    print()
    
    # Process left to right, prepending each byte
    chunks = [padded[i:i+8] for i in range(0, len(padded), 8)]
    print(f"Chunks ({len(chunks)}): {chunks}")
    
    vals = [int(ch, 2) for ch in chunks]
    print(f"Values: {[hex(v) for v in vals]}")
    
    # Prepend (reverse)
    vals.reverse()
    print(f"After reverse: {[hex(v) for v in vals]}")
    
    return bytes([0x4a, 0x4d] + vals)

# Exact bitString from C++ after insert (before byte align)
cpp_bitstring = "00000010011110100101101111111110010000000101011010000000111010100100000111000000000011001010000000010100000000000000000010000"

print("=" * 80)
print("Testing C++ bitString")
print("=" * 80)

result = bitstring_to_bytes(cpp_bitstring)
print(f"\nResult ({len(result)} bytes): {result.hex()}")

expected = bytes.fromhex("4a4d00020014a00cc041ea805640fe5b7a02")
print(f"Expected ({len(expected)} bytes): {expected.hex()}")
print(f"Match: {result == expected}")

if result != expected:
    print("\n❌ Byte-by-byte comparison:")
    for i in range(max(len(result), len(expected))):
        r = f"{result[i]:02x}" if i < len(result) else '--'
        e = f"{expected[i]:02x}" if i < len(expected) else '--'
        match = '✓' if r == e else '✗'
        print(f"  Byte {i}: result={r}, expected={e} {match}")
