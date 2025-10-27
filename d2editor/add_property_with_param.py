#!/usr/bin/env python3
"""
Add Property with Parameter to D2I Item
========================================

General-purpose script to add ANY property with parameter to D2I items.
Property information (bits, paramBits, addv) is loaded from props.db.

Supports all parameterized properties:
- Property 97: item_nonclassskill (Oskill)
- Property 107: item_singleskill (+Skills)
- Property 151: item_aura (Auras)
- Any other property with paramBits or h_saveParamBits

Usage:
    python3 add_property_with_param.py <input.d2i> <output.d2i> <prop_id> <value> <param>
    
Examples:
    # Add Level 5 Blink Oskill (Property 97, Skill ID 1134)
    python3 add_property_with_param.py d2i/amulet.d2i d2i/output.d2i 97 5 1134
    
    # Add +3 to Blink (Property 107, Skill ID 1134)
    python3 add_property_with_param.py d2i/amulet.d2i d2i/output.d2i 107 3 1134
    
    # Add Level 10 Might Aura (Property 151, Aura ID 98)
    python3 add_property_with_param.py d2i/amulet.d2i d2i/output.d2i 151 10 98
"""

import sys
import sqlite3
from pathlib import Path

from property_adder import PropertyAdder
from bitutils import bitstring_to_bytes
from property_bits import build_forward_property_bits


