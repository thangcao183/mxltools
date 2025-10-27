#!/usr/bin/env python3
"""
Python version of property_adder_logic.cpp
Adds properties to D2I item files using LSB-first bit encoding
"""

import struct
import os
import sqlite3
from pathlib import Path
from typing import List, Dict, Tuple, Optional

# Property database - will be loaded from props.db
PROPERTIES = {}

def load_properties_from_db():
    """Load all properties from data/props.db"""
    global PROPERTIES
    
    db_path = Path(__file__).parent / 'data' / 'props.db'
    if not db_path.exists():
        print(f"⚠️ Warning: props.db not found at {db_path}")
        print("Using fallback hardcoded properties")
        # Fallback to some basic properties
        PROPERTIES = {
            0: {"name": "strength", "add": 200, "bits": 11, "paramBits": 0},
            1: {"name": "energy", "add": 200, "bits": 11, "paramBits": 0},
            2: {"name": "dexterity", "add": 200, "bits": 11, "paramBits": 0},
            3: {"name": "vitality", "add": 200, "bits": 11, "paramBits": 0},
            7: {"name": "maxhp", "add": 500, "bits": 12, "paramBits": 0},
            79: {"name": "item_goldbonus", "add": 100, "bits": 9, "paramBits": 0},
            80: {"name": "item_magicbonus", "add": 100, "bits": 9, "paramBits": 0},
            93: {"name": "item_fasterattackrate", "add": 150, "bits": 9, "paramBits": 0},
            127: {"name": "item_allskills", "add": 0, "bits": 5, "paramBits": 0},
        }
        return
    
    try:
        conn = sqlite3.connect(str(db_path))
        cursor = conn.cursor()
        
        # Query all properties with required fields
        cursor.execute("SELECT code, name, addv, bits, paramBits FROM props")
        
        for row in cursor.fetchall():
            code = row[0]
            name = row[1] if row[1] else f"prop_{code}"
            add = row[2] if row[2] is not None else 0
            bits = row[3] if row[3] is not None else 0
            param_bits = row[4] if row[4] is not None else 0
            
            # Only add properties that have valid bits (can be encoded)
            if bits > 0:
                PROPERTIES[code] = {
                    "name": name,
                    "add": add,
                    "bits": bits,
                    "paramBits": param_bits
                }
        
        conn.close()
        print(f"✅ Loaded {len(PROPERTIES)} properties from database")
        
    except Exception as e:
        print(f"❌ Error loading properties from database: {e}")
        print("Using fallback hardcoded properties")
        PROPERTIES = {
            0: {"name": "strength", "add": 200, "bits": 11, "paramBits": 0},
            7: {"name": "maxhp", "add": 500, "bits": 12, "paramBits": 0},
            79: {"name": "item_goldbonus", "add": 100, "bits": 9, "paramBits": 0},
            80: {"name": "item_magicbonus", "add": 100, "bits": 9, "paramBits": 0},
        }

# Load properties when module is imported
load_properties_from_db()

from bitutils import create_bitstring_from_bytes, bitstring_to_bytes, number_to_binary_msb
from property_bits import build_forward_property_bits

def number_to_binary(number: int, bits: int) -> str:
    """Backwards-compatible helper that returns MSB-first binary strings."""
    from bitutils import number_to_binary_msb
    return number_to_binary_msb(number, bits)

def find_all_end_markers(bitstring: str) -> List[int]:
    """Find all positions of end marker (111111111) in bitstring"""
    positions = []
    end_marker = "111111111"
    pos = 0
    while True:
        pos = bitstring.find(end_marker, pos)
        if pos == -1:
            break
        positions.append(pos)
        pos += 1
    return positions

