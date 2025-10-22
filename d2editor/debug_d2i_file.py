#!/usr/bin/env python3
"""
Debug tool to analyze .d2i file structure and compare original vs modified files.
This helps identify corruption issues when adding properties.
"""

import sys
import struct

def byte_to_binary_msb(byte):
    """Convert byte to MSB-first binary string (standard representation)"""
    return format(byte, '08b')

def byte_to_binary_lsb(byte):
    """Convert byte to LSB-first binary string"""
    return format(byte, '08b')[::-1]

def read_d2i_file(filepath):
    """Read .d2i file and return bytes"""
    try:
        with open(filepath, 'rb') as f:
            data = f.read()
        return data
    except FileNotFoundError:
        print(f"❌ File not found: {filepath}")
        return None

def create_bitstring_prepend_msb(data):
    """Create bitstring using prepend logic with MSB-first bytes (like ItemParser)"""
    bitstring = ""
    # Skip 'JM' header
    for i in range(2, len(data)):
        byte_bits = byte_to_binary_msb(data[i])
        bitstring = byte_bits + bitstring  # prepend
    return bitstring

def parse_item_header(bitstring):
    """Parse item header and return info dict"""
    info = {}
    pos = len(bitstring)  # Start from end (like ReverseBitReader)
    
    def read_bits(num_bits):
        nonlocal pos
        if pos - num_bits < 0:
            raise ValueError(f"Not enough bits to read {num_bits} bits at pos {pos}")
        pos -= num_bits
        chunk = bitstring[pos:pos+num_bits]
        # Convert MSB-first binary string to int
        return int(chunk, 2), chunk
    
    def skip_bits(num_bits):
        nonlocal pos
        pos -= num_bits
    
    try:
        # Parse header fields (matching ItemParser.cpp)
        info['isQuest'], _ = read_bits(1)
        skip_bits(3)
        info['isIdentified'], _ = read_bits(1)
        skip_bits(5)
        skip_bits(1)  # is duped
        info['isSocketed'], _ = read_bits(1)
        skip_bits(2)
        skip_bits(2)  # is illegal equip + unk
        info['isEar'], _ = read_bits(1)
        info['isStarter'], _ = read_bits(1)
        skip_bits(2)
        skip_bits(1)
        is_simple_val, _ = read_bits(1)
        info['isExtended'] = not is_simple_val
        info['isEthereal'], _ = read_bits(1)
        skip_bits(1)
        info['isPersonalized'], _ = read_bits(1)
        skip_bits(1)
        info['isRW'], _ = read_bits(1)
        skip_bits(5)
        skip_bits(8)  # version
        skip_bits(2)
        info['location'], _ = read_bits(3)
        info['whereEquipped'], _ = read_bits(4)
        info['column'], _ = read_bits(4)
        info['row'], _ = read_bits(4)
        info['storage'], _ = read_bits(3)
        
        if info['isEar']:
            info['itemType'] = 'EAR'
            return info, pos
        
        # Read item type (4 bytes, 8 bits each)
        item_type_chars = []
        for i in range(4):
            char_val, _ = read_bits(8)
            if char_val != 0:
                item_type_chars.append(chr(char_val))
        info['itemType'] = ''.join(item_type_chars)
        
        if not info['isExtended']:
            info['quality'] = 1  # Normal
            info['properties_start_pos'] = len(bitstring) - pos
            return info, pos
        
        # Extended item
        skip_bits(3)  # socketablesNumber
        skip_bits(32)  # guid
        skip_bits(7)  # ilvl
        info['quality'], _ = read_bits(4)
        
        has_var_gfx, _ = read_bits(1)
        if has_var_gfx:
            skip_bits(3)
        
        has_auto_prefix, _ = read_bits(1)
        if has_auto_prefix:
            skip_bits(11)
        
        # Quality-specific fields
        quality = info['quality']
        if quality in [2, 3]:  # Low/High quality
            skip_bits(3)
        elif quality == 4:  # Magic
            skip_bits(22)
        elif quality in [5, 7]:  # Set/Unique
            skip_bits(15)
        elif quality in [6, 8]:  # Rare/Crafted
            skip_bits(16)
            for i in range(6):
                has_affix, _ = read_bits(1)
                if has_affix:
                    skip_bits(11)
        elif quality == 9:  # Honorific
            skip_bits(16)
        
        if info['isRW']:
            skip_bits(16)
        
        if info['isPersonalized']:
            for i in range(16):
                char_val, _ = read_bits(7)
                if char_val == 0:
                    break
        
        if info['isSocketed']:
            skip_bits(4)
        
        if quality == 5:  # Set
            for i in range(5):
                read_bits(1)
        
        info['properties_start_pos'] = len(bitstring) - pos
        return info, pos
        
    except Exception as e:
        print(f"❌ Error parsing header: {e}")
        return info, pos