def add_property_with_param(input_file: str, output_file: str, 
                            property_id: int, value: int, parameter: int):
    """
    Add a property with parameter to a D2I file.
    
    Args:
        input_file: Path to input .d2i file
        output_file: Path to output .d2i file
        property_id: Property ID from props.db
        value: Property value
        parameter: Property parameter (e.g., skill ID, aura ID)
    """
    
    print("=" * 70)
    print("ADD PROPERTY WITH PARAMETER TO D2I ITEM")
    print("=" * 70)
    print(f"Input file:    {input_file}")
    print(f"Output file:   {output_file}")
    print(f"Property ID:   {property_id}")
    print(f"Value:         {value}")
    print(f"Parameter:     {parameter}")
    print("=" * 70)
    
    # Load property database
    db_path = Path('data/props.db')
    if not db_path.exists():
        print(f"❌ Error: Database not found at {db_path}")
        return False
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Get property info from database
    cursor.execute("""
        SELECT code, name, addv, bits, paramBits, h_saveParamBits 
        FROM props 
        WHERE code = ?
    """, (property_id,))
    
    prop_info = cursor.fetchone()
    
    if not prop_info:
        print(f"❌ Error: Property ID {property_id} not found in database!")
        conn.close()
        return False
    
    code, name, addv, bits, param_bits_col, h_save_param_bits = prop_info
    
    # Determine which parameter bits to use
    # Priority: h_saveParamBits (runeword) > paramBits (normal)
    param_bits_to_use = None
    param_source = None
    
    if h_save_param_bits:
        param_bits_to_use = int(h_save_param_bits)
        param_source = "h_saveParamBits (runeword)"
    elif param_bits_col:
        param_bits_to_use = int(param_bits_col)
        param_source = "paramBits"
    else:
        print(f"❌ Error: Property {property_id} ({name}) has no parameter bits defined!")
        print(f"   paramBits: {param_bits_col}")
        print(f"   h_saveParamBits: {h_save_param_bits}")
        conn.close()
        return False
    
    print(f"\n📊 Property Info from Database:")
    print(f"   Property ID:       {property_id}")
    print(f"   Name:              {name}")
    print(f"   Add Value:         {addv if addv else 0}")
    print(f"   Value Bits:        {bits if bits else 0}")
    print(f"   Param Bits (DB):   {param_bits_col if param_bits_col else 'NULL'}")
    print(f"   h_saveParamBits:   {h_save_param_bits if h_save_param_bits else 'NULL'}")
    print(f"   → Using:           {param_bits_to_use} bits for parameter ({param_source})")
    
    # Validate bits
    if not bits or bits <= 0:
        print(f"\n❌ Error: Property {property_id} has invalid value bits: {bits}")
        conn.close()
        return False
    
    bits = int(bits)
    addv = int(addv) if addv else 0
    
    # Try to lookup parameter name (skill or aura)
    param_name = None
    param_type = None
    
    # Check if it's a skill
    cursor.execute("SELECT name, class FROM skills WHERE code = ?", (parameter,))
    skill_info = cursor.fetchone()
    if skill_info:
        param_name = skill_info[0] if skill_info[0] else f"Skill {parameter}"
        param_type = "Skill"
        skill_class = skill_info[1]
        print(f"\n🎯 Parameter Info (Skill):")
        print(f"   Parameter ID:      {parameter}")
        print(f"   Skill Name:        {param_name}")
        print(f"   Class:             {skill_class}")
    
    conn.close()
    
    # Validate parameter fits in param_bits_to_use
    max_param = (1 << param_bits_to_use) - 1
    if parameter < 0 or parameter > max_param:
        print(f"\n❌ Error: Parameter {parameter} out of range!")
        print(f"   Valid range: 0 to {max_param} ({param_bits_to_use} bits)")
        return False
    
    # Validate value fits in bits (after adding offset)
    storage_value = value + addv
    max_storage_value = (1 << bits) - 1
    if storage_value < 0 or storage_value > max_storage_value:
        print(f"\n❌ Error: Value {value} (storage: {storage_value}) out of range!")
        print(f"   Valid range: {-addv} to {max_storage_value - addv}")
        print(f"   (storage range: 0 to {max_storage_value}, {bits} bits)")
        return False
    
    # Load and modify item
    print(f"\n🔧 Processing item file...")
    
    try:
        adder = PropertyAdder(input_file)
        
        # Load the file
        if not adder.load_file():
            print(f"❌ Error: Failed to load file {input_file}")
            return False
        
        # Build property bitstring
        print(f"\n➕ Adding property...")
        print(f"   Property ID: {property_id} ({name})")
        print(f"   Parameter:   {parameter} ({param_bits_to_use} bits)")
        if param_name:
            print(f"                → {param_name}")
        print(f"   Value:       {value} ({bits} bits)")
        
        # Build forward-order property bits using centralized helper
        prop_info_dict = {
            'name': name,
            'addv': addv,
            'bits': bits,
            'paramBits': param_bits_col,
            'h_saveParamBits': h_save_param_bits
        }

        full_property, vb, pb, total_bits, raw_value = build_forward_property_bits(
            prop_info_dict, property_id, value, parameter
        )

        print(f"\n📝 Bit Encoding (forward order: [value][param][ID]):")
        print(f"   Value bits ({vb}), Param bits ({pb}), ID bits (9)")
        print(f"   Total bits:        {total_bits} bits")
        print(f"   Combined:          {full_property}")
        
        # Use end marker position from PropertyAdder (last occurrence)
        end_marker = '111111111'  # 0x1FF in binary
        end_pos = adder.end_marker_pos
        
        if end_pos is None:
            print(f"\n❌ Error: End marker position not found!")
            return False
        
        print(f"\n🔍 Using end marker at bit position: {end_pos} (from PropertyAdder)")
        
        # Get pure content (before end marker) - this preserves ALL existing properties!
        pure_content = adder.original_bitstring[:end_pos]
        print(f"✂️  Pure content (no end marker): {len(pure_content)} bits")
        print(f"   This includes ALL existing properties!")
        
        # Insert property before end marker
        new_content = pure_content + full_property
        print(f"➕ Content after adding property: {len(new_content)} bits")
        
        # Add end marker back
        print(f"🔚 Adding end marker: {end_marker}")
        
        # Calculate padding for byte boundary
        total_content_bits = len(new_content) + 9  # +9 for end marker
        padding_needed = (8 - (total_content_bits % 8)) % 8
        padding = '0' * padding_needed
        
        print(f"🔢 Padding needed: {padding_needed} bits")
        if padding_needed > 0:
            print(f"📋 Padding pattern: \"{padding}\"")
        
        # Create final bitstring
        final_bitstring = new_content + end_marker + padding
        print(f"🎯 Final bitString: {len(final_bitstring)} bits")
        
        # Convert back to bytes (bitstring_to_bytes already includes JM header)
        final_bytes = bitstring_to_bytes(final_bitstring)
        
        print(f"✅ Property inserted successfully")
        print(f"   New bitstring length: {len(final_bitstring)} bits")
        print(f"   New data length: {len(final_bytes)} bytes")
        
        # Save to output file
        with open(output_file, 'wb') as f:
            f.write(final_bytes)
        
        print(f"\n✅ SUCCESS! Item saved to: {output_file}")
        
        # File size comparison
        input_size = Path(input_file).stat().st_size
        output_size = Path(output_file).stat().st_size
        size_diff = output_size - input_size
        
        print(f"\n📦 File Size:")
        print(f"   Input:   {input_size} bytes")
        print(f"   Output:  {output_size} bytes")
        print(f"   Diff:    +{size_diff} bytes")
        
        return True
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return False


