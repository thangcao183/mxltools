#!/usr/bin/env python3
"""
Calculate exact properties start position for rich.d2i
Following ItemParser logic exactly
"""

def calculate_properties_start():
    print("="*60)
    print("Calculating properties start position for rich.d2i")
    print("="*60)
    
    # According to ItemParser.cpp parsing sequence
    # ReverseBitReader starts from END of bitString
    
    bit_pos = 0
    
    def skip(bits, name):
        nonlocal bit_pos
        print(f"  Position {bit_pos:3d}: Skip {bits:2d} bits - {name}")
        bit_pos += bits
    
    def read(bits, name):
        nonlocal bit_pos
        print(f"  Position {bit_pos:3d}: Read {bits:2d} bits - {name}")
        bit_pos += bits
        return 0  # placeholder
    
    print("\n📊 Parsing sequence (from END of bitString):")
    print()
    
    # Header flags
    skip(1, "isQuest")
    skip(3, "unknown1")
    skip(1, "isIdentified")
    skip(5, "unknown2")
    skip(1, "isDuped")
    read(1, "isSocketed")
    skip(2, "unknown3")
    skip(2, "illegal+unk")
    read(1, "isEar")
    read(1, "isStarter")
    skip(2, "unknown4")
    skip(1, "unknown5")
    is_extended_bit = bit_pos
    read(1, "isExtended")
    skip(1, "isEthereal")
    skip(1, "unknown6")
    read(1, "isPersonalized")
    skip(1, "unknown7")
    read(1, "isRW")
    skip(5, "unknown8")
    skip(8, "version")
    skip(2, "unknown9")
    skip(3, "location")
    skip(4, "whereEquipped")
    skip(4, "column")
    skip(4, "row")
    skip(3, "storage")
    
    print(f"\n✅ After header: position = {bit_pos}")
    print(f"   (isExtended bit was at position {is_extended_bit})")
    
    # Item type (4 bytes)
    skip(32, "itemType (4 chars)")
    
    print(f"\n✅ After item type: position = {bit_pos}")
    
    # isExtended = true for rich.d2i
    print("\n📦 Item is extended, reading extended fields:")
    skip(3, "socketablesNumber")
    skip(32, "guid")
    skip(7, "ilvl")
    quality = bit_pos
    skip(4, "quality")
    
    print(f"\n   Quality field at position {quality}")
    print(f"   For rich.d2i, quality = 4 (Normal)")
    
    # Variable graphic check
    print(f"\n   Checking variable graphic bit...")
    has_variable_graphic = False  # Need to read from actual file
    read(1, "has variable graphic")
    if has_variable_graphic:
        skip(3, "variable graphic index")
    
    # Autoprefix check
    print(f"   Checking autoprefix bit...")
    has_autoprefix = False  # Need to read from actual file
    read(1, "has autoprefix")
    if has_autoprefix:
        skip(11, "autoprefix")
    
    # Quality-specific fields
    print(f"\n   Quality = Normal → no extra fields")
    
    # RW check
    print(f"\n   isRW = false → no RW code")
    
    # Personalization
    print(f"   isPersonalized = false → no inscribed name")
    
    # Tome check
    skip(1, "tome of ID bit")
    
    print(f"\n✅ After extended fields: position = {bit_pos}")
    
    # rich.d2i is miscellaneous (not armor/weapon)
    print("\n📦 Item is miscellaneous → no defense/durability")
    
    # Quantity (rich.d2i is stackable?)
    print("\n   Need to check if stackable...")
    print("   If stackable: read 9 bits for quantity")
    print("   If not stackable: skip")
    
    # For now, assume rich.d2i is NOT stackable (need to verify)
    # skip(9, "quantity")
    
    # Sockets
    print(f"\n   isSocketed = false → no sockets field")
    
    # Set lists (only for Set quality items)
    print(f"   Quality != Set → no set lists")
    
    print(f"\n" + "="*60)
    print(f"🎯 PROPERTIES START AT POSITION: {bit_pos}")
    print(f"="*60)
    print(f"\nThis position is counted from END of bitString")
    print(f"(ReverseBitReader reads backwards)")
    
    return bit_pos

if __name__ == "__main__":
    pos = calculate_properties_start()
    
    # For rich.d2i with 14 bytes = 96 bits after JM header
    print(f"\n📁 For rich.d2i:")
    print(f"   File size: 14 bytes")
    print(f"   BitString length: 96 bits (after JM header)")
    print(f"   Properties start: position {pos} from END")
    print(f"   Properties start in bitString: index {96 - pos}")
    print(f"\n💡 To insert property:")
    print(f"   1. Convert file to bitString (prepend logic)")
    print(f"   2. Insert property bits at index {96 - pos}")
    print(f"   3. Add end marker (9 bits = 511)")
    print(f"   4. Byte align (add padding zeros)")
    print(f"   5. Convert back to bytes (prepend logic)")
