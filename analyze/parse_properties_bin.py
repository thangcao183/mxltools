#!/usr/bin/env python3
"""Parse analyze/properties.bin into a CSV/TSV with raw uint16 fields for inspection.

Assumptions:
- File contains a 4-byte header, then fixed-size records of 92 bytes (46 little-endian uint16 fields).
- code = field0 // 2 (observed pattern where field0 increments 0,2,4...)

Output:
- TSV file with columns: code, f0..f45, name (optional lookup from generated/props_en.tsv if present)

This is an exploratory parser so you can inspect raw fields before we map them to semantic columns.
"""

import struct
from pathlib import Path
import csv
import sys


def load_name_map(props_tsv_path: Path):
    """Try to load a mapping from code -> name from generated/props_en.tsv if it exists.
    Expect the TSV to have the code in the first column and a human-readable name in one of the later columns.
    This function tries a few heuristics to pick a sensible name column.
    """
    if not props_tsv_path.exists():
        return {}
    mapping = {}
    with props_tsv_path.open('r', encoding='utf-8', errors='replace') as f:
        # read header and sample row to detect columns
        lines = [l.rstrip('\n') for l in f]
    if not lines:
        return {}
    # split by tab
    rows = [l.split('\t') for l in lines if l.strip()]
    # find best candidate for name: look for a column that contains alphabetic text in many rows
    col_alpha_scores = []
    ncols = max(len(r) for r in rows)
    for c in range(ncols):
        score = 0
        for r in rows:
            if c < len(r):
                cell = r[c]
                if any(ch.isalpha() for ch in cell) and len(cell) > 0:
                    score += 1
        col_alpha_scores.append((score, c))
    col_alpha_scores.sort(reverse=True)
    best_col = col_alpha_scores[0][1]
    # now build mapping
    for r in rows:
        if not r:
            continue
        try:
            code = int(r[0])
        except Exception:
            continue
        name = r[best_col] if best_col < len(r) else ''
        mapping[code] = name
    return mapping


def parse_properties(bin_path: Path, out_path: Path, props_tsv: Path = Path('generated/props_en.tsv')):
    data = bin_path.read_bytes()
    if len(data) < 8:
        raise SystemExit('file too small')
    # assume 4-byte header
    offset = 4
    rec_size = 92
    if (len(data) - offset) % rec_size != 0:
        # warn, but continue using floor division
        print(f'Warning: file length {len(data)} minus offset {offset} not divisible by {rec_size}')
    n = (len(data) - offset) // rec_size
    print(f'Parsing {n} records (record_size={rec_size}, offset={offset})')

    name_map = load_name_map(props_tsv)

    with out_path.open('w', encoding='utf-8', newline='') as csvf:
        writer = csv.writer(csvf, delimiter='\t')
        # header
        header = ['code'] + [f'f{i}' for i in range(46)] + ['name']
        writer.writerow(header)
        for i in range(n):
            start = offset + i * rec_size
            rec = data[start:start+rec_size]
            if len(rec) != rec_size:
                print(f'skipping short record at index {i}')
                continue
            fields = list(struct.unpack('<46H', rec))
            code = fields[0] // 2
            name = name_map.get(code, '')
            row = [code] + fields + [name]
            writer.writerow(row)


if __name__ == '__main__':
    import argparse

    p = argparse.ArgumentParser(description='Parse analyze/properties.bin into TSV of uint16 fields')
    p.add_argument('bin', nargs='?', default='analyze/properties.bin')
    p.add_argument('out', nargs='?', default='analyze/properties_parsed.tsv')
    p.add_argument('--props-tsv', default='generated/props_en.tsv', help='Optional props TSV for name lookup')
    args = p.parse_args()

    bin_path = Path(args.bin)
    out_path = Path(args.out)
    props_tsv = Path(args.props_tsv)

    if not bin_path.exists():
        print('Bin file not found:', bin_path)
        sys.exit(2)
    parse_properties(bin_path, out_path, props_tsv)
    print('Wrote', out_path)
