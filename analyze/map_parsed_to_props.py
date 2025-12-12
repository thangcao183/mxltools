#!/usr/bin/env python3
"""Map parsed property records (raw uint16 fields) to semantic columns from generated/props_en.tsv.

Produces:
- analyze/mapping_report.txt: summary of which parsed field indices match each semantic column and match rates.
- analyze/props_from_bins_mapped.tsv: a TSV with the same header as generated/props_en.tsv plus two extra columns:
    mapped_fields (semicolon-separated per semantic column showing which parsed field(s) matched),
    mapping_confidence (fraction of semantic columns that were matched unambiguously for that row).

Heuristics:
- Candidates are the 46 uint16 fields f0..f45 and 32-bit pairs f_i + (f_{i+1} << 16).
- For each semantic numeric field, we look for exact equality among the candidates.
- If exactly one candidate matches, it's recorded as the mapping; if multiple or none, it's left ambiguous.

This is conservative but will give a clear report of where parsed binaries align with the "canonical" props TSV.
"""

from pathlib import Path
import csv
import struct
from collections import defaultdict, Counter


def load_parsed_combined(path: Path):
    # parsed files have header: code,f0..f45,name
    rows = {}
    with path.open('r', encoding='utf-8', errors='replace') as f:
        r = csv.reader(f, delimiter='\t')
        hdr = next(r)
        for row in r:
            if not row:
                continue
            try:
                code = int(row[0])
            except:
                continue
            # fields are columns 1..46
            fields = []
            for i in range(1, 47):
                try:
                    fields.append(int(row[i]))
                except Exception:
                    fields.append(None)
            rows[code] = fields
    return rows


def load_ref_props(path: Path):
    lines = [l for l in path.read_text(encoding='utf-8', errors='replace').splitlines() if l.strip()]
    # find header line starting with # or first
    header = None
    for l in lines:
        if l.startswith('#'):
            header = l.lstrip('#')
            break
    if header is None:
        header = lines[0]
    cols = [c.strip() for c in header.split('\t')]
    rows = []
    for l in lines[1:]:
        parts = l.split('\t')
        if not parts:
            continue
        try:
            code = int(parts[0])
        except:
            continue
        row = {cols[i]: (parts[i] if i < len(parts) else '') for i in range(len(cols))}
        rows.append((code, row))
    return cols, rows


def build_candidates(fields):
    # fields: list of 46 uint16 (or None)
    cands = {}
    for i, v in enumerate(fields):
        if v is None:
            continue
        cands[f'f{i}'] = v
    # 32-bit little-endian pairs
    for i in range(len(fields)-1):
        lo = fields[i]
        hi = fields[i+1]
        if lo is None or hi is None:
            continue
        val = lo + (hi << 16)
        cands[f'f{i}_f{i+1}_u32'] = val
    return cands


def try_map(cols, ref_rows, parsed_rows, out_tsv: Path, report_txt: Path):
    numeric_cols = []
    # pick numeric-looking columns from ref header
    for c in cols:
        # common numeric column names
        if c in ('add', 'bits', 'bitsParamSave', 'bitsSave', 'saveParamBits', 'descpriority', 'descval', 'dgrp', 'dgrpfunc', 'dgrpval'):
            numeric_cols.append(c)
    if 'add' not in numeric_cols and 'add' in cols:
        numeric_cols.insert(0, 'add')

    # stats
    col_match_counter = {c: Counter() for c in numeric_cols}
    col_matched_total = Counter()

    # write mapped TSV header
    with out_tsv.open('w', encoding='utf-8', newline='') as outf:
        w = csv.writer(outf, delimiter='\t')
        out_header = cols + ['mapped_fields', 'mapping_confidence']
        w.writerow(out_header)
        for code, ref in ref_rows:
            ref_vals = ref
            parsed_fields = parsed_rows.get(code)
            mapped_fields = {}
            matched_count = 0
            possible_total = len(numeric_cols)
            if parsed_fields is None:
                # no parsed data for this code
                mapped_fields = {c: '' for c in numeric_cols}
            else:
                cands = build_candidates(parsed_fields)
                # for each numeric column, try to find exact match among candidates
                for nc in numeric_cols:
                    ref_val_raw = ref_vals.get(nc, '')
                    try:
                        ref_val = int(ref_val_raw) if ref_val_raw != '' else None
                    except:
                        ref_val = None
                    if ref_val is None:
                        mapped_fields[nc] = ''
                        continue
                    matches = [k for k, v in cands.items() if v == ref_val]
                    if len(matches) == 1:
                        mapped_fields[nc] = matches[0]
                        col_match_counter[nc][matches[0]] += 1
                        col_matched_total[nc] += 1
                        matched_count += 1
                    elif len(matches) > 1:
                        mapped_fields[nc] = ';'.join(matches)
                    else:
                        mapped_fields[nc] = ''

            # build mapped_fields string
            mf_items = [f'{k}={mapped_fields.get(k,"")}' for k in numeric_cols]
            mapping_confidence = matched_count / possible_total if possible_total else 0.0
            # write row: keep original ref columns (so file matches canonical)
            out_row = [ref.get(c, '') for c in cols] + [';'.join(mf_items), f'{mapping_confidence:.3f}']
            w.writerow(out_row)

    # write report
    with report_txt.open('w', encoding='utf-8') as rf:
        rf.write('Mapping report\n')
        rf.write('Numeric columns attempted: ' + ','.join(numeric_cols) + '\n\n')
        for nc in numeric_cols:
            rf.write(f'Column: {nc}\n')
            total = sum(col_match_counter[nc].values())
            rf.write(f'  total unambiguous matches: {total}\n')
            most = col_match_counter[nc].most_common(10)
            for k, v in most:
                rf.write(f'    {k} -> {v}\n')
            rf.write('\n')


if __name__ == '__main__':
    parsed_path = Path('analyze/properties_combined_parsed.tsv')
    if not parsed_path.exists():
        # fallback: try properties_parsed.tsv
        parsed_path = Path('analyze/properties_parsed.tsv')
    parsed = load_parsed_combined(parsed_path)
    ref_path = Path('generated/props_en.tsv')
    if not ref_path.exists():
        print('Reference generated/props_en.tsv not found')
        raise SystemExit(2)
    cols, ref_rows = load_ref_props(ref_path)

    out_tsv = Path('analyze/props_from_bins_mapped.tsv')
    report = Path('analyze/mapping_report.txt')
    try_map(cols, ref_rows, parsed, out_tsv, report)
    print('Wrote', out_tsv, 'and', report)
