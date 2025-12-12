#!/usr/bin/env python3
"""
extract_props_dat.py

Decompress a Qt "*.dat" resource file created by utils/CompressFiles/main.cpp (qCompress + 4-byte CRC header)
and write the contained TSV (or original) file to disk.

Usage:
    python utils/extract_props_dat.py [INPUT_DAT] [OUTPUT_FILE]

Defaults:
    INPUT_DAT = resources/data/props.dat
    OUTPUT_FILE = generated/props_from_dat.tsv

The script will attempt simple CRC checks and try both common qCompress wrappers when decompressing.
"""

import sys
import os
import argparse
import struct
import zlib
import binascii


def read_dat(path: str) -> bytes:
    with open(path, 'rb') as f:
        header = f.read(4)
        if len(header) < 4:
            raise ValueError('file too short')
        compressed_crc, original_crc = struct.unpack('<HH', header)
        compressed_data = f.read()
    return compressed_crc, original_crc, compressed_data


def qchecksum(data: bytes) -> int:
    """Approximate Qt qChecksum using CRC-16/CCITT (binascii.crc_hqx).
    Qt's qChecksum historically uses CRC-16 variant; this gives a reasonable check and will match
    for many builds. If it doesn't match, we only warn and continue.
    """
    return binascii.crc_hqx(data, 0) & 0xFFFF


def try_decompress(compressed: bytes) -> bytes:
    """Try common ways to decompress data produced by Qt's qCompress.

    qCompress returns zlib-compressed data. Historically some wrappers include a 4-byte
    length prefix at the start of the compressed payload. We'll try:
      1) decompress the whole payload
      2) decompress skipping the first 4 bytes
    """
    # Try raw
    try:
        return zlib.decompress(compressed)
    except zlib.error:
        pass

    # Try skipping first 4 bytes (qCompress sometimes puts a 4-byte uncompressed length)
    if len(compressed) > 4:
        try:
            return zlib.decompress(compressed[4:])
        except zlib.error:
            pass

    # As a last resort, try decompress with negative wbits to allow raw deflate stream
    try:
        return zlib.decompress(compressed, -zlib.MAX_WBITS)
    except zlib.error as e:
        raise RuntimeError(f"Failed to decompress data: {e}")


def main():
    p = argparse.ArgumentParser(description='Extract TSV (or original) from a Qt .dat resource file')
    p.add_argument('input', nargs='?', default='resources/data/en/props.dat', help='Input .dat file path')
    p.add_argument('output', nargs='?', default='generated/props_from_dat.tsv', help='Output file path')
    args = p.parse_args()

    if not os.path.exists(args.input):
        print(f"Input file not found: {args.input}")
        sys.exit(2)

    compressed_crc, original_crc, compressed_data = read_dat(args.input)
    print(f"Read {len(compressed_data)} bytes compressed payload; compressed_crc=0x{compressed_crc:04x}, original_crc=0x{original_crc:04x}")

    # quick CRC check of compressed payload
    try:
        check = qchecksum(compressed_data)
        if check != compressed_crc:
            print(f"Warning: compressed CRC mismatch (computed 0x{check:04x} != header 0x{compressed_crc:04x})")
        else:
            print("Compressed CRC ok")
    except Exception:
        print("Warning: failed to compute compressed CRC")

    # attempt to decompress
    try:
        original = try_decompress(compressed_data)
    except RuntimeError as e:
        print(e)
        sys.exit(3)

    # optional CRC check on original
    try:
        check_orig = qchecksum(original)
        if check_orig != original_crc:
            print(f"Warning: original CRC mismatch (computed 0x{check_orig:04x} != header 0x{original_crc:04x})")
        else:
            print("Original CRC ok")
    except Exception:
        print("Warning: failed to compute original CRC")

    # ensure output directory
    outdir = os.path.dirname(args.output)
    if outdir and not os.path.exists(outdir):
        os.makedirs(outdir, exist_ok=True)

    # write result
    with open(args.output, 'wb') as f:
        f.write(original)

    # Also try to show the first few lines (if text)
    try:
        txt = original.decode('utf-8')
    except Exception:
        try:
            txt = original.decode('latin-1')
        except Exception:
            txt = None

    if txt is not None:
        print('Wrote output and detected text. First 10 lines:')
        for i, line in enumerate(txt.splitlines()):
            if i >= 10:
                break
            print(line)
    else:
        print('Wrote output (binary content)')

    print(f'Output written to: {args.output}')


if __name__ == '__main__':
    main()
