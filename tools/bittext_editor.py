#!/usr/bin/env python3
"""
tools/bittext_editor.py

Usage:
  python3 tools/bittext_editor.py dump <file.d2i> <out.txt>
  python3 tools/bittext_editor.py load <file.d2i> <in.txt>
  python3 tools/bittext_editor.py insert <file.d2i> <prop_id> <value>

- dump: writes canonical item bitstring (as '0'/'1' text) to <out.txt>. A single line contains the bits for the item after the 'JM' header.
- load: reads bits from <in.txt> and writes back to <file.d2i> (creates a backup <file.d2i>.bak_txt first). The tool validates length is multiple of 8.
- insert: convenience: inserts a property before the 9-bit end-marker and writes file (backup created). Uses props metadata if available.

Notes:
- This tool preserves the same bit/byte ordering as the internal ItemParser/Writer: each byte is represented MSB-first and the sequence of bytes is reversed relative to the item bitstring (matching the writer's prepend behavior).
- Always keep backups. After writing, the tool runs the project's inspector (if found) to verify parsing.
"""

import sys
from pathlib import Path
from itertools import zip_longest

SCRIPTDIR = Path(__file__).resolve().parent
PROPS_TSV = SCRIPTDIR.parent / 'utils' / 'txt_parser' / 'generated' / 'en' / 'props.tsv'
INSPECTOR = SCRIPTDIR / 'inspect_d2i.py'


def read_props_tsv(tsv_path):
    props = {}
    if not tsv_path.exists():
        return props
    for line in tsv_path.read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        parts = line.split('\t')
        try:
            pid = int(parts[0])
            add = int(parts[1]) if parts[1] else 0
            bits = int(parts[2]) if parts[2] else 0
            paramBits = int(parts[3]) if len(parts) > 3 and parts[3] else 0
            props[pid] = {'add': add, 'bits': bits, 'paramBits': paramBits}
        except Exception:
            continue
    return props


def bytes_to_item_bitstring(item_bytes: bytes) -> str:
    # ItemParser builds itemBitData by reading bytes and doing itemBitData.prepend(binaryStringFromNumber(aByte))
    # binaryStringFromNumber is MSB-first (format(b,'08b')) and prepend causes the bitstring to be the reverse of the byte order.
    chunks = [format(b, '08b') for b in item_bytes]
    return ''.join(reversed(chunks))


def item_bitstring_to_bytes(item_bitstring: str) -> bytes:
    if len(item_bitstring) % 8 != 0:
        raise ValueError('bitstring length not a multiple of 8')
    chunks = [item_bitstring[i:i+8] for i in range(0, len(item_bitstring), 8)]
    vals = [int(ch, 2) for ch in chunks]
    vals.reverse()
    return bytes(vals)


def find_end_marker(bitstring: str) -> int:
    marker = '1' * 9
    max_search = min(len(bitstring), 16384)
    for i in range(0, max_search):
        if i + 9 > len(bitstring):
            break
        if bitstring[i:i+9] == marker:
            return i
    return -1


def insert_property_in_item_bitstring(item_bitstring: str, prop_id: int, stored_value: int, value_bits: int, param_bits: int = 0, param_value: int = 0) -> str:
    # Use MSB-first for id/param/value inside the bitstring
    id_bits = format(prop_id, '09b')
    param_bits_s = format(param_value, f'0{param_bits}b') if param_bits else ''
    val_bits = format(stored_value, f'0{value_bits}b')
    newprop = id_bits + param_bits_s + val_bits
    ins = find_end_marker(item_bitstring)
    if ins == -1:
        raise RuntimeError('End marker not found in item bitstring')
    new_bitstring = item_bitstring[:ins] + newprop + item_bitstring[ins:]
    pad = (8 - (len(new_bitstring) % 8)) % 8
    new_bitstring += '0' * pad
    return new_bitstring


def dump_item_bits(d2i_path: Path, out_txt: Path):
    data = d2i_path.read_bytes()
    idx = data.find(b'JM')
    if idx == -1:
        raise RuntimeError('JM header not found')
    item_bytes = data[idx+2:]
    bits = bytes_to_item_bitstring(item_bytes)
    out_txt.write_text(bits, encoding='utf-8')
    print(f'Wrote bitstring to {out_txt} (len {len(bits)})')


def load_item_bits(d2i_path: Path, in_txt: Path):
    data = d2i_path.read_bytes()
    idx = data.find(b'JM')
    if idx == -1:
        raise RuntimeError('JM header not found')
    bits = in_txt.read_text(encoding='utf-8').strip().replace('\n','').replace(' ','')
    if len(bits) % 8 != 0:
        raise RuntimeError('Edited bitstring length must be multiple of 8')
    new_bytes = item_bitstring_to_bytes(bits)
    bak = d2i_path.with_suffix(d2i_path.suffix + '.bak_txt')
    if not bak.exists():
        bak.write_bytes(data)
        print('Backup created at', bak)
    d2i_path.write_bytes(data[:idx+2] + new_bytes)
    print('Wrote', d2i_path)
    # optional inspector run
    if INSPECTOR.exists():
        import subprocess
        p = subprocess.run(['python3', str(INSPECTOR), str(d2i_path)], capture_output=True, text=True)
        print(p.stdout)
        if p.stderr:
            print(p.stderr)


