#!/usr/bin/env python3
"""Check original file parsing vs actual bits"""

with open('d2i/complete/relic_fungus.d2i', 'rb') as f:
    data = f.read()[2:]
    
# Convert to bitstring using PREPEND
bitstring = ''
for byte in data:
    bitstring = format(byte, '08b') + bitstring

print(f'Bitstring length: {len(bitstring)}')
print()

# Parser says: "Properties should start at bit offset: 159 (absolute pos from end: 129)"
# "Property 42 ... @ bit 129"

# "bit offset 159" means the BitReader (reading from END) is at position 159 from END
# That's position 288-159 = 129 from START

parser_start_pos = 288 - 159  # = 129
print(f'Parser thinks properties start at: bit {parser_start_pos} from START')
print()

# But we know property 42 is at bit 114 from START
actual_start_pos = 114
print(f'Property 42 actually at: bit {actual_start_pos} from START')
print()

# So parser is reading from position 129, not 114
# What does parser READ at position 129?
print(f'Bits at parser position {parser_start_pos} (16 bits):')
parser_bits = bitstring[parser_start_pos:parser_start_pos+16]
print(f'  {parser_bits}')

# Decode as LSB
def lsb_to_number(bits):
    return int(bits[::-1], 2)

prop_id_parser = lsb_to_number(parser_bits[:9])
print(f'  Property ID: {prop_id_parser}')
print()

# What's at the ACTUAL position?
print(f'Bits at actual position {actual_start_pos} (16 bits):')
actual_bits = bitstring[actual_start_pos:actual_start_pos+16]
print(f'  {actual_bits}')
prop_id_actual = lsb_to_number(actual_bits[:9])
print(f'  Property ID: {prop_id_actual}')
print()

# Wait - maybe the parser reads BACKWARDS?
# If BitReader reads from END backwards, when it's at "position 159 from END"
# and reads a property, where does it actually read from?

print('='*70)
print('UNDERSTANDING BitReader BACKWARDS READING')
print('='*70)
print()
print('BitReader starts at END (position 0 from END = position 288 from START)')
print('It reads backwards, consuming bits')
print()
print('After reading header, BitReader is at "bit offset 159" from END')
print('This means it has consumed 288-159 = 129 bits')
print('Current position (from START): 288 - 159 = 129')
print()
print('Now it reads property 42 (16 bits) BACKWARDS:')
print('  Reads bits from position 129 backwards to position 129-16 = 113')
print('  So it reads bits 113-129 (reading backwards)')
print()
print('Bits 113-129 (what BitReader would read):')
backward_bits = bitstring[113:129]
print(f'  {backward_bits}')
print(f'  Length: {len(backward_bits)}')
print()

# But wait - the property is at 114-130, not 113-129!
# There's a 1-bit offset!

print('Actual property 42 location: bits 114-130')
print(f'  {bitstring[114:130]}')