class PropertyAdder:
    def __init__(self, filename: str):
        self.filename = filename
        self.original_bytes = None
        self.original_bitstring = None
        self.end_marker_pos = None
        
    def load_file(self) -> bool:
        """Load D2I file and analyze structure"""
        try:
            with open(self.filename, 'rb') as f:
                self.original_bytes = f.read()
            
            print(f"\n=== LOADING FILE ===")
            print(f"📁 File: {self.filename} ({len(self.original_bytes)} bytes)")
            
            # Create bitstring (use canonical helper)
            self.original_bitstring = create_bitstring_from_bytes(self.original_bytes)
            print(f"📊 Original bitString: {len(self.original_bitstring)} bits")
            
            # Find end markers
            positions = find_all_end_markers(self.original_bitstring)
            if not positions:
                print("❌ Cannot find any end marker in bitString")
                return False
            
            print(f"🔍 Found {len(positions)} occurrences of end marker:")
            for p in positions:
                print(f"   - Position {p}")
            
            # Use the last one
            self.end_marker_pos = positions[-1]
            print(f"🔍 Using end marker at position: {self.end_marker_pos}")
            
            # Analyze structure
            content_bits = self.end_marker_pos
            padding_bits = len(self.original_bitstring) - self.end_marker_pos - 9
            
            print(f"📏 Analysis:")
            print(f"   - Total bitString: {len(self.original_bitstring)} bits")
            print(f"   - End marker position: {self.end_marker_pos}")
            print(f"   - Current padding: {padding_bits} bits")
            print(f"   - Current content: {content_bits} bits")
            
            return True
            
        except Exception as e:
            print(f"❌ Error loading file: {e}")
            return False
    
    def add_property(self, prop_id: int, value: int) -> Tuple[str, int]:
        """Add a single property and return the new property bits and total bits"""
        if prop_id not in PROPERTIES:
            raise ValueError(f"Unknown property ID: {prop_id}")
        
        prop = PROPERTIES[prop_id]
        # Use shared helper to build MSB-forward bits: [value][param][ID]
        prop_info = {
            'name': prop.get('name'),
            'addv': prop.get('add', 0),
            'bits': prop.get('bits', 0),
            'paramBits': prop.get('paramBits', 0),
            'h_saveParamBits': None
        }

        prop_bits, vb, pb, total_bits, raw_value = build_forward_property_bits(prop_info, prop_id, value, 0)

        print(f"🔧 Property: {prop['name']} (ID={prop_id}, Value={value})")
        print(f"   - Value bits ({vb}): (raw={raw_value})")
        if pb > 0:
            print(f"   - Param bits ({pb})")
        print(f"   - Total: {total_bits} bits")

        return prop_bits, total_bits
    
    def add_properties(self, properties: List[Tuple[int, int]]) -> bool:
        """
        Add multiple properties to the file
        properties: List of (property_id, value) tuples
        """
        if not self.original_bitstring:
            print("❌ File not loaded. Call load_file() first.")
            return False
        
        print(f"\n=== ADDING {len(properties)} PROPERTIES ===")
        
        # Get pure content (before end marker)
        pure_content = self.original_bitstring[:self.end_marker_pos]
        print(f"✂️  Pure content (no end marker/padding): {len(pure_content)} bits")
        
        # Add all properties
        all_property_bits = ""
        for prop_id, value in properties:
            property_bits, bits_count = self.add_property(prop_id, value)
            all_property_bits += property_bits
        
        print(f"\n➕ Total new property bits: {len(all_property_bits)} bits")
        
        # Create new content
        new_content = pure_content + all_property_bits
        print(f"➕ Content after adding properties: {len(new_content)} bits")
        
        # Add end marker
        end_marker = "111111111"
        print(f"🔚 Adding end marker: {end_marker}")
        
        # Calculate padding for byte boundary
        total_content_bits = len(new_content) + 9  # +9 for end marker
        padding_needed = (8 - (total_content_bits % 8)) % 8
        padding = '0' * padding_needed
        
        print(f"🔢 New padding needed: {padding_needed} bits")
        if padding_needed > 0:
            print(f"📋 New padding pattern: \"{padding}\"")
        
        # Create final bitstring
        final_bitstring = new_content + end_marker + padding
        print(f"🎯 Final bitString: {len(final_bitstring)} bits")
        
        # Convert to bytes
        new_bytes = bitstring_to_bytes(final_bitstring)
        print(f"📦 New file size: {len(new_bytes)} bytes")
        print(f"📈 Size change: {len(self.original_bytes)} → {len(new_bytes)} "
              f"(+{len(new_bytes) - len(self.original_bytes)} bytes)")
        
        # Save to new file
        output_file = self.filename + ".added"
        try:
            with open(output_file, 'wb') as f:
                f.write(new_bytes)
            print(f"✅ Successfully created: {output_file}")
            return True
        except Exception as e:
            print(f"❌ Error writing file: {e}")
            return False

def add_properties_to_file(filename: str, properties: List[Tuple[int, int]]) -> bool:
    """
    Convenience function to add properties to a file
    
    Args:
        filename: Path to .d2i file
        properties: List of (property_id, value) tuples
        
    Returns:
        True if successful, False otherwise
        
    Example:
        add_properties_to_file("rich.d2i", [(7, 10), (79, 50), (80, 30)])
    """
    adder = PropertyAdder(filename)
    if not adder.load_file():
        return False
    return adder.add_properties(properties)

def main():
    import sys
    
    if len(sys.argv) < 4 or len(sys.argv) % 2 != 0:
        print("Usage: python3 property_adder.py <file.d2i> <prop_id1> <value1> [<prop_id2> <value2> ...]")
        print("\nExamples:")
        print("  python3 property_adder.py amu.d2i 7 10                # Add +10 Life")
        print("  python3 property_adder.py amu.d2i 7 10 79 50         # Add +10 Life and 50% Gold Find")
        print("  python3 property_adder.py amu.d2i 0 20 1 15 2 10     # Add +20 Str, +15 Energy, +10 Dex")
        print(f"\nTotal Available Properties: {len(PROPERTIES)}")
        print("\nCommon Properties (use --list-all to see all):")
        common = [0, 1, 2, 3, 7, 9, 79, 80, 93, 127]
        for prop_id in common:
            if prop_id in PROPERTIES:
                prop = PROPERTIES[prop_id]
                print(f"  ID {prop_id:3d} = {prop['name']:<30} (add={prop['add']}, bits={prop['bits']})")
        
        if '--list-all' in sys.argv:
            print(f"\n=== All {len(PROPERTIES)} Properties ===")
            for prop_id, prop in sorted(PROPERTIES.items()):
                print(f"  ID {prop_id:3d} = {prop['name']:<30} (add={prop['add']}, bits={prop['bits']})")
        else:
            print("\nUse '--list-all' flag to see all properties")
        return 1
    
    filename = sys.argv[1]
    
    # Parse properties from command line
    properties = []
    for i in range(2, len(sys.argv), 2):
        prop_id = int(sys.argv[i])
        value = int(sys.argv[i + 1])
        properties.append((prop_id, value))
    
    print(f"🎯 Target file: {filename}")
    print(f"📝 Properties to add: {len(properties)}")
    
    success = add_properties_to_file(filename, properties)
    
    if success:
        print("\n🎉 Property addition completed successfully!")
        print("📋 You can now test the modified file with MedianXL Tools")
        return 0
    else:
        print("\n❌ Property addition failed!")
        return 1

if __name__ == "__main__":
    exit(main())