def insert_property_file(d2i_path: Path, prop_id: int, value: int):
    props = read_props_tsv(PROPS_TSV)
    meta = props.get(prop_id)
    if not meta:
        print('Property metadata not found for id', prop_id, '- defaulting to bits=12 add=0 paramBits=0')
        add = 0; bits = 12; paramBits = 0
    else:
        add = meta['add']; bits = meta['bits']; paramBits = meta['paramBits']
    stored = value + add
    data = d2i_path.read_bytes()
    idx = data.find(b'JM')
    if idx == -1:
        raise RuntimeError('JM header not found')
    item_bytes = data[idx+2:]
    bits_orig = bytes_to_item_bitstring(item_bytes)

    # helper: parse a few basic fields (isEar, isExtended, isPersonalized, isSocketed, itemType)
    def parse_basic_fields(bits: str):
        pos = 0
        def read_bits_local(l):
            nonlocal pos
            s = bits[pos:pos+l]
            pos += l
            return s
        def read_bool_local():
            return read_bits_local(1) == '1'
        def read_num_local(l):
            return int(read_bits_local(l), 2)

        try:
            isQuest = read_bool_local()
            pos += 3
            isIdentified = read_bool_local()
            pos += 5
            pos += 1
            isSocketed = read_bool_local()
            pos += 2
            pos += 2
            isEar = read_bool_local()
            isStarter = read_bool_local()
            pos += 2
            pos += 1
            isExtended_bool = read_bool_local()
            isExtended = not isExtended_bool
            isEthereal = read_bool_local()
            pos += 1
            isPersonalized = read_bool_local()
            pos += 1
            isRW = read_bool_local()
            pos += 5
            pos += 8
            pos += 2
            location = read_num_local(3)
            whereEquipped = read_num_local(4)
            column = read_num_local(4)
            row = read_num_local(4)
            storage = read_num_local(3)
            if isEar:
                return {'isEar': True, 'isExtended': isExtended, 'isPersonalized': isPersonalized, 'isSocketed': isSocketed, 'itemType': 'ear'}
            # read 4 bytes of itemType
            itemTypeBytes = []
            for i in range(4):
                # if not enough bits left, abort
                if pos + 8 > len(bits):
                    return None
                val = read_num_local(8)
                itemTypeBytes.append(val)
            itemType = ''.join(chr(v) for v in itemTypeBytes).rstrip('\x00').rstrip()
            return {'isEar': False, 'isExtended': isExtended, 'isPersonalized': isPersonalized, 'isSocketed': isSocketed, 'itemType': itemType}
        except Exception:
            return None

    pre = parse_basic_fields(bits_orig)
    if pre is None:
        raise RuntimeError('Failed to parse basic fields from original item — aborting insert')

    ins = find_end_marker(bits_orig)
    if ins == -1:
        raise RuntimeError('End marker not found')

    new_bits = insert_property_in_item_bitstring(bits_orig, prop_id, stored, bits, paramBits, 0)
    new_bytes = item_bitstring_to_bytes(new_bits)

    bak = d2i_path.with_suffix(d2i_path.suffix + '.bak_bittext')
    if not bak.exists():
        bak.write_bytes(data)
        print('Backup created at', bak)

    # write and then verify; on mismatch, restore backup
    d2i_path.write_bytes(data[:idx+2] + new_bytes)
    print('Wrote candidate file', d2i_path, ' — running verification')

    # compute post state from newly-written bytes
    new_data = d2i_path.read_bytes()
    new_item_bytes = new_data[idx+2:]
    bits_new = bytes_to_item_bitstring(new_item_bytes)
    post = parse_basic_fields(bits_new)

    if post is None:
        # restore
        d2i_path.write_bytes(bak.read_bytes())
        print('Post-parse failed. Restored backup:', bak)
        raise RuntimeError('Post-parse failed after insertion; backup restored')

    # compare key invariants
    mismatch = []
    for k in ('isEar', 'isExtended', 'isPersonalized', 'isSocketed', 'itemType'):
        if pre.get(k) != post.get(k):
            mismatch.append((k, pre.get(k), post.get(k)))

    if mismatch:
        # restore
        d2i_path.write_bytes(bak.read_bytes())
        print('Insertion caused mismatch in key fields — restored backup. Mismatches:')
        for k, a, b in mismatch:
            print(f"  {k}: before={a} after={b}")
        raise RuntimeError('Insertion aborted due to field mismatches; backup restored')

    print('Inserted property id', prop_id, 'value', value, '(stored', stored, ') into', d2i_path)
    if INSPECTOR.exists():
        import subprocess
        p = subprocess.run(['python3', str(INSPECTOR), str(d2i_path)], capture_output=True, text=True)
        print(p.stdout)
        if p.stderr:
            print(p.stderr)


def print_help():
    print(__doc__)


def main():
    if len(sys.argv) < 2:
        print_help(); sys.exit(1)
    cmd = sys.argv[1]
    if cmd == 'dump' and len(sys.argv) == 4:
        dump_item_bits(Path(sys.argv[2]), Path(sys.argv[3])); return
    if cmd == 'load' and len(sys.argv) == 4:
        load_item_bits(Path(sys.argv[2]), Path(sys.argv[3])); return
    if cmd == 'insert' and len(sys.argv) == 4:
        insert_property_file(Path(sys.argv[2]), int(sys.argv[3]), int(sys.argv[3]) if False else int(sys.argv[3])); return
    # support old API: insert file prop value
    if cmd == 'insert' and len(sys.argv) == 5:
        insert_property_file(Path(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])); return
    print_help()

if __name__ == '__main__':
    main()
