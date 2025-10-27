from d2i_full_parser import D2ItemParser
import sqlite3
from d2i_full_parser import BitReader

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
prop = item.properties[1]
prop_info = property_db[prop.prop_id]
prop_start = prop.bit_offset
prop_bits = prop.prop_bits
found_start = prop_start
found_end = found_start + prop_bits
print('prop_index=1 id=',prop.prop_id,'prop_start=',prop_start,'prop_bits=',prop_bits,'found_start=',found_start,'found_end=',found_end)
print('existing_bits=', item.bitstring[found_start:found_end])
# Build new bits
addv = prop_info.get('addv',0)
raw_value = 50 + addv
param_bits = parser.get_param_bits(prop_info)
new_prop_bits = format(prop.prop_id,'09b')
if param_bits>0:
    new_prop_bits += format(prop.param, f'0{param_bits}b')
new_prop_bits += format(raw_value, f'0{prop_info["bits"]}b')
print('new_bits=', new_prop_bits)
print('lengths old/new', len(item.bitstring[found_start:found_end]), len(new_prop_bits))
# Show where ID bits are located within that slice
prop_start_pos = None
for i in range(len(item.bitstring)):
    # find the prop_start_pos printed earlier by parser: the prop_start_pos corresponds to the absolute position
    # before reading ID which is prop_start_pos = found_end + 9 ? We'll compute from debug prints instead.
    pass
slice_bits = item.bitstring[found_start:found_end]
print('slice_bits:', slice_bits)

# Use a local BitReader to re-read the slice like the parser (reads from end backwards)
local_reader = BitReader(slice_bits)
pos_before = local_reader.get_absolute_pos()
id_read = local_reader.read_number(9)
pos_after_id = local_reader.get_absolute_pos()
id_bits_rel_start = pos_after_id
id_bits_rel_end = pos_after_id + 9
print('local read id:', id_read, 'id bits in slice at', id_bits_rel_start, id_bits_rel_end,
    '->', slice_bits[id_bits_rel_start:id_bits_rel_end])

# param bits (if any)
param_bits = parser.get_param_bits(prop_info)
param_val = None
if param_bits > 0:
    param_val = local_reader.read_number(param_bits)
    pos_after_param = local_reader.get_absolute_pos()
    param_rel_start = pos_after_param
    param_rel_end = pos_after_param + param_bits
    print('local read param:', param_val, 'bits at', param_rel_start, param_rel_end,
        '->', slice_bits[param_rel_start:param_rel_end])

# value bits
value_bits = prop_info['bits']
value_val = local_reader.read_number(value_bits)
pos_after_value = local_reader.get_absolute_pos()
value_rel_start = pos_after_value
value_rel_end = pos_after_value + value_bits
print('local read value:', value_val, 'bits at', value_rel_start, value_rel_end,
    '->', slice_bits[value_rel_start:value_rel_end])
