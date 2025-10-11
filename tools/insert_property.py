#!/usr/bin/env python3
import sys
from pathlib import Path

def read_props_tsv(tsv_path):
    props = {}
    if not tsv_path.exists():
        return props
    for line in tsv_path.read_text(encoding='utf-8').splitlines():
        line=line.strip()
        if not line or line.startswith('#'):
            continue
        parts=line.split('\t')
        try:
            pid=int(parts[0])
            add=int(parts[1]) if parts[1] else 0
            bits=int(parts[2]) if parts[2] else 0
            paramBits=int(parts[3]) if len(parts)>3 and parts[3] else 0
            props[pid] = {'add':add,'bits':bits,'paramBits':paramBits}
        except Exception:
            continue
    return props

def bytes_to_item_bitstring(item_bytes):
    # ItemParser builds itemBitData by for each byte: binaryStringFromNumber(aByte) [MSB-first] then prepend
    # So item_bitstring = ''.join(reversed([format(b,'08b') for b in item_bytes]))
    chunks = [format(b,'08b') for b in item_bytes]
    return ''.join(reversed(chunks))

def item_bitstring_to_bytes(item_bitstring):
    # chunk into 8-bit MSB-first pieces, then create bytes_list by prepending each chunk value (so reverse order)
    chunks = [item_bitstring[i:i+8] for i in range(0, len(item_bitstring), 8)]
    vals = [int(ch,2) for ch in chunks]
    vals.reverse()
    return bytes(vals)

def find_end_marker(bitstring):
    # search for 9 ones sliding
    marker = '1'*9
    max_search = min(len(bitstring), 4096)
    for i in range(0, max_search):
        if i+9>len(bitstring): break
        if bitstring[i:i+9]==marker:
            return i
    return -1

def insert_property_in_item_bitstring(item_bitstring, prop_id, stored_value, value_bits, param_bits=0, param_value=0):
    # prop id and values use MSB-first bit order inside item->bitString
    id_bits = format(prop_id, '09b')
    param_bits_s = format(param_value, f'0{param_bits}b') if param_bits else ''
    val_bits = format(stored_value, f'0{value_bits}b')
    newprop = id_bits + param_bits_s + val_bits
    # find insertion point as end-marker
    ins = find_end_marker(item_bitstring)
    if ins == -1:
        raise RuntimeError('End marker not found in item bitstring')
    new_bitstring = item_bitstring[:ins] + newprop + item_bitstring[ins:]
    # pad
    pad = (8 - (len(new_bitstring) % 8)) % 8
    new_bitstring += '0'*pad
    return new_bitstring

def main():
    if len(sys.argv) != 4:
        print('Usage: insert_property.py <file.d2i> <prop_id> <value>')
        return 2
    path = Path(sys.argv[1])
    prop_id = int(sys.argv[2])
    value = int(sys.argv[3])

    if not path.exists():
        print('File not found:', path)
        return 3

    props = read_props_tsv(Path(__file__).parent.parent / 'utils' / 'txt_parser' / 'generated' / 'en' / 'props.tsv')
    meta = props.get(prop_id)
    if not meta:
        print('Warning: property metadata not found for id', prop_id, '- proceeding with add=0 bits=12')
        add = 0
        bits = 12
        paramBits = 0
    else:
        add = meta['add']
        bits = meta['bits']
        paramBits = meta['paramBits']

    stored = value + add

    data = path.read_bytes()
    # find first occurrence of JM and treat following bytes as item
    header = b'JM'
    idx = data.find(header)
    if idx == -1:
        print('JM header not found')
        return 4
    item_bytes = data[idx+2:]

    item_bitstring = bytes_to_item_bitstring(item_bytes)
    # insert
    new_item_bitstring = insert_property_in_item_bitstring(item_bitstring, prop_id, stored, bits, paramBits, 0)
    new_item_bytes = item_bitstring_to_bytes(new_item_bitstring)
    out = data[:idx+2] + new_item_bytes

    bak = path.with_suffix(path.suffix + '.bak_py')
    if not bak.exists():
        bak.write_bytes(data)
    path.write_bytes(out)
    print('Wrote', path, 'backup at', bak)

    # run inspector
    import subprocess
    insp = Path(__file__).parent / 'inspect_d2i.py'
    proc = subprocess.run(['python3', str(insp), str(path)], capture_output=True, text=True)
    print(proc.stdout)
    if proc.stderr:
        print(proc.stderr)
    return 0

if __name__ == '__main__':
    sys.exit(main())
