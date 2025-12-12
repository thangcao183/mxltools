extract_props_dat.py — README

Purpose

This small helper extracts the original TSV (or other) text from a Qt-compressed `.dat` resource file
(created by the project's `utils/CompressFiles` tool which uses `qCompress`).

Usage

- Default: (reads `resources/data/props.dat` and writes `generated/props_from_dat.tsv`)

  python3 utils/extract_props_dat.py

- Specify input and output paths:

  python3 utils/extract_props_dat.py resources/data/en/props.dat generated/props_en.tsv

Notes

- The script attempts a CRC check and will warn if checksums don't match; it will still write output when decompression succeeds.
- If the root `resources/data/props.dat` doesn't decompress correctly, try the localized ones (for example `resources/data/en/props.dat`).
- The script writes the decompressed bytes as-is. If the original file is UTF-8 TSV, you'll get a readable `.tsv` file.

If you want, I can:
- wire this into an npm/Makefile target,
- add a small test that validates decompression for the `en/props.dat` file,
- or automatically import the TSV into `data/props.db` (SQLite) using existing scripts.
