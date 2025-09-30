#!/usr/bin/env python3
"""
Complete D2 Item Extractor with Built-in Database
One-stop solution for building database and extracting items with proper names
"""

import os
import sys
import csv
import sqlite3
import shutil
from pathlib import Path

# Built-in Database Builder Class
class D2ItemDatabaseBuilder:
    def __init__(self, tsv_dir, db_path='d2_items.db'):
        self.tsv_dir = Path(tsv_dir)
        self.db_path = db_path
        self.conn = None
        
    def create_connection(self):
        try:
            self.conn = sqlite3.connect(self.db_path)
            self.conn.execute("PRAGMA foreign_keys = ON")
            return True
        except sqlite3.Error:
            return False
    
    def create_tables(self):
        # Items table
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS items (
                code TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                width INTEGER,
                height INTEGER,
                type TEXT,
                category TEXT DEFAULT 'item'
            )
        ''')
        
        # Gems table
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS gems (
                code TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                category TEXT DEFAULT 'gem'
            )
        ''')
        
        # Runes table
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS rune_items (
                code TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                number INTEGER,
                category TEXT DEFAULT 'rune'
            )
        ''')
        
        # Combined view
        self.conn.execute('''
            CREATE VIEW IF NOT EXISTS item_lookup AS
            SELECT code, name, category FROM items
            UNION ALL
            SELECT code, name, category FROM gems
            UNION ALL
            SELECT code, name, category FROM rune_items
        ''')
        
        self.conn.commit()
    
    def build_quick_database(self):
        """Build a quick database with essential items"""
        if not self.create_connection():
            return False
        
        self.create_tables()
        
        print("Building essential item database...")
        
        # Essential gems
        gems_data = [
            ('gcr', 'Chipped Ruby', 'gem'),
            ('gfr', 'Flawed Ruby', 'gem'),
            ('gsr', 'Ruby', 'gem'),
            ('glr', 'Flawless Ruby', 'gem'),
            ('gpr', 'Perfect Ruby', 'gem'),
            ('gcb', 'Chipped Sapphire', 'gem'),
            ('gfb', 'Flawed Sapphire', 'gem'),
            ('gsb', 'Sapphire', 'gem'),
            ('glb', 'Flawless Sapphire', 'gem'),
            ('gpb', 'Perfect Sapphire', 'gem'),
            ('gcg', 'Chipped Emerald', 'gem'),
            ('gfg', 'Flawed Emerald', 'gem'),
            ('gsg', 'Emerald', 'gem'),
            ('glg', 'Flawless Emerald', 'gem'),
            ('gpg', 'Perfect Emerald', 'gem'),
            ('gcv', 'Chipped Amethyst', 'gem'),
            ('gfv', 'Flawed Amethyst', 'gem'),
            ('gsv', 'Amethyst', 'gem'),
            ('glv', 'Flawless Amethyst', 'gem'),
            ('gpv', 'Perfect Amethyst', 'gem'),
            ('gcw', 'Chipped Diamond', 'gem'),
            ('gfw', 'Flawed Diamond', 'gem'),
            ('gsw', 'Diamond', 'gem'),
            ('glw', 'Flawless Diamond', 'gem'),
            ('gpw', 'Perfect Diamond', 'gem'),
            ('gcy', 'Chipped Topaz', 'gem'),
            ('gfy', 'Flawed Topaz', 'gem'),
            ('gsy', 'Topaz', 'gem'),
            ('gly', 'Flawless Topaz', 'gem'),
            ('gpy', 'Perfect Topaz', 'gem'),
        ]
        
        self.conn.executemany('INSERT OR REPLACE INTO gems (code, name, category) VALUES (?, ?, ?)', gems_data)
        
        # Essential runes
        rune_names = [
            "El", "Eld", "Tir", "Nef", "Eth", "Ith", "Tal", "Ral", "Ort", "Thul",
            "Amn", "Sol", "Shael", "Dol", "Hel", "Io", "Lum", "Ko", "Fal", "Lem",
            "Pul", "Um", "Mal", "Ist", "Gul", "Vex", "Ohm", "Lo", "Sur", "Ber",
            "Jah", "Cham", "Zod"
        ]
        
        runes_data = []
        for i, name in enumerate(rune_names, 1):
            code = f"r{i:02d}"
            runes_data.append((code, f"{name} Rune", i, 'rune'))
        
        self.conn.executemany('INSERT OR REPLACE INTO rune_items (code, name, number, category) VALUES (?, ?, ?, ?)', runes_data)
        
        # Essential items
        items_data = [
            ('jew', 'Jewel', 1, 1, 'item'),
            ('cm1', 'Small Charm', 1, 1, 'charm'),
            ('cm2', 'Large Charm', 1, 2, 'charm'),
            ('cm3', 'Grand Charm', 1, 3, 'charm'),
            ('hp1', 'Minor Healing Potion', 1, 2, 'potion'),
            ('hp2', 'Light Healing Potion', 1, 2, 'potion'),
            ('hp3', 'Healing Potion', 1, 2, 'potion'),
            ('hp4', 'Greater Healing Potion', 1, 2, 'potion'),
            ('hp5', 'Super Healing Potion', 1, 2, 'potion'),
            ('mp1', 'Minor Mana Potion', 1, 2, 'potion'),
            ('mp2', 'Light Mana Potion', 1, 2, 'potion'),
            ('mp3', 'Mana Potion', 1, 2, 'potion'),
            ('mp4', 'Greater Mana Potion', 1, 2, 'potion'),
            ('mp5', 'Super Mana Potion', 1, 2, 'potion'),
            ('tsc', 'Scroll of Town Portal', 1, 1, 'scroll'),
            ('isc', 'Scroll of Identify', 1, 1, 'scroll'),
        ]
        
        self.conn.executemany('INSERT OR REPLACE INTO items (code, name, width, height, type) VALUES (?, ?, ?, ?, ?)', items_data)
        
        self.conn.commit()
        self.conn.close()
        
        print(f"Built essential database: {self.db_path}")
        print(f"- {len(gems_data)} gems")
        print(f"- {len(runes_data)} runes") 
        print(f"- {len(items_data)} items")
        return True

# Built-in Item Lookup Class
class D2ItemLookup:
    def __init__(self, db_path='d2_items.db'):
        self.db_path = db_path
        self.conn = None
        self._connect()
        
    def _connect(self):
        if not os.path.exists(self.db_path):
            return False
        try:
            self.conn = sqlite3.connect(self.db_path)
            self.conn.row_factory = sqlite3.Row
            return True
        except sqlite3.Error:
            return False
    
    def lookup_item(self, code):
        if not self.conn:
            return None
        try:
            cursor = self.conn.execute("SELECT code, name, category FROM item_lookup WHERE code = ? LIMIT 1", (code,))
            result = cursor.fetchone()
            if result:
                return {'code': result['code'], 'name': result['name'], 'category': result['category']}
            return None
        except sqlite3.Error:
            return None
    
    def close(self):
        if self.conn:
            self.conn.close()
            self.conn = None

class EnhancedD2Extractor:
    def __init__(self, save_file, output_dir='extracted_items', db_path='d2_items.db'):
        self.save_file = save_file
        self.output_dir = Path(output_dir)
        self.db_path = db_path
        self.lookup = None
        self.extracted_count = 0
        
        # Initialize item lookup
        if D2ItemLookup and os.path.exists(db_path):
            self.lookup = D2ItemLookup(db_path)
            print(f"Using item database: {db_path}")
        else:
            print(f"Database not found at {db_path}, using generic names")
    
    def get_item_name(self, code):
        """Get proper item name from database"""
        if self.lookup:
            result = self.lookup.lookup_item(code)
            if result:
                # Clean up name (remove color codes)
                name = result['name']
                name = name.replace('\\purple;', '').replace('\\orange;', '').replace('\\gold;', '')
                name = name.replace('\\green;', '').replace('\\blue;', '').replace('\\red;', '')
                return name, result['category']
        
        return f"Unknown Item ({code})", 'unknown'
    
    def categorize_item_by_code(self, code):
        """Categorize item by code patterns"""
        if not code:
            return 'unknown'
            
        code_lower = code.lower()
        
        # Gems
        if code_lower.startswith(('gc', 'gf', 'gs', 'gl', 'gp')):
            return 'gems'
        
        # Runes
        if code_lower.startswith('r') and len(code) == 3 and code[1:].isdigit():
            return 'runes'
        
        # Jewels
        if code_lower == 'jew':
            return 'jewels'
        
        # Charms
        if code_lower.startswith('cm'):
            return 'charms'
        
        # Potions
        if code_lower.startswith(('hp', 'mp')) and code_lower[-1:].isdigit():
            return 'potions'
        
        # Scrolls
        if code_lower in ('tsc', 'isc'):
            return 'scrolls'
        
        return 'items'
    
    def extract_items_enhanced(self, start_offset, item_size=14, max_items=50):
        """Enhanced extraction with database lookup"""
        
        self.output_dir.mkdir(exist_ok=True)
        
        try:
            with open(self.save_file, 'rb') as f:
                # Get file size
                f.seek(0, 2)
                file_size = f.tell()
                
                print(f"File: {self.save_file} ({file_size} bytes)")
                print(f"Starting at offset: {start_offset}")
                print(f"Item size: {item_size} bytes")
                print(f"Max items: {max_items}")
                print()
                
                # Seek to starting position
                f.seek(start_offset)
                
                category_counts = {}
                
                for item_num in range(max_items):
                    current_pos = f.tell()
                    
                    # Check if we're near end of file
                    if current_pos + item_size + 7 >= file_size:
                        print(f"Reached end of file at position {current_pos}")
                        break
                    
                    # Skip page header (7 bytes)
                    f.seek(7, 1)
                    
                    # Read item data
                    item_data = f.read(item_size)
                    
                    if len(item_data) < item_size:
                        print(f"Not enough data for item {item_num + 1}")
                        break
                    
                    # Process item
                    if len(item_data) >= 6 and item_data[:2] == b'JM':
                        # Extract item type
                        item_type_raw = item_data[2:6]
                        try:
                            # Clean up item code
                            item_code = item_type_raw.decode('ascii', errors='ignore')
                            item_code = ''.join(c for c in item_code if c.isprintable() and c != '\x00')
                            
                            if not item_code:
                                item_code = 'unknown'
                            
                            # Get proper name and category from database
                            item_name, db_category = self.get_item_name(item_code)
                            
                            # Fallback to code-based categorization if database doesn't have it
                            if db_category == 'unknown':
                                category = self.categorize_item_by_code(item_code)
                            else:
                                category = db_category
                            
                            # Count items per category
                            if category not in category_counts:
                                category_counts[category] = 0
                            category_counts[category] += 1
                            
                            # Generate filename with proper name
                            safe_name = ''.join(c for c in item_name if c.isalnum() or c in ' -_').strip()
                            safe_name = safe_name.replace(' ', '_')
                            
                            if len(safe_name) > 30:  # Limit filename length
                                safe_name = safe_name[:30]
                            
                            filename = f"{category}_{safe_name}_{category_counts[category]:03d}.d2i"
                            filepath = self.output_dir / filename
                            
                            # Write item
                            with open(filepath, 'wb') as out_f:
                                out_f.write(item_data)
                            
                            print(f"Extracted: {filename}")
                            print(f"  Code: {item_code} -> Name: {item_name}")
                            self.extracted_count += 1
                            
                        except Exception as e:
                            print(f"Error processing item {item_num + 1}: {e}")
                            continue
                    
                    else:
                        # Handle data without JM signature
                        if any(item_data):  # Not all zeros
                            category = 'unknown'
                            if category not in category_counts:
                                category_counts[category] = 0
                            category_counts[category] += 1
                            
                            filename = f"data_{category_counts[category]:03d}.d2i"
                            filepath = self.output_dir / filename
                            
                            with open(filepath, 'wb') as out_f:
                                out_f.write(item_data)
                            
                            print(f"Extracted (raw data): {filename}")
                            self.extracted_count += 1
                        else:
                            print(f"Item {item_num + 1}: Empty data, stopping")
                            break
                
                print(f"\nExtraction Summary:")
                print(f"Total items extracted: {self.extracted_count}")
                print(f"Output directory: {self.output_dir}")
                
                if category_counts:
                    print("\nItems by category:")
                    for category, count in sorted(category_counts.items()):
                        print(f"  {category}: {count} items")
                
        except Exception as e:
            print(f"Error: {e}")
            return False
        
        finally:
            if self.lookup:
                self.lookup.close()
        
        return self.extracted_count > 0

def suggest_offsets_enhanced(save_file, db_path='d2_items.db'):
    """Enhanced offset suggestion with database lookup"""
    
    print(f"Analyzing {save_file} for possible item offsets...")
    
    # Initialize lookup for better item identification
    lookup = None
    if D2ItemLookup and os.path.exists(db_path):
        lookup = D2ItemLookup(db_path)
    
    with open(save_file, 'rb') as f:
        f.seek(0, 2)
        file_size = f.tell()
        f.seek(0)
        
        # Look for JM signatures
        jm_positions = []
        data = f.read()
        pos = 0
        
        while pos < len(data) - 10:
            if data[pos:pos+2] == b'JM':
                # Check if followed by reasonable item type
                type_bytes = data[pos+2:pos+6]
                try:
                    type_str = type_bytes.decode('ascii', errors='ignore').rstrip('\x00')
                    if len(type_str) >= 2:
                        # Try to get proper name if we have database
                        if lookup:
                            result = lookup.lookup_item(type_str)
                            if result:
                                name = result['name'].replace('\\purple;', '').replace('\\orange;', '')
                                jm_positions.append((pos, type_str, name))
                            else:
                                jm_positions.append((pos, type_str, f"Unknown ({type_str})"))
                        else:
                            jm_positions.append((pos, type_str, f"Code: {type_str}"))
                except:
                    pass
            pos += 1
        
        if lookup:
            lookup.close()
        
        if jm_positions:
            print(f"\nFound {len(jm_positions)} potential JM signatures:")
            
            # Group nearby positions
            groups = []
            current_group = [jm_positions[0]]
            
            for i in range(1, len(jm_positions)):
                pos, type_str, name = jm_positions[i]
                prev_pos = current_group[-1][0]
                
                # If positions are close (within 200 bytes), group them
                if pos - prev_pos <= 200:
                    current_group.append(jm_positions[i])
                else:
                    groups.append(current_group)
                    current_group = [jm_positions[i]]
            
            if current_group:
                groups.append(current_group)
            
            print(f"\nGrouped into {len(groups)} potential item sections:")
            
            for i, group in enumerate(groups):
                start_pos = group[0][0]
                end_pos = group[-1][0]
                count = len(group)
                
                print(f"  Group {i+1}: Offset {start_pos}-{end_pos} ({count} items)")
                
                # Show first few items in group with names
                for j, (pos, type_str, name) in enumerate(group[:5]):
                    print(f"    {pos}: {type_str} -> {name}")
                    
                if len(group) > 5:
                    print(f"    ... and {len(group) - 5} more")
                
                print(f"    Suggested command:")
                print(f"      python3 {sys.argv[0]} {save_file} extracted_items {start_pos}")
                print()

def create_test_items(output_dir='test_items'):
    """Create test .d2i files for demonstration"""
    print("Creating test .d2i files...")
    
    test_dir = Path(output_dir)
    test_dir.mkdir(exist_ok=True)
    
    # Test items with proper JM structure
    test_items = [
        ('gcr', 'chipped_ruby'),
        ('gfr', 'flawed_ruby'),
        ('gsr', 'ruby'),
        ('r01', 'el_rune'),
        ('r02', 'eld_rune'),
        ('r33', 'zod_rune'),
        ('jew', 'jewel'),
        ('cm1', 'small_charm'),
        ('hp1', 'minor_healing_potion'),
        ('tsc', 'town_portal_scroll'),
    ]
    
    created_files = []
    
    for i, (code, name) in enumerate(test_items, 1):
        # Create proper .d2i structure: JM + 4-byte code + padding
        code_bytes = code.ljust(4, '\x00')[:4].encode('ascii')
        item_data = b'JM' + code_bytes + b'\x00' * 10  # 16 bytes total
        
        filename = f"item_{i:03d}_{name}.d2i"
        filepath = test_dir / filename
        
        with open(filepath, 'wb') as f:
            f.write(item_data)
        
        created_files.append(filepath)
        print(f"  Created: {filename} (code: {code})")
    
    print(f"Created {len(created_files)} test files in {test_dir}/")
    return test_dir

def rename_extracted_items(input_dir, output_dir=None, db_path='d2_items.db'):
    """Rename extracted .d2i files with proper names"""
    input_path = Path(input_dir)
    if not input_path.exists():
        print(f"Error: Input directory not found: {input_dir}")
        return False
    
    if output_dir:
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True)
    else:
        output_path = input_path / 'renamed'
        output_path.mkdir(exist_ok=True)
    
    if not os.path.exists(db_path):
        print(f"Error: Database not found at {db_path}")
        return False
    
    print(f"Renaming .d2i files from {input_path} to {output_path}")
    
    db = D2ItemLookup(db_path)
    if not db.conn:
        print("Could not connect to database")
        return False
    
    d2i_files = list(input_path.glob('*.d2i'))
    if not d2i_files:
        print("No .d2i files found!")
        return False
    
    category_counts = {}
    renamed_count = 0
    
    for d2i_file in d2i_files:
        try:
            with open(d2i_file, 'rb') as f:
                data = f.read()
            
            if len(data) < 6:
                continue
            
            if data[:2] == b'JM':
                item_code = data[2:6].decode('ascii', errors='ignore').rstrip('\x00')
                
                if item_code:
                    result = db.lookup_item(item_code)
                    
                    if result:
                        # Clean name for filename
                        clean_name = result['name']
                        for color_code in ['\\purple;', '\\orange;', '\\gold;', '\\green;', '\\blue;', '\\red;']:
                            clean_name = clean_name.replace(color_code, '')
                        clean_name = clean_name.strip()
                        
                        safe_name = ''.join(c for c in clean_name if c.isalnum() or c in ' -_().[]')
                        safe_name = safe_name.replace(' ', '_')
                        
                        if len(safe_name) > 40:
                            safe_name = safe_name[:40]
                        
                        category = result['category']
                        
                        if category not in category_counts:
                            category_counts[category] = 0
                        category_counts[category] += 1
                        
                        new_filename = f"{category}_{safe_name}_{category_counts[category]:03d}.d2i"
                        new_filepath = output_path / new_filename
                        
                        shutil.copy2(d2i_file, new_filepath)
                        
                        print(f"✓ {d2i_file.name} → {new_filename}")
                        print(f"  Code: {item_code} → Name: {clean_name}")
                        renamed_count += 1
                    else:
                        # Unknown item
                        new_filename = f"unknown_{item_code}_{len(list(output_path.glob('unknown_*.d2i'))) + 1:03d}.d2i"
                        new_filepath = output_path / new_filename
                        shutil.copy2(d2i_file, new_filepath)
                        print(f"? {d2i_file.name} → {new_filename} (unknown)")
                        renamed_count += 1
            else:
                # No JM signature
                new_filename = f"data_{len(list(output_path.glob('data_*.d2i'))) + 1:03d}.d2i"
                new_filepath = output_path / new_filename
                shutil.copy2(d2i_file, new_filepath)
                print(f"? {d2i_file.name} → {new_filename} (no signature)")
                renamed_count += 1
                
        except Exception as e:
            print(f"✗ Error processing {d2i_file.name}: {e}")
    
    db.close()
    
    print(f"\nRenaming complete: {renamed_count}/{len(d2i_files)} files processed")
    if category_counts:
        print("Items by category:")
        for category, count in sorted(category_counts.items()):
            print(f"  {category}: {count} items")
    
    return True

def demo_complete_workflow():
    """Complete demonstration workflow"""
    print("=" * 60)
    print("COMPLETE D2 ITEM EXTRACTION & DATABASE DEMO")
    print("=" * 60)
    
    # Step 1: Build database
    print("\n1. Building Essential Item Database...")
    builder = D2ItemDatabaseBuilder('', 'd2_items_quick.db')
    if builder.build_quick_database():
        print("✓ Database built successfully")
    else:
        print("✗ Database build failed")
        return False
    
    # Step 2: Create test items
    print("\n2. Creating Test .d2i Files...")
    test_dir = create_test_items('demo_test_items')
    
    # Step 3: Rename with database
    print("\n3. Renaming Files with Database Lookup...")
    success = rename_extracted_items('demo_test_items', 'demo_renamed_items', 'd2_items_quick.db')
    
    if success:
        print(f"\n🎉 DEMO COMPLETE!")
        print("✓ Database created with essential items")
        print("✓ Test .d2i files created with proper structure")
        print("✓ Files renamed with proper names from database")
        print(f"\nCheck demo_renamed_items/ for results!")
        
        # Show final results
        renamed_files = list(Path('demo_renamed_items').glob('*.d2i'))
        print(f"\nFinal Results ({len(renamed_files)} files):")
        for filepath in sorted(renamed_files)[:10]:  # Show first 10
            print(f"  {filepath.name}")
        if len(renamed_files) > 10:
            print(f"  ... and {len(renamed_files) - 10} more")
            
    return success

def main():
    if len(sys.argv) < 2:
        print("Complete D2 Item Extractor with Built-in Database")
        print()
        print("Usage:")
        print(f"  {sys.argv[0]} --demo                    # Run complete demo")
        print(f"  {sys.argv[0]} --build-db [tsv_dir]     # Build database from TSV files")
        print(f"  {sys.argv[0]} --create-test            # Create test .d2i files")
        print(f"  {sys.argv[0]} --rename <dir>          # Rename .d2i files in directory")
        print(f"  {sys.argv[0]} --suggest <save_file>   # Analyze save file")
        print(f"  {sys.argv[0]} <save_file> [options]   # Extract from save file")
        print()
        print("Extract Options:")
        print("  <save_file> [output_dir] [start_offset] [item_size] [max_items]")
        print()
        print("Examples:")
        print(f"  {sys.argv[0]} --demo                    # Complete demonstration")
        print(f"  {sys.argv[0]} --create-test            # Create test files")
        print(f"  {sys.argv[0]} --rename test_items/     # Rename test files")
        print(f"  {sys.argv[0]} --suggest character.d2s  # Analyze save file")
        print(f"  {sys.argv[0]} character.d2s extracted_items 957 50 20")
        return 1
    
    command = sys.argv[1]
    
    # Handle special commands
    if command == '--demo':
        success = demo_complete_workflow()
        return 0 if success else 1
    
    elif command == '--build-db':
        tsv_dir = sys.argv[2] if len(sys.argv) > 2 else 'utils/txt_parser/'
        builder = D2ItemDatabaseBuilder(tsv_dir)
        success = builder.build_quick_database()
        return 0 if success else 1
    
    elif command == '--create-test':
        create_test_items()
        print("Test files created! Use --rename test_items/ to rename them.")
        return 0
    
    elif command == '--rename':
        if len(sys.argv) < 3:
            print("Usage: --rename <input_directory> [output_directory]")
            return 1
        input_dir = sys.argv[2]
        output_dir = sys.argv[3] if len(sys.argv) > 3 else None
        
        # Build database if it doesn't exist
        if not os.path.exists('d2_items.db') and not os.path.exists('d2_items_quick.db'):
            print("Building quick database first...")
            builder = D2ItemDatabaseBuilder('', 'd2_items_quick.db')
            builder.build_quick_database()
            db_path = 'd2_items_quick.db'
        else:
            db_path = 'd2_items.db' if os.path.exists('d2_items.db') else 'd2_items_quick.db'
        
        success = rename_extracted_items(input_dir, output_dir, db_path)
        return 0 if success else 1
    
    elif command == '--suggest':
        if len(sys.argv) < 3:
            print("Usage: --suggest <save_file>")
            return 1
        save_file = sys.argv[2]
        
        # Build database if needed
        if not os.path.exists('d2_items.db') and not os.path.exists('d2_items_quick.db'):
            builder = D2ItemDatabaseBuilder('', 'd2_items_quick.db')
            builder.build_quick_database()
            db_path = 'd2_items_quick.db'
        else:
            db_path = 'd2_items.db' if os.path.exists('d2_items.db') else 'd2_items_quick.db'
        
        suggest_offsets_enhanced(save_file, db_path)
        return 0
    
    # Regular extraction
    save_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else 'extracted_items'
    start_offset = int(sys.argv[3]) if len(sys.argv) > 3 else None
    item_size = int(sys.argv[4]) if len(sys.argv) > 4 else 50
    max_items = int(sys.argv[5]) if len(sys.argv) > 5 else 50
    
    if not os.path.exists(save_file):
        print(f"Error: File not found: {save_file}")
        return 1
    
    # Build database if needed
    if not os.path.exists('d2_items.db') and not os.path.exists('d2_items_quick.db'):
        print("Building database first...")
        builder = D2ItemDatabaseBuilder('', 'd2_items_quick.db')
        builder.build_quick_database()
        db_path = 'd2_items_quick.db'
    else:
        db_path = 'd2_items.db' if os.path.exists('d2_items.db') else 'd2_items_quick.db'
    
    # Auto-suggest if no offset provided
    if start_offset is None:
        print("No offset provided, analyzing file...")
        suggest_offsets_enhanced(save_file, db_path)
        return 0
    
    # Extract items
    extractor = EnhancedD2Extractor(save_file, output_dir, db_path)
    success = extractor.extract_items_enhanced(start_offset, item_size, max_items)
    
    if success:
        print("\nExtraction completed successfully!")
        print(f"Check {output_dir}/ for extracted .d2i files with proper names")
    else:
        print("Extraction failed!")
    
    return 0 if success else 1

if __name__ == '__main__':
    sys.exit(main())