def show_common_properties():
    """Show common parameterized properties for reference."""
    print("\n📚 Common Parameterized Properties:")
    print("=" * 70)
    
    db_path = Path('data/props.db')
    if not db_path.exists():
        print("Database not found. Cannot show property list.")
        return
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Query properties with parameters
    cursor.execute("""
        SELECT code, name, addv, bits, paramBits, h_saveParamBits 
        FROM props 
        WHERE paramBits IS NOT NULL OR h_saveParamBits IS NOT NULL
        ORDER BY code
    """)
    
    print(f"\n{'ID':<6} {'Name':<30} {'ParamBits':<12} {'ValueBits':<12} {'Add':<6}")
    print("-" * 70)
    
    count = 0
    for row in cursor.fetchall():
        code, name, addv, bits, param_bits, h_save_param = row
        param_display = str(h_save_param) if h_save_param else str(param_bits) if param_bits else "?"
        print(f"{code:<6} {name:<30} {param_display:<12} {bits if bits else '?':<12} {addv if addv else 0:<6}")
        count += 1
        
        # Show first 20 only
        if count >= 20:
            print(f"\n... and more (total: {count}+ properties with parameters)")
            break
    
    conn.close()
    print("=" * 70)
    
    print("\n💡 Popular Examples:")
    print("   Property 97:  item_nonclassskill (Oskill)")
    print("   Property 107: item_singleskill (+Skill)")
    print("   Property 151: item_aura (Aura when equipped)")


def main():
    """Main entry point."""
    
    if len(sys.argv) < 2:
        print(__doc__)
        show_common_properties()
        sys.exit(1)
    
    # Show help
    if sys.argv[1] in ['-h', '--help', 'help']:
        print(__doc__)
        show_common_properties()
        sys.exit(0)
    
    # Show property list
    if sys.argv[1] in ['-l', '--list', 'list']:
        show_common_properties()
        sys.exit(0)
    
    if len(sys.argv) < 6:
        print(__doc__)
        print("\n❌ Error: Missing arguments!")
        print("\nUsage:")
        print(f"  {sys.argv[0]} <input.d2i> <output.d2i> <prop_id> <value> <param>")
        print("\nExamples:")
        print(f"  {sys.argv[0]} d2i/amulet.d2i d2i/output.d2i 97 5 1134")
        print(f"    → Adds Level 5 Blink Oskill")
        print(f"  {sys.argv[0]} d2i/amulet.d2i d2i/output.d2i 107 3 1134")
        print(f"    → Adds +3 to Blink")
        print("\nFor more info:")
        print(f"  {sys.argv[0]} --help")
        print(f"  {sys.argv[0]} --list   (show all parameterized properties)")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    property_id = int(sys.argv[3])
    value = int(sys.argv[4])
    parameter = int(sys.argv[5])
    
    # Verify input file exists
    if not Path(input_file).exists():
        print(f"❌ Error: Input file not found: {input_file}")
        sys.exit(1)
    
    # Add the property
    success = add_property_with_param(
        input_file, 
        output_file, 
        property_id, 
        value,
        parameter
    )
    
    if success:
        print("\n" + "=" * 70)
        print("🎉 DONE! Property with parameter added successfully!")
        print("=" * 70)
        sys.exit(0)
    else:
        print("\n" + "=" * 70)
        print("❌ FAILED! Could not add property.")
        print("=" * 70)
        sys.exit(1)


if __name__ == '__main__':
    main()
