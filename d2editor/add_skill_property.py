#!/usr/bin/env python3
"""
Add Oskill Property (Property 97) to D2I Item
==============================================

Adds Oskill (cross-class skill) using property 97 (item_nonclassskill):
- Property ID: 97
- Parameter: Skill ID (12 bits from h_saveParamBits)
- Value: Skill level (7 bits)

Usage:
    python3 add_skill_property.py <input.d2i> <output.d2i> <skill_id> <level>
    
Example:
    python3 add_skill_property.py d2i/amulet_clean.d2i d2i/output.d2i 1134 5
    # Adds Level 5 Blink Oskill (Skill ID 1134)
"""

import sys
import sqlite3
from pathlib import Path

# Import the property_adder module
try:
    from property_adder import PropertyAdder, number_to_binary_lsb, bitstring_to_bytes
except ImportError:
    print("❌ Error: property_adder.py not found!")
    print("Please ensure property_adder.py is in the same directory.")
    sys.exit(1)


def add_skill_property(input_file: str, output_file: str, 
                       skill_id: int, skill_level: int):
    """
    Add +X to Skill property to a D2I file.
    
    Args:
        input_file: Path to input .d2i file
        output_file: Path to output .d2i file
        skill_id: Skill ID from skills.tsv
        skill_level: Skill level (e.g., 3 for +3)
    """
    
    print("=" * 70)
    print("ADD SKILL PROPERTY TO D2I ITEM")
    print("=" * 70)
    print(f"Input file:    {input_file}")
    print(f"Output file:   {output_file}")
    print(f"Skill ID:      {skill_id}")
    print(f"Skill Level:   +{skill_level}")
    print("=" * 70)

    # Property 97 = item_nonclassskill (Oskill)
    property_id = 97
    
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
    
    code, name, addv, bits, param_bits, h_save_param_bits = prop_info
    
    # Use h_saveParamBits for runeword properties
    param_bits_to_use = h_save_param_bits if h_save_param_bits else param_bits
    if param_bits_to_use:
        param_bits_to_use = int(param_bits_to_use)
    
    print(f"\n📊 Property Info from Database:")
    print(f"   Property ID:       {property_id}")
    print(f"   Name:              {name}")
    print(f"   Add Value:         {addv}")
    print(f"   Value Bits:        {bits}")
    print(f"   Param Bits:        {param_bits if param_bits else 'NULL'}")
    print(f"   h_saveParamBits:   {h_save_param_bits if h_save_param_bits else 'NULL'}")
    print(f"   → Using:           {param_bits_to_use} bits for parameter")
    
    # Lookup skill name
    cursor.execute("SELECT name, class FROM skills WHERE code = ?", (skill_id,))
    skill_info = cursor.fetchone()
    conn.close()
    
    if skill_info:
        skill_name = skill_info[0] if skill_info[0] else f"Skill {skill_id}"
        skill_class = skill_info[1] if skill_info[1] else "Unknown"
        print(f"\n🎯 Skill Info:")
        print(f"   Skill ID:          {skill_id}")
        print(f"   Skill Name:        {skill_name}")
        print(f"   Class:             {skill_class}")
    else:
        print(f"\n⚠️  Warning: Skill ID {skill_id} not found in database")
        print(f"   Using skill ID anyway...")
    
    # Validate skill_id fits in param_bits_to_use
    max_skill_id = (1 << param_bits_to_use) - 1
    if skill_id > max_skill_id:
        print(f"\n❌ Error: Skill ID {skill_id} too large for {param_bits_to_use} bits!")
        print(f"   Maximum skill ID: {max_skill_id}")
        return False
    
    # Validate skill_level fits in bits
    max_level = (1 << bits) - 1 - addv
    if skill_level < 0 or skill_level > max_level:
        print(f"\n❌ Error: Skill level {skill_level} out of range!")
        print(f"   Valid range: 0 to {max_level}")
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
        print(f"   Property ID: {property_id}")
        print(f"   Parameter:   {skill_id} ({param_bits_to_use} bits)")
        print(f"   Value:       +{skill_level} ({bits} bits)")
        
        # Format: [Property ID 9 bits][Parameter N bits][Value M bits]
        property_bits = number_to_binary_lsb(property_id, 9)
        parameter_bits = number_to_binary_lsb(skill_id, param_bits_to_use)
        
        # Apply add offset to value before encoding
        storage_value = skill_level + addv
        value_bits = number_to_binary_lsb(storage_value, bits)
        
        print(f"\n📝 Bit Encoding:")
        print(f"   Property ID bits:  {property_bits} ({len(property_bits)} bits)")
        print(f"   Parameter bits:    {parameter_bits} ({len(parameter_bits)} bits)")
        print(f"   Value bits:        {value_bits} ({len(value_bits)} bits)")
        print(f"   Storage value:     {storage_value} (display: +{skill_level}, add: {addv})")
        
        # Combine all bits
        full_property = property_bits + parameter_bits + value_bits
        print(f"   Total bits:        {len(full_property)} bits")
        print(f"   Combined:          {full_property}")
        
        # Find end marker position (before 0x1FF)
        end_marker = '111111111'  # 0x1FF in binary
        try:
            end_pos = adder.original_bitstring.index(end_marker)
            print(f"\n🔍 Found end marker at bit position: {end_pos}")
        except ValueError:
            print(f"\n❌ Error: End marker (0x1FF) not found in bitstring!")
            return False
        
        # Get pure content (before end marker)
        pure_content = adder.original_bitstring[:end_pos]
        print(f"✂️  Pure content (no end marker): {len(pure_content)} bits")
        
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


def main():
    """Main entry point."""
    
    if len(sys.argv) < 5:
        print(__doc__)
        print("\n❌ Error: Missing arguments!")
        print("\nUsage:")
        print(f"  {sys.argv[0]} <input.d2i> <output.d2i> <skill_id> <level>")
        print("\nExamples:")
        print(f"  {sys.argv[0]} d2i/amulet_clean.d2i d2i/output.d2i 1134 3")
        print(f"    → Adds +3 to Blink (Skill ID 1134)")
        print(f"  {sys.argv[0]} d2i/amulet_clean.d2i d2i/output.d2i 54 5")
        print(f"    → Adds +5 to Teleport (Skill ID 54)")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    skill_id = int(sys.argv[3])
    skill_level = int(sys.argv[4])
    
    # Verify input file exists
    if not Path(input_file).exists():
        print(f"❌ Error: Input file not found: {input_file}")
        sys.exit(1)
    
    # Add the property
    success = add_skill_property(input_file, output_file, skill_id, skill_level)
    
    if success:
        print("\n" + "=" * 70)
        print("🎉 DONE! Skill property added successfully!")
        print("=" * 70)
        sys.exit(0)
    else:
        print("\n" + "=" * 70)
        print("❌ FAILED! Could not add skill property.")
        print("=" * 70)
        sys.exit(1)


if __name__ == '__main__':
    main()
