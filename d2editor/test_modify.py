from d2i_full_parser import D2ItemParser
import sqlite3

# load property db
property_db = {}
conn = sqlite3.connect('data/props.db')
cursor = conn.cursor()
cursor.execute("SELECT code, name, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0")
for row in cursor.fetchall():
    code, name, addv, bits, param_bits, h_save_param_bits = row
    if param_bits == '':
        param_bits = None
    if h_save_param_bits == '':
        h_save_param_bits = None
    property_db[code] = {
        'name': name or f'prop_{code}',
        'addv': addv if addv is not None else 0,
        'bits': bits,
        'paramBits': param_bits,
        'h_saveParamBits': h_save_param_bits
    }
conn.close()

parser = D2ItemParser(property_db)
item = parser.parse_file('d2i/complete/relic_fungus_modified.d2i')
print('Before:', [(p.prop_id, p.value, p.param, p.bit_offset, p.prop_bits) for p in item.properties])
# dump surrounding bits for inspection
start_dump = 70
end_dump = 140
print('bitstring slice before:', item.bitstring[start_dump:end_dump])
parser.modify_property(item, 1, new_value=50)
print('now modify param of prop 1 -> 1800')
parser.modify_property(item, 1, new_value=None, new_param=1800)
print('bitstring slice after:', item.bitstring[start_dump:end_dump])
parser.save_file(item, 'd2i/complete/relic_fungus_modified_testout.d2i')
print('Saved')
# Re-parse in-memory bitstring to verify modification before writing to disk
from d2i_full_parser import BitReader
reparsed = parser.parse_item(item.file_data, item.bitstring, BitReader(item.bitstring))
print('Reparsed props:', [(p.prop_id, p.value, p.param, p.bit_offset) for p in reparsed.properties])
