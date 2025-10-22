#!/usr/bin/env python3

def bytes_to_bitstring(data):
    """Convert bytes to bitstring using prepend logic (like ItemParser)"""
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

def bitstring_to_bytes(bitstring):
    """Convert bitstring back to bytes using reverse prepend"""
    # Pad to byte boundary
    padded = bitstring + '0' * ((8 - len(bitstring) % 8) % 8)
    chunks = [bitstring[i:i+8] for i in range(0, len(bitstring), 8)]
    vals = [int(ch, 2) for ch in chunks]
    vals.reverse()
    return bytes(vals)

# Original rich.d2i (without JM)
original_bytes = bytes([0x10, 0x00, 0xa0, 0x00, 0x65, 0x00, 0x0e, 0x52, 0x07, 0xb4, 0x02, 0x02])
original_bits = bytes_to_bitstring(original_bytes)

print("Original bitString (96 bits):")
print(original_bits)
print()

# Property bits to insert (MSB-first, verified correct)
property_bits = "001001111010010110111111111"  # 27 bits

# Insert at position 92 from END (which is position 4 from START of 96-bit string)
insertPos = len(original_bits) - 92  # = 96 - 92 = 4

print(f"Inserting {len(property_bits)} bits at position {insertPos} (logical offset 92 from end)")
print(f"Property bits: {property_bits}")
print()

# Split and reconstruct
before = original_bits[:insertPos]
after = original_bits[insertPos:]

print(f"Before (bits 0-{insertPos-1}): {before} ({len(before)} bits)")
print(f"Insert: {property_bits} ({len(property_bits)} bits)")
print(f"After (bits {insertPos}-end): {after} ({len(after)} bits)")
print()

new_bitstring = before + property_bits + after
print(f"New bitString: {len(new_bitstring)} bits")
print(new_bitstring)
print()

# Byte align
padded = new_bitstring + '0' * ((8 - len(new_bitstring) % 8) % 8)
print(f"After byte align: {len(padded)} bits")
print(padded)
print()

# Convert to bytes
new_bytes = bitstring_to_bytes(new_bitstring)
print(f"New bytes ({len(new_bytes)}): {' '.join(format(b, '02x') for b in new_bytes)}")
print()

# Compare with actual output from v3
v3_bytes = bytes([0x00, 0x02, 0x00, 0x14, 0xa0, 0x0c, 0xc0, 0x41, 0xea, 0x80, 0x56, 0x40, 0xfe, 0x5b, 0x7a, 0x02])
print(f"v3 output bytes ({len(v3_bytes)}): {' '.join(format(b, '02x') for b in v3_bytes)}")
print(f"Match: {new_bytes == v3_bytes}")

if new_bytes != v3_bytes:
    print("\n❌ MISMATCH! Comparing byte-by-byte:")
    for i in range(max(len(new_bytes), len(v3_bytes))):
        exp = format(new_bytes[i], '02x') if i < len(new_bytes) else '--'
        got = format(v3_bytes[i], '02x') if i < len(v3_bytes) else '--'
        match = '✓' if exp == got else '✗'
        print(f"  Byte {i}: expected={exp}, got={got} {match}")
