from d2i_full_parser import D2ItemParser
import sqlite3

# Load property DB
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

# Parse base file
input_file = 'd2i/complete/relic_fungus_modified.d2i'
print('Parsing:', input_file)
item = parser.parse_file(input_file)

print('Before add, properties:')
for i,p in enumerate(item.properties):
    info = property_db.get(p.prop_id, {})
    print(i, p.prop_id, info.get('name'), 'value=', p.value, 'param=', p.param, 'bit_offset=', p.bit_offset)

# Add property id 97 value 50 param 1974
prop_id = 97
value = 50
param = 1974
print(f"Adding property {prop_id} value={value} param={param}")
parser.add_property_to_item(item, prop_id, value, param)

print('After add, properties:')
for i,p in enumerate(item.properties):
    info = property_db.get(p.prop_id, {})
    print(i, p.prop_id, info.get('name'), 'value=', p.value, 'param=', p.param, 'bit_offset=', p.bit_offset)

out_file = 'd2i/complete/relic_fungus_added_test.d2i'
parser.save_file(item, out_file)

print('Re-parsing saved file to verify...')
item2 = parser.parse_file(out_file)
print('Parsed properties:')
for i,p in enumerate(item2.properties):
    info = property_db.get(p.prop_id, {})
    print(i, p.prop_id, info.get('name'), 'value=', p.value, 'param=', p.param, 'bit_offset=', p.bit_offset)
print('Done')
