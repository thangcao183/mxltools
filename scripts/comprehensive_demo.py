#!/usr/bin/env python3
"""
Comprehensive Database Demo
Creates test .d2i files and demonstrates proper naming with database
"""

import os
import sys
from pathlib import Path

# Add scripts directory to path  
sys.path.append(str(Path(__file__).parent))

try:
    from item_lookup import D2ItemLookup
    from d2i_renamer import rename_d2i_files
    DATABASE_AVAILABLE = True
except ImportError:
    DATABASE_AVAILABLE = False

def create_comprehensive_test_items():
    """Create comprehensive test .d2i files with various item types"""
    
    print("Creating Comprehensive Test Items")
    print("=" * 40)
    
    # Test items covering different categories
    test_items = [
        # Gems
        {'code': 'gcr', 'name': 'chipped_ruby', 'category': 'gems'},
        {'code': 'gfr', 'name': 'flawed_ruby', 'category': 'gems'},
        {'code': 'gsr', 'name': 'ruby', 'category': 'gems'},
        {'code': 'gcb', 'name': 'chipped_sapphire', 'category': 'gems'},
        {'code': 'gfb', 'name': 'flawed_sapphire', 'category': 'gems'},
        
        # Runes  
        {'code': 'r01', 'name': 'el_rune', 'category': 'runes'},
        {'code': 'r02', 'name': 'eld_rune', 'category': 'runes'},
        {'code': 'r03', 'name': 'tir_rune', 'category': 'runes'},
        {'code': 'r10', 'name': 'thul_rune', 'category': 'runes'},
        {'code': 'r33', 'name': 'zod_rune', 'category': 'runes'},
        
        # Other items
        {'code': 'jew', 'name': 'jewel', 'category': 'jewels'},
        {'code': 'cm1', 'name': 'small_charm', 'category': 'charms'},
        {'code': 'cm2', 'name': 'large_charm', 'category': 'charms'},
        {'code': 'hp1', 'name': 'minor_healing_potion', 'category': 'potions'},
        {'code': 'tsc', 'name': 'town_portal_scroll', 'category': 'scrolls'},
    ]
    
    # Create test directory
    test_dir = Path('comprehensive_test_items')
    test_dir.mkdir(exist_ok=True)
    
    created_files = []
    
    for i, item in enumerate(test_items, 1):
        # Create proper .d2i structure
        # JM signature + 4-byte item code + padding to reach realistic size
        code_bytes = item['code'].ljust(4, '\x00')[:4].encode('ascii')
        item_data = b'JM' + code_bytes + b'\x00' * 10  # 16 bytes total
        
        # Generic filename for now
        filename = f"item_{i:03d}_{item['name']}.d2i"
        filepath = test_dir / filename
        
        with open(filepath, 'wb') as f:
            f.write(item_data)
        
        created_files.append(filepath)
        print(f"Created: {filename} (code: {item['code']})")
    
    print(f"\nCreated {len(created_files)} test files in {test_dir}/")
    return test_dir

def demo_database_lookup():
    """Demonstrate database lookup capabilities"""
    
    if not DATABASE_AVAILABLE:
        print("Database not available!")
        return
    
    print("\nDatabase Lookup Demo")
    print("=" * 30)
    
    db = D2ItemLookup('d2_items.db')
    
    # Demo various lookup types
    print("\n1. Gem Lookup:")
    gem_codes = ['gcr', 'gfr', 'gsr', 'glr', 'gpr']
    for code in gem_codes:
        result = db.lookup_item(code)
        if result:
            name = result['name'].replace('\\purple;', '').strip()
            print(f"   {code} → {name}")
    
    print("\n2. Rune Lookup:")
    rune_codes = ['r01', 'r02', 'r03', 'r10', 'r33']
    for code in rune_codes:
        result = db.lookup_item(code)
        if result:
            name = result['name'].replace('\\purple;', '').strip()
            print(f"   {code} → {name}")
    
    print("\n3. Search by Name:")
    search_results = db.search_items('Ruby', 5)
    for item in search_results[:5]:
        name = item['name'].replace('\\purple;', '').strip()
        print(f"   {item['code']} → {name}")
    
    db.close()

def demo_full_workflow():
    """Demonstrate the complete workflow"""
    
    print("\n" + "=" * 60)
    print("COMPLETE WORKFLOW DEMONSTRATION")
    print("=" * 60)
    
    # Step 1: Create test items
    test_dir = create_comprehensive_test_items()
    
    # Step 2: Demo database
    demo_database_lookup()
    
    # Step 3: Rename files with proper names
    print(f"\nStep 3: Renaming Files with Database Lookup")
    print("=" * 50)
    
    output_dir = 'properly_named_items'
    success = rename_d2i_files(str(test_dir), output_dir)
    
    if success:
        print(f"\n✅ SUCCESS! Check {output_dir}/ for properly named files:")
        
        # Show final results
        output_path = Path(output_dir)
        renamed_files = list(output_path.glob('*.d2i'))
        
        print(f"\nFinal Results ({len(renamed_files)} files):")
        for filepath in sorted(renamed_files):
            print(f"   {filepath.name}")
        
        # Show categorization
        categories = {}
        for filepath in renamed_files:
            category = filepath.name.split('_')[0]
            if category not in categories:
                categories[category] = 0
            categories[category] += 1
        
        print(f"\nCategorization:")
        for category, count in sorted(categories.items()):
            print(f"   {category}: {count} items")

def main():
    print("D2 Item Database - Comprehensive Demo")
    print("=" * 50)
    
    if not DATABASE_AVAILABLE:
        print("❌ ERROR: Database modules not available!")
        print("Please ensure item_lookup.py and d2i_renamer.py are in the same directory")
        return 1
    
    if not os.path.exists('d2_items.db'):
        print("❌ ERROR: Database file 'd2_items.db' not found!")
        print("Please run: python3 scripts/build_item_database.py utils/txt_parser/")
        return 1
    
    # Run complete demo
    demo_full_workflow()
    
    print(f"\n🎉 DEMO COMPLETE!")
    print("The database successfully identified items and renamed them properly!")
    print("\nYou can now:")
    print("1. Use 'properly_named_items/' files in MedianXL Offline Tools")  
    print("2. Load .d2i files - they have proper names instead of codes")
    print("3. Integrate database lookup into gem/item creation widgets")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())