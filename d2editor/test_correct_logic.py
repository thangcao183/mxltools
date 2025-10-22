#!/usr/bin/env python3
"""
Recreate the CORRECT expected output using Python
"""

def bytes_to_bitstring(data):
    """Convert bytes to bitstring using prepend logic"""
    chunks = [format(b, '08b') for b in data]
    return ''.join(reversed(chunks))

def bitstring_to_bytes(bitstring):
    """Convert bitstring to bytes using prepend logic"""
    # Pad at end
    extra = len(bitstring) % 8
    padded = bitstring + '0' * (8 - extra) if extra else bitstring
    
    # Split and convert
    chunks = [padded[i:i+8] for i in range(0, len(padded), 8)]
    vals = [int(ch, 2) for ch in chunks]
    
    # Reverse (prepend)
    vals.reverse()
    
    return bytes([0x4a, 0x4d] + vals)

# Read original rich.d2i
with open('d2i/rich.d2i', 'rb') as f:
    orig_data = f.read()

orig_bitstring = bytes_to_bitstring(orig_data[2:])
print(f"Original bitString: {len(orig_bitstring)} bits")
print(f"Original: {orig_bitstring}")
print()

# Analyze structure
print("Original structure:")
print(f"  Bits 0-3 (padding): {orig_bitstring[0:4]}")
print(f"  Bits 4-35 (item type): {orig_bitstring[4:36]}")
print(f"  Bits 36-95 (header): {orig_bitstring[36:96]}")
print()

# Create property bits (MSB-first like ItemParser)
prop_id = 79
value = 50
add = 100
raw_value = value + add  # 150

prop_id_bits = format(prop_id, '09b')  # MSB
value_bits = format(raw_value, '09b')   # MSB
end_marker_bits = format(511, '09b')    # MSB

property_bits = prop_id_bits + value_bits + end_marker_bits

print(f"Property bits (MSB-first):")
print(f"  ID 79: {prop_id_bits}")
print(f"  Value 150: {value_bits}")
print(f"  End marker 511: {end_marker_bits}")
print(f"  Combined: {property_bits} ({len(property_bits)} bits)")
print()

# Insert AFTER item type, BEFORE header
# Position 36 in bitString
insert_pos = 36

before = orig_bitstring[:insert_pos]
after = orig_bitstring[insert_pos:]

new_bitstring = before + property_bits + after

print(f"New bitString structure:")
print(f"  Before (padding + item type): {len(before)} bits")
print(f"  Insert (properties): {len(property_bits)} bits")
print(f"  After (header): {len(after)} bits")
print(f"  Total: {len(new_bitstring)} bits")
print()

# Convert to bytes
new_bytes = bitstring_to_bytes(new_bitstring)

print(f"Result bytes ({len(new_bytes)}):")
print(f"  Hex: {new_bytes.hex()}")
print()

# Compare with C++ output
with open('d2i/rich.d2i.added_v4', 'rb') as f:
    cpp_bytes = f.read()

print(f"C++ output ({len(cpp_bytes)}):")
print(f"  Hex: {cpp_bytes.hex()}")
print()

print(f"Match: {new_bytes == cpp_bytes}")

if new_bytes != cpp_bytes:
    print("\nDifferences:")
    for i in range(max(len(new_bytes), len(cpp_bytes))):
        py = f'{new_bytes[i]:02x}' if i < len(new_bytes) else '--'
        cpp = f'{cpp_bytes[i]:02x}' if i < len(cpp_bytes) else '--'
        if py != cpp:
            print(f"  Byte {i}: Python={py}, C++={cpp}")
