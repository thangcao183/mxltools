#!/usr/bin/env python3
"""
Generate a SQLite database from items.tsv.
Usage:
  python3 scripts/generate_items_db.py [input_items.tsv] [output_db_path]
Defaults:
  input: utils/txt_parser/generated/en/items.tsv
  output: resources/data/items.db
"""
import sqlite3
import sys
import os

infile = sys.argv[1] if len(sys.argv) > 1 else os.path.join('utils', 'txt_parser', 'generated', 'en', 'items.tsv')
outfile = sys.argv[2] if len(sys.argv) > 2 else os.path.join('resources', 'data', 'items.db')

if not os.path.exists(infile):
    print('Input TSV not found:', infile)
    sys.exit(2)

os.makedirs(os.path.dirname(outfile), exist_ok=True)
conn = sqlite3.connect(outfile)
c = conn.cursor()

c.execute('''
CREATE TABLE IF NOT EXISTS items (
    code TEXT PRIMARY KEY,
    name TEXT,
    tags TEXT,
    line TEXT
)
''')

with open(infile, 'r', encoding='utf-8', errors='ignore') as fh:
    for raw in fh:
        line = raw.rstrip('\n')
        if not line.strip():
            continue
        parts = line.split('\t')
        if len(parts) < 2:
            continue
        code = parts[0].strip()
        name = parts[1].strip()
        # rest of columns as tags/metadata
        tags = '\t'.join(parts[2:]) if len(parts) > 2 else ''
        try:
            c.execute('INSERT OR REPLACE INTO items(code, name, tags, line) VALUES (?, ?, ?, ?)', (code, name, tags, line))
        except Exception as e:
            # ignore malformed lines
            continue

conn.commit()
conn.close()
print('Wrote', outfile)
