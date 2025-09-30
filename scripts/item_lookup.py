#!/usr/bin/env python3
"""
D2 Item Lookup Library
Uses SQLite database to lookup item names by code
"""

import sqlite3
import os
from pathlib import Path

class D2ItemLookup:
    def __init__(self, db_path='d2_items.db'):
        self.db_path = db_path
        self.conn = None
        self._connect()
        
    def _connect(self):
        """Connect to SQLite database"""
        if not os.path.exists(self.db_path):
            print(f"Warning: Database not found at {self.db_path}")
            print("Run build_item_database.py first to create the database")
            return False
            
        try:
            self.conn = sqlite3.connect(self.db_path)
            self.conn.row_factory = sqlite3.Row  # Enable column access by name
            return True
        except sqlite3.Error as e:
            print(f"Error connecting to database: {e}")
            return False
    
    def lookup_item(self, code):
        """
        Lookup item name by code
        Returns dict with name, category, and additional info
        """
        if not self.conn:
            return None
        
        try:
            # Try exact match first
            cursor = self.conn.execute(
                "SELECT code, name, category FROM item_lookup WHERE code = ? LIMIT 1",
                (code,)
            )
            result = cursor.fetchone()
            
            if result:
                return {
                    'code': result['code'],
                    'name': result['name'],
                    'category': result['category']
                }
            
            # Try partial match (for codes with special characters)
            cursor = self.conn.execute(
                "SELECT code, name, category FROM item_lookup WHERE code LIKE ? LIMIT 1",
                (f"%{code}%",)
            )
            result = cursor.fetchone()
            
            if result:
                return {
                    'code': result['code'],
                    'name': result['name'],
                    'category': result['category']
                }
            
            return None
            
        except sqlite3.Error as e:
            print(f"Database error: {e}")
            return None
    
    def lookup_gem(self, code):
        """Lookup gem details by code"""
        if not self.conn:
            return None
            
        try:
            cursor = self.conn.execute(
                "SELECT * FROM gems WHERE code = ? LIMIT 1",
                (code,)
            )
            result = cursor.fetchone()
            
            if result:
                return dict(result)
            return None
            
        except sqlite3.Error as e:
            print(f"Database error: {e}")
            return None
    
    def lookup_rune(self, code):
        """Lookup rune details by code"""
        if not self.conn:
            return None
            
        try:
            cursor = self.conn.execute(
                "SELECT * FROM rune_items WHERE code = ? LIMIT 1",
                (code,)
            )
            result = cursor.fetchone()
            
            if result:
                return dict(result)
            return None
            
        except sqlite3.Error as e:
            print(f"Database error: {e}")
            return None
    
    def lookup_item_details(self, code):
        """Lookup detailed item information"""
        if not self.conn:
            return None
            
        try:
            cursor = self.conn.execute(
                "SELECT * FROM items WHERE code = ? LIMIT 1",
                (code,)
            )
            result = cursor.fetchone()
            
            if result:
                return dict(result)
            return None
            
        except sqlite3.Error as e:
            print(f"Database error: {e}")
            return None
    
    def search_items(self, search_term, limit=10):
        """Search items by name"""
        if not self.conn:
            return []
            
        try:
            cursor = self.conn.execute(
                "SELECT code, name, category FROM item_lookup WHERE name LIKE ? LIMIT ?",
                (f"%{search_term}%", limit)
            )
            results = cursor.fetchall()
            
            return [dict(row) for row in results]
            
        except sqlite3.Error as e:
            print(f"Database error: {e}")
            return []
    
    def get_stats(self):
        """Get database statistics"""
        if not self.conn:
            return {}
            
        stats = {}
        tables = ['items', 'gems', 'runes', 'rune_items', 'uniques', 'setitems']
        
        for table in tables:
            try:
                cursor = self.conn.execute(f"SELECT COUNT(*) as count FROM {table}")
                result = cursor.fetchone()
                stats[table] = result['count']
            except sqlite3.Error:
                stats[table] = 0
                
        return stats
    
    def close(self):
        """Close database connection"""
        if self.conn:
            self.conn.close()
            self.conn = None

# Convenience functions
def lookup_item_name(code, db_path='d2_items.db'):
    """Quick lookup of item name by code"""
    lookup = D2ItemLookup(db_path)
    result = lookup.lookup_item(code)
    lookup.close()
    
    if result:
        return result['name']
    return f"Unknown Item ({code})"

def get_item_info(code, db_path='d2_items.db'):
    """Get comprehensive item information"""
    lookup = D2ItemLookup(db_path)
    
    # Try different lookup methods
    info = lookup.lookup_item(code)
    if info and info['category'] == 'gem':
        details = lookup.lookup_gem(code)
        if details:
            info.update(details)
    elif info and info['category'] == 'rune':
        details = lookup.lookup_rune(code)
        if details:
            info.update(details)
    else:
        details = lookup.lookup_item_details(code)
        if details:
            info = details
    
    lookup.close()
    return info

# Test function
def test_lookup(db_path='d2_items.db'):
    """Test the lookup functionality"""
    lookup = D2ItemLookup(db_path)
    
    print("D2 Item Lookup Test")
    print("=" * 40)
    
    # Test gem lookup
    print("\nTesting Gem Lookup:")
    gem_codes = ['gcr', 'gfr', 'gsr', 'glr', 'gpr']  # Ruby progression
    for code in gem_codes:
        result = lookup.lookup_item(code)
        if result:
            print(f"  {code}: {result['name']} ({result['category']})")
        else:
            print(f"  {code}: Not found")
    
    # Test rune lookup
    print("\nTesting Rune Lookup:")
    rune_codes = ['r01', 'r02', 'r03', 'r10', 'r33']
    for code in rune_codes:
        result = lookup.lookup_item(code)
        if result:
            print(f"  {code}: {result['name']} ({result['category']})")
        else:
            print(f"  {code}: Not found")
    
    # Test search
    print("\nTesting Search:")
    search_results = lookup.search_items("Ruby", 5)
    for item in search_results:
        print(f"  {item['code']}: {item['name']} ({item['category']})")
    
    # Show stats
    print(f"\nDatabase Statistics:")
    stats = lookup.get_stats()
    for table, count in stats.items():
        print(f"  {table}: {count} items")
    
    lookup.close()

if __name__ == '__main__':
    import sys
    
    if len(sys.argv) > 1:
        if sys.argv[1] == '--test':
            db_path = sys.argv[2] if len(sys.argv) > 2 else 'd2_items.db'
            test_lookup(db_path)
        else:
            # Lookup specific item
            code = sys.argv[1]
            db_path = sys.argv[2] if len(sys.argv) > 2 else 'd2_items.db'
            
            result = get_item_info(code, db_path)
            if result:
                print(f"Item Code: {code}")
                print(f"Name: {result.get('name', 'Unknown')}")
                if 'category' in result:
                    print(f"Category: {result['category']}")
                if 'width' in result and 'height' in result:
                    print(f"Size: {result['width']}x{result['height']}")
                if 'type' in result:
                    print(f"Type: {result['type']}")
            else:
                print(f"Item not found: {code}")
    else:
        print("Usage:")
        print("  python3 item_lookup.py <item_code> [database_path]")
        print("  python3 item_lookup.py --test [database_path]")
        print()
        print("Examples:")
        print("  python3 item_lookup.py gcr")
        print("  python3 item_lookup.py r01")
        print("  python3 item_lookup.py --test")