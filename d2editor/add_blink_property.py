#!/usr/bin/env python3
"""
Add Blink Property to D2I Item
================================

Adds the special property found in item_with_blink.d2i:
- Property ID: 0 (Strength)
- Parameter: 1134 (Blink Skill ID)
- Value: 5

This is a special property with parameter that doesn't normally exist
in the database (Strength doesn't have paramBits defined).

Usage:
    python3 add_blink_property.py <input.d2i> <output.d2i>
    
Example:
    python3 add_blink_property.py d2i/rich_clean_magic.d2i d2i/output_with_blink.d2i
"""

import sys
import sqlite3
from pathlib import Path

# Import the property_adder module
try:
    from property_adder import PropertyAdder, load_properties_from_db
    from property_bits import build_forward_property_bits
except ImportError:
    print("❌ Error: property_adder.py not found!")
    print("Please ensure property_adder.py is in the same directory.")
    sys.exit(1)


def add_blink_property_to_file(input_file: str, output_file: str, 
                               property_id: int = 0, 
                               parameter: int = 1134, 
                               value: int = 5):
    """
    Add the Blink property to a D2I file.
    
    Args:
        input_file: Path to input .d2i file
        output_file: Path to output .d2i file
        property_id: Property ID (default: 0 = Strength)
        parameter: Skill ID (default: 1134 = Blink)
        value: Property value (default: 5)
    """
    
    print("=" * 70)
    print("ADD BLINK PROPERTY TO D2I ITEM")
    print("=" * 70)
    print(f"Input file:    {input_file}")
    print(f"Output file:   {output_file}")
    print(f"Property ID:   {property_id} (Strength)")
    print(f"Parameter:     {parameter} (Blink Skill ID)")
    print(f"Value:         {value}")
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
    conn.close()
    
    if not prop_info:
        print(f"❌ Error: Property ID {property_id} not found in database!")
        return False
    
    code, name, addv, bits, param_bits, h_save_param_bits = prop_info
    
    print(f"\n📊 Property Info from Database:")
    print(f"   Name:              {name}")
    print(f"   Add Value:         {addv}")
    print(f"   Value Bits:        {bits}")
    print(f"   Param Bits:        {param_bits if param_bits else 'NULL'}")
    print(f"   h_saveParamBits:   {h_save_param_bits if h_save_param_bits else 'NULL'}")
    
    # Calculate parameter bits needed
    if parameter > 0:
        # Calculate minimum bits needed for parameter
        import math
        param_bits_needed = max(11, math.ceil(math.log2(parameter + 1)))
        print(f"\n⚠️  Parameter {parameter} requires at least {param_bits_needed} bits")
        
        # Override param_bits for this special case
        if not param_bits:
            param_bits = param_bits_needed
            print(f"   Using {param_bits} bits for parameter (database has NULL)")
    
    # Load and modify item
    print(f"\n🔧 Processing item file...")
    
    try:
        adder = PropertyAdder(input_file)
        
        # Load the file
        if not adder.load_file():
            print(f"❌ Error: Failed to load file {input_file}")
            return False
        
        # Add the property with parameter
        print(f"\n➕ Adding property...")
        print(f"   Property ID: {property_id}")
        print(f"   Parameter:   {parameter} ({param_bits} bits)")
        print(f"   Value:       {value} ({bits} bits)")
        
        # Build property bitstring using centralized helper (forward order: [value][param][ID])
        prop_info = {
            'name': name,
            'addv': addv,
            'bits': bits,
            'paramBits': param_bits,
            'h_saveParamBits': None
        }

        full_property, vb, pb, total_bits, raw_value = build_forward_property_bits(
            prop_info, property_id, value, parameter
        )

        print(f"\n📝 Bit Encoding (forward order [value][param][ID]):")
        print(f"   Value bits ({vb}), Param bits ({pb}), ID bits (9)")
        print(f"   Total bits:        {total_bits} bits")
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

        # Convert back to bytes (use canonical helper)
        from bitutils import bitstring_to_bytes
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
    
    if len(sys.argv) < 3:
        print(__doc__)
        print("\n❌ Error: Missing arguments!")
        print("\nUsage:")
        print(f"  {sys.argv[0]} <input.d2i> <output.d2i>")
        print("\nOptional arguments:")
        print(f"  {sys.argv[0]} <input.d2i> <output.d2i> <property_id> <parameter> <value>")
        print("\nExamples:")
        print(f"  {sys.argv[0]} d2i/rich_clean_magic.d2i d2i/output.d2i")
        print(f"  {sys.argv[0]} d2i/rich_clean_magic.d2i d2i/output.d2i 0 1134 5")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    # Optional custom values
    property_id = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    parameter = int(sys.argv[4]) if len(sys.argv) > 4 else 1134
    value = int(sys.argv[5]) if len(sys.argv) > 5 else 5
    
    # Verify input file exists
    if not Path(input_file).exists():
        print(f"❌ Error: Input file not found: {input_file}")
        sys.exit(1)
    
    # Add the property
    success = add_blink_property_to_file(
        input_file, 
        output_file, 
        property_id, 
        parameter, 
        value
    )
    
    if success:
        print("\n" + "=" * 70)
        print("🎉 DONE! Property added successfully!")
        print("=" * 70)
        sys.exit(0)
    else:
        print("\n" + "=" * 70)
        print("❌ FAILED! Could not add property.")
        print("=" * 70)
        sys.exit(1)


if __name__ == '__main__':
    main()
