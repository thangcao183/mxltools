#!/usr/bin/env python3
"""Test prepend logic"""

# Simulate exact ItemParser logic
bytes_orig = bytes.fromhex('10 00 a0 00 65 00 0e 52 07 b4 02 02')
print('Original bytes (after JM):', ' '.join(f'{b:02x}' for b in bytes_orig))

# Create bitString using prepend (ItemParser style)
bitString = ''
for byte in bytes_orig:
    byte_bits = format(byte, '08b')  # MSB-first
    bitString = byte_bits + bitString  # prepend
    print(f'  Byte {byte:02x} -> {byte_bits}, bitString length: {len(bitString)}')

print(f'\nFinal bitString length: {len(bitString)}')
print(f'First 50 bits: {bitString[:50]}')
print(f'Last 50 bits: {bitString[-50:]}')

# Now reverse: convert bitString back to bytes
print('\n--- Converting back to bytes (using prepend) ---')
result_bytes = []
for i in range(0, len(bitString), 8):
    chunk = bitString[i:i+8]
    byte_val = int(chunk, 2)
    result_bytes.insert(0, byte_val)  # prepend
    print(f'  Chunk[{i:3d}:{i+8:3d}] = {chunk} -> byte {byte_val:02x}, prepend to position 0')

print(f'\nResult bytes: {" ".join(f"{b:02x}" for b in result_bytes)}')
print(f'Original:     {" ".join(f"{b:02x}" for b in bytes_orig)}')
print(f'Match: {result_bytes == list(bytes_orig)}')
