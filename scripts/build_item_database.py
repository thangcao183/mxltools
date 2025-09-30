#!/usr/bin/env python3
"""
D2 Item Database Builder
Parses TSV files from utils/txt_parser/ and creates SQLite database for item lookup
"""

import sqlite3
import csv
import os
import sys
from pathlib import Path

class D2ItemDatabaseBuilder:
    def __init__(self, tsv_dir, db_path='d2_items.db'):
        self.tsv_dir = Path(tsv_dir)
        self.db_path = db_path
        self.conn = None
        
    def create_connection(self):
        """Create SQLite connection"""
        try:
            self.conn = sqlite3.connect(self.db_path)
            self.conn.execute("PRAGMA foreign_keys = ON")
            print(f"Connected to SQLite database: {self.db_path}")
            return True
        except sqlite3.Error as e:
            print(f"Error connecting to database: {e}")
            return False
    
    def create_tables(self):
        """Create database tables"""
        
        # Items table (from generated/en/items.tsv)
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS items (
                code TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                width INTEGER,
                height INTEGER,
                gentype INTEGER,
                stackable TEXT,
                rlvl INTEGER,
                rstr INTEGER,
                rdex INTEGER,
                type TEXT,
                sockettype TEXT,
                class INTEGER
            )
        ''')
        
        # Gems table (from txt/gems.tsv)
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS gems (
                code TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                letter TEXT,
                transform INTEGER,
                nummods INTEGER,
                weaponMod1Code TEXT,
                weaponMod1Min INTEGER,
                weaponMod1Max INTEGER,
                helmMod1Code TEXT,
                helmMod1Min INTEGER,
                helmMod1Max INTEGER,
                shieldMod1Code TEXT,
                shieldMod1Min INTEGER,
                shieldMod1Max INTEGER
            )
        ''')
        
        # Runes table (from txt/runes.tsv)
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS runes (
                name TEXT PRIMARY KEY,
                complete INTEGER,
                status TEXT,
                rune1 TEXT,
                rune2 TEXT,
                rune3 TEXT,
                rune4 TEXT,
                rune5 TEXT,
                rune6 TEXT,
                server INTEGER
            )
        ''')
        
        # Individual rune items
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS rune_items (
                code TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                number INTEGER
            )
        ''')
        
        # Unique items table
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS uniques (
                index_id INTEGER PRIMARY KEY,
                item TEXT,
                rlvl INTEGER,
                ilvl INTEGER,
                image TEXT
            )
        ''')
        
        # Set items table
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS setitems (
                index_id TEXT PRIMARY KEY,
                item TEXT,
                set_name TEXT,
                rlvl INTEGER,
                ilvl INTEGER
            )
        ''')
        
        # Item lookup view for quick searches
        self.conn.execute('''
            CREATE VIEW IF NOT EXISTS item_lookup AS
            SELECT code, name, 'item' as category FROM items
            UNION ALL
            SELECT code, name, 'gem' as category FROM gems
            UNION ALL
            SELECT name as code, name, 'runeword' as category FROM runes
            UNION ALL
            SELECT code, name, 'rune' as category FROM rune_items
        ''')
        
        self.conn.commit()
        print("Database tables created successfully")
    
    def parse_items_tsv(self):
        """Parse items.tsv file"""
        items_file = self.tsv_dir / 'generated' / 'en' / 'items.tsv'
        
        if not items_file.exists():
            print(f"Items file not found: {items_file}")
            return
            
        print(f"Parsing items from: {items_file}")
        
        with open(items_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f, delimiter='\t')
            
            items_data = []
            for row in reader:
                if row['#code'].startswith('#'):  # Skip comment lines
                    continue
                    
                items_data.append((
                    row['#code'],
                    row['name'],
                    int(row['width']) if row['width'] else 0,
                    int(row['height']) if row['height'] else 0,
                    int(row['gentype']) if row['gentype'] else 0,
                    row['stackable'],
                    int(row['rlvl']) if row['rlvl'] else 0,
                    int(row['rstr']) if row['rstr'] else 0,
                    int(row['rdex']) if row['rdex'] else 0,
                    row['type'],
                    row['sockettype'],
                    int(row['class']) if row['class'] else 0
                ))
            
            # Insert data
            self.conn.executemany('''
                INSERT OR REPLACE INTO items 
                (code, name, width, height, gentype, stackable, rlvl, rstr, rdex, type, sockettype, class)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', items_data)
            
            print(f"Inserted {len(items_data)} items")
    
    def parse_gems_tsv(self):
        """Parse gems.tsv file"""
        gems_file = self.tsv_dir / 'txt' / 'gems.tsv'
        
        if not gems_file.exists():
            print(f"Gems file not found: {gems_file}")
            return
            
        print(f"Parsing gems from: {gems_file}")
        
        with open(gems_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f, delimiter='\t')
            
            gems_data = []
            for row in reader:
                gems_data.append((
                    row['code'],
                    row['name'],
                    row['letter'],
                    int(row['transform']) if row['transform'] else 0,
                    int(row['nummods']) if row['nummods'] else 0,
                    row['weaponMod1Code'],
                    int(row['weaponMod1Min']) if row['weaponMod1Min'] else 0,
                    int(row['weaponMod1Max']) if row['weaponMod1Max'] else 0,
                    row['helmMod1Code'],
                    int(row['helmMod1Min']) if row['helmMod1Min'] else 0,
                    int(row['helmMod1Max']) if row['helmMod1Max'] else 0,
                    row['shieldMod1Code'],
                    int(row['shieldMod1Min']) if row['shieldMod1Min'] else 0,
                    int(row['shieldMod1Max']) if row['shieldMod1Max'] else 0
                ))
            
            # Insert data
            self.conn.executemany('''
                INSERT OR REPLACE INTO gems 
                (code, name, letter, transform, nummods, weaponMod1Code, weaponMod1Min, weaponMod1Max,
                 helmMod1Code, helmMod1Min, helmMod1Max, shieldMod1Code, shieldMod1Min, shieldMod1Max)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', gems_data)
            
            print(f"Inserted {len(gems_data)} gems")
    
    def parse_runes_tsv(self):
        """Parse runes.tsv file"""
        runes_file = self.tsv_dir / 'txt' / 'runes.tsv'
        
        if not runes_file.exists():
            print(f"Runes file not found: {runes_file}")
            return
            
        print(f"Parsing runes from: {runes_file}")
        
        with open(runes_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f, delimiter='\t')
            
            runes_data = []
            for row in reader:
                runes_data.append((
                    row['Name'],
                    int(row['complete']) if row['complete'] else 0,
                    row['* status'],
                    row['Rune1'],
                    row['Rune2'],
                    row['Rune3'],
                    row['Rune4'],
                    row['Rune5'],
                    row['Rune6'],
                    int(row['server']) if row['server'] else 0
                ))
            
            # Insert data
            self.conn.executemany('''
                INSERT OR REPLACE INTO runes 
                (name, complete, status, rune1, rune2, rune3, rune4, rune5, rune6, server)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', runes_data)
            
            print(f"Inserted {len(runes_data)} runewords")
            
        # Add individual rune items (r01-r33)
        rune_items = []
        rune_names = [
            "El", "Eld", "Tir", "Nef", "Eth", "Ith", "Tal", "Ral", "Ort", "Thul",
            "Amn", "Sol", "Shael", "Dol", "Hel", "Io", "Lum", "Ko", "Fal", "Lem",
            "Pul", "Um", "Mal", "Ist", "Gul", "Vex", "Ohm", "Lo", "Sur", "Ber",
            "Jah", "Cham", "Zod"
        ]
        
        for i, name in enumerate(rune_names, 1):
            code = f"r{i:02d}"
            rune_items.append((code, f"{name} Rune", i))
        
        self.conn.executemany('''
            INSERT OR REPLACE INTO rune_items (code, name, number)
            VALUES (?, ?, ?)
        ''', rune_items)
        
        print(f"Inserted {len(rune_items)} individual runes")
    
    def parse_uniques_tsv(self):
        """Parse uniques.tsv file"""
        uniques_file = self.tsv_dir / 'generated' / 'en' / 'uniques.tsv'
        
        if not uniques_file.exists():
            print(f"Uniques file not found: {uniques_file}")
            return
            
        print(f"Parsing uniques from: {uniques_file}")
        
        with open(uniques_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f, delimiter='\t')
            
            uniques_data = []
            for row in reader:
                if row['#index'].startswith('#'):  # Skip comment lines
                    continue
                    
                uniques_data.append((
                    int(row['#index']) if row['#index'] else 0,
                    row['item'],
                    int(row['rlvl']) if row['rlvl'] else 0,
                    int(row['ilvl']) if row['ilvl'] else 0,
                    row['image']
                ))
            
            # Insert data
            self.conn.executemany('''
                INSERT OR REPLACE INTO uniques 
                (index_id, item, rlvl, ilvl, image)
                VALUES (?, ?, ?, ?, ?)
            ''', uniques_data)
            
            print(f"Inserted {len(uniques_data)} unique items")
    
    def parse_setitems_tsv(self):
        """Parse setitems.tsv file"""
        setitems_file = self.tsv_dir / 'generated' / 'en' / 'setitems.tsv'
        
        if not setitems_file.exists():
            print(f"Set items file not found: {setitems_file}")
            return
            
        print(f"Parsing set items from: {setitems_file}")
        
        with open(setitems_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f, delimiter='\t')
            
            setitems_data = []
            for row in reader:
                if row.get('#index', '').startswith('#'):  # Skip comment lines
                    continue
                    
                setitems_data.append((
                    row.get('#index', ''),
                    row.get('item', ''),
                    row.get('set', ''),
                    int(row.get('rlvl', 0)) if row.get('rlvl') else 0,
                    int(row.get('ilvl', 0)) if row.get('ilvl') else 0
                ))
            
            # Insert data
            self.conn.executemany('''
                INSERT OR REPLACE INTO setitems 
                (index_id, item, set_name, rlvl, ilvl)
                VALUES (?, ?, ?, ?, ?)
            ''', setitems_data)
            
            print(f"Inserted {len(setitems_data)} set items")
    
    def build_database(self):
        """Main method to build the database"""
        print("Building D2 Item Database...")
        print(f"TSV Directory: {self.tsv_dir}")
        print(f"Database Path: {self.db_path}")
        print()
        
        if not self.create_connection():
            return False
        
        try:
            # Create tables
            self.create_tables()
            
            # Parse all TSV files
            self.parse_items_tsv()
            self.parse_gems_tsv()
            self.parse_runes_tsv()
            self.parse_uniques_tsv()
            self.parse_setitems_tsv()
            
            # Commit all changes
            self.conn.commit()
            
            # Show statistics
            self.show_statistics()
            
            print(f"\nDatabase built successfully: {self.db_path}")
            return True
            
        except Exception as e:
            print(f"Error building database: {e}")
            self.conn.rollback()
            return False
        
        finally:
            if self.conn:
                self.conn.close()
    
    def show_statistics(self):
        """Show database statistics"""
        print("\nDatabase Statistics:")
        
        tables = ['items', 'gems', 'runes', 'rune_items', 'uniques', 'setitems']
        
        for table in tables:
            try:
                cursor = self.conn.execute(f"SELECT COUNT(*) FROM {table}")
                count = cursor.fetchone()[0]
                print(f"  {table}: {count} records")
            except sqlite3.Error as e:
                print(f"  {table}: Error - {e}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 build_item_database.py <tsv_directory> [database_path]")
        print()
        print("Example:")
        print("  python3 build_item_database.py utils/txt_parser/")
        print("  python3 build_item_database.py utils/txt_parser/ d2_items.db")
        return 1
    
    tsv_dir = sys.argv[1]
    db_path = sys.argv[2] if len(sys.argv) > 2 else 'd2_items.db'
    
    if not os.path.exists(tsv_dir):
        print(f"Error: TSV directory not found: {tsv_dir}")
        return 1
    
    # Remove existing database
    if os.path.exists(db_path):
        os.remove(db_path)
        print(f"Removed existing database: {db_path}")
    
    # Build database
    builder = D2ItemDatabaseBuilder(tsv_dir, db_path)
    success = builder.build_database()
    
    return 0 if success else 1

if __name__ == '__main__':
    sys.exit(main())