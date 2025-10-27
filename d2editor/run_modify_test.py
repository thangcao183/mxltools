#!/usr/bin/env python3
import importlib.util
import sqlite3
from pathlib import Path

BASE = Path(__file__).resolve().parent
PARSER_PY = BASE / 'd2i_full_parser.py'
DB_PATH = BASE / 'data' / 'props.db'
TEST_FILE = BASE / 'd2i' / 'complete' / 'relic_fungus.d2i'
OUT_FILE = BASE / 'd2i' / 'complete' / 'relic_fungus_saved_test.d2i'

if not TEST_FILE.exists():
    raise SystemExit(f"Test file not found: {TEST_FILE}")
if not PARSER_PY.exists():
    raise SystemExit(f"Parser not found: {PARSER_PY}")
if not DB_PATH.exists():
    raise SystemExit(f"Property DB not found: {DB_PATH}")

# import parser module
spec = importlib.util.spec_from_file_location('d2i_full_parser', str(PARSER_PY))
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

# load property DB
conn = sqlite3.connect(str(DB_PATH))
cur = conn.cursor()
cur.execute("SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0")
property_db = {}
for row in cur.fetchall():
    code, name, addv, bits, paramBits, h_save = row
    if paramBits == '':
        paramBits = None
    if h_save == '':
        h_save = None
    property_db[code] = {
        'name': name or f'prop_{code}',
        'addv': addv or 0,
        'bits': bits,
        'paramBits': paramBits,
        'h_saveParamBits': h_save,
    }
conn.close()

parser = mod.D2ItemParser(property_db)
print('Parsing original...')
item = parser.parse_file(str(TEST_FILE))
if not item.properties:
    raise SystemExit('No properties parsed')
print('Original prop0:', item.properties[0].prop_id, item.properties[0].value, 'bit_offset:', item.properties[0].bit_offset, 'prop_bits:', item.properties[0].prop_bits)
start = item.properties[0].bit_offset
end = start + item.properties[0].prop_bits
print('Bits at start before:', item.bitstring[start:end])

print('\nModifying prop0 -> value=5')
parser.modify_property(item, 0, new_value=5)
print('\nBits at start after modify (in memory):', item.bitstring[start:end])
parser.save_file(item, str(OUT_FILE))

print('\nRe-parsing saved file...')
item2 = parser.parse_file(str(OUT_FILE))
print('Reparsed prop0:', item2.properties[0].prop_id, item2.properties[0].value, 'bit_offset:', item2.properties[0].bit_offset, 'prop_bits:', item2.properties[0].prop_bits)
start2 = item2.properties[0].bit_offset
end2 = start2 + item2.properties[0].prop_bits
print('Bits at start in reparsed item:', item2.bitstring[start2:end2])
slice_bits = item2.bitstring[start2:end2]
id_bits = slice_bits[:9]
val_bits = slice_bits[9:]
print('id_bits=', id_bits, '=>', int(id_bits,2))
print('val_bits=', val_bits, '=>', int(val_bits,2), '(displayed value should be', int(val_bits,2)- (parser.property_db[item2.properties[0].prop_id].get('addv',0)),')')

print('\nDone')