def analyze_properties(bitstring, start_pos):
    """Analyze property block starting from start_pos"""
    props = []
    pos = len(bitstring) - start_pos  # Convert to position from end
    
    # Look for end marker (511 = 0b111111111 in MSB-first)
    end_marker = '111111111'
    prop_section = bitstring[:pos]
    
    print(f"\n📊 Property Section Analysis:")
    print(f"   - Section length: {pos} bits")
    print(f"   - Section (last 50 bits): ...{prop_section[-50:] if len(prop_section) >= 50 else prop_section}")
    
    # Search for end marker
    marker_pos = prop_section.find(end_marker)
    if marker_pos != -1:
        print(f"   - End marker found at position: {marker_pos} (from start of section)")
        print(f"   - Bits before marker: {marker_pos}")
        print(f"   - Bits after marker: {len(prop_section) - marker_pos - 9}")
        
        if marker_pos > 0:
            print(f"   - Content before marker: {prop_section[:marker_pos]}")
    else:
        print(f"   - ⚠️  No end marker found!")
    
    return props

def main():
    if len(sys.argv) < 2:
        print("Usage: python debug_d2i_file.py <file.d2i> [original_file.d2i]")
        print("\nExamples:")
        print("  python debug_d2i_file.py d2i/rich.d2i.added_v3")
        print("  python debug_d2i_file.py d2i/rich.d2i.added_v3 d2i/rich.d2i")
        sys.exit(1)
    
    modified_file = sys.argv[1]
    original_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    print("=" * 80)
    print("D2I FILE STRUCTURE ANALYZER")
    print("=" * 80)
    
    # Analyze modified file
    print(f"\n📁 Analyzing: {modified_file}")
    data = read_d2i_file(modified_file)
    if not data:
        sys.exit(1)
    
    print(f"   - File size: {len(data)} bytes")
    print(f"   - Header: {data[:2]}")
    print(f"   - Raw bytes: {' '.join(f'{b:02x}' for b in data)}")
    
    bitstring = create_bitstring_prepend_msb(data)
    print(f"   - BitString length: {len(bitstring)} bits")
    print(f"   - BitString (first 100): {bitstring[-100:] if len(bitstring) >= 100 else bitstring}...")
    
    info, pos_after_header = parse_item_header(bitstring)
    
    print(f"\n📋 Parsed Header Info:")
    for key, value in info.items():
        print(f"   - {key}: {value}")
    
    analyze_properties(bitstring, info.get('properties_start_pos', 0))
    
    # Compare with original if provided
    if original_file:
        print(f"\n\n{'=' * 80}")
        print(f"📁 Analyzing Original: {original_file}")
        orig_data = read_d2i_file(original_file)
        if orig_data:
            print(f"   - File size: {len(orig_data)} bytes")
            print(f"   - Raw bytes: {' '.join(f'{b:02x}' for b in orig_data)}")
            
            orig_bitstring = create_bitstring_prepend_msb(orig_data)
            print(f"   - BitString length: {len(orig_bitstring)} bits")
            
            orig_info, _ = parse_item_header(orig_bitstring)
            print(f"\n📋 Original Header Info:")
            for key, value in orig_info.items():
                print(f"   - {key}: {value}")
            
            analyze_properties(orig_bitstring, orig_info.get('properties_start_pos', 0))
            
            # Compare
            print(f"\n\n{'=' * 80}")
            print("🔍 COMPARISON:")
            print(f"   - Size change: {len(orig_data)} → {len(data)} bytes ({len(data) - len(orig_data):+d})")
            print(f"   - BitString change: {len(orig_bitstring)} → {len(bitstring)} bits ({len(bitstring) - len(orig_bitstring):+d})")
            
            if info.get('itemType') != orig_info.get('itemType'):
                print(f"   - ⚠️  ITEM TYPE CHANGED: '{orig_info.get('itemType')}' → '{info.get('itemType')}'")
            
            if info.get('isEar') != orig_info.get('isEar'):
                print(f"   - ⚠️  IS_EAR FLAG CHANGED: {orig_info.get('isEar')} → {info.get('isEar')}")

if __name__ == '__main__':
    main()
