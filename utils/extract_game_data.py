#!/usr/bin/env python3
"""
Extract and decompress game data from .dat files to CSV/TSV/SQLite formats.

This script reads compressed .dat files (used by MedianXL Offline Tools),
decompresses them, and exports to various formats for easier analysis.

File format: [2-byte compressed CRC][2-byte original CRC][compressed data]
The data is compressed using Qt's qCompress (zlib format with 4-byte size header)
"""

import struct
import zlib
import csv
import sqlite3
import argparse
import sys
from pathlib import Path
from typing import List, Tuple, Optional


def calculate_qchecksum(data: bytes) -> int:
    """
    Calculate Qt's qChecksum (16-bit checksum).
    Algorithm matches Qt's qChecksum implementation.
    """
    checksum = 0
    for byte in data:
        checksum = ((checksum >> 1) | ((checksum & 1) << 15)) & 0xFFFF
        checksum = (checksum + byte) & 0xFFFF
    return checksum


def decompress_dat_file(file_path: Path) -> Optional[bytes]:
    """
    Decompress a .dat file using Qt's compression format.
    
    Returns:
        Decompressed data as bytes, or None if failed
    """
    try:
        with open(file_path, 'rb') as f:
            # Read CRCs
            compressed_crc_bytes = f.read(2)
            original_crc_bytes = f.read(2)
            
            if len(compressed_crc_bytes) < 2 or len(original_crc_bytes) < 2:
                print(f"Error: File too short: {file_path}")
                return None
            
            compressed_crc = struct.unpack('<H', compressed_crc_bytes)[0]
            original_crc = struct.unpack('<H', original_crc_bytes)[0]
            
            # Read compressed data
            compressed_data = f.read()
            
            # Verify compressed CRC
            calculated_compressed_crc = calculate_qchecksum(compressed_data)
            if calculated_compressed_crc != compressed_crc:
                print(f"Warning: Compressed CRC mismatch in {file_path}")
                print(f"  Expected: {compressed_crc:04x}, Got: {calculated_compressed_crc:04x}")
                # Continue anyway, might still work
            
            # Decompress using Qt's qUncompress format
            # Qt's qCompress adds a 4-byte big-endian size header before zlib data
            if len(compressed_data) < 4:
                print(f"Error: Compressed data too short in {file_path}")
                return None
            
            # Skip the 4-byte size header and decompress
            original_data = zlib.decompress(compressed_data[4:])
            
            # Verify original CRC
            calculated_original_crc = calculate_qchecksum(original_data)
            if calculated_original_crc != original_crc:
                print(f"Warning: Original CRC mismatch in {file_path}")
                print(f"  Expected: {original_crc:04x}, Got: {calculated_original_crc:04x}")
            
            print(f"Successfully decompressed {file_path}")
            print(f"  Compressed size: {len(compressed_data)} bytes")
            print(f"  Decompressed size: {len(original_data)} bytes")
            
            return original_data
            
    except FileNotFoundError:
        print(f"Error: File not found: {file_path}")
        return None
    except zlib.error as e:
        print(f"Error: Decompression failed for {file_path}: {e}")
        return None
    except Exception as e:
        print(f"Error: Unexpected error processing {file_path}: {e}")
        return None


def parse_tsv_data(data: bytes) -> List[List[str]]:
    """
    Parse decompressed TSV data into rows.
    Each line is tab-separated values.
    """
    text = data.decode('utf-8', errors='replace')
    rows = []
    for line in text.strip().split('\n'):
        if line.strip():
            # Split by tab
            row = line.split('\t')
            rows.append(row)
    return rows


def export_to_csv(rows: List[List[str]], output_path: Path, delimiter: str = ','):
    """Export data to CSV/TSV format."""
    with open(output_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f, delimiter=delimiter)
        writer.writerows(rows)
    print(f"Exported to {output_path}")


def export_to_sqlite(rows: List[List[str]], output_path: Path, table_name: str):
    """Export data to SQLite database."""
    if not rows:
        print("No data to export")
        return
    
    # Create database
    conn = sqlite3.connect(output_path)
    cursor = conn.cursor()
    
    # Determine number of columns
    max_cols = max(len(row) for row in rows) if rows else 0
    
    # Create table with generic column names
    columns = [f"col{i}" for i in range(max_cols)]
    create_table_sql = f"CREATE TABLE IF NOT EXISTS {table_name} ({', '.join(f'{col} TEXT' for col in columns)})"
    cursor.execute(create_table_sql)
    
    # Insert data
    placeholders = ', '.join(['?'] * max_cols)
    insert_sql = f"INSERT INTO {table_name} VALUES ({placeholders})"
    
    for row in rows:
        # Pad row to match column count
        padded_row = row + [''] * (max_cols - len(row))
        cursor.execute(insert_sql, padded_row)
    
    conn.commit()
    conn.close()
    print(f"Exported to SQLite database: {output_path}")
    print(f"  Table: {table_name}")
    print(f"  Rows: {len(rows)}")


def process_file(input_path: Path, output_format: str, output_dir: Optional[Path] = None):
    """
    Process a single .dat file and export to specified format.
    
    Args:
        input_path: Path to input .dat file
        output_format: 'csv', 'tsv', or 'sqlite'
        output_dir: Optional output directory (defaults to same as input)
    """
    # Decompress
    data = decompress_dat_file(input_path)
    if data is None:
        return False
    
    # Parse TSV data
    rows = parse_tsv_data(data)
    if not rows:
        print(f"Warning: No data rows found in {input_path}")
        return False
    
    print(f"Parsed {len(rows)} rows with {len(rows[0]) if rows else 0} columns")
    
    # Determine output path
    if output_dir is None:
        output_dir = input_path.parent
    output_dir.mkdir(parents=True, exist_ok=True)
    
    base_name = input_path.stem
    
    # Export based on format
    if output_format == 'csv':
        output_path = output_dir / f"{base_name}.csv"
        export_to_csv(rows, output_path, delimiter=',')
    elif output_format == 'tsv':
        output_path = output_dir / f"{base_name}.tsv"
        export_to_csv(rows, output_path, delimiter='\t')
    elif output_format == 'sqlite':
        output_path = output_dir / f"{base_name}.db"
        table_name = base_name.replace('-', '_').replace('.', '_')
        export_to_sqlite(rows, output_path, table_name)
    else:
        print(f"Error: Unknown output format: {output_format}")
        return False
    
    return True


def process_directory(input_dir: Path, output_format: str, output_dir: Optional[Path] = None):
    """Process all .dat files in a directory."""
    dat_files = list(input_dir.glob("**/*.dat"))
    
    if not dat_files:
        print(f"No .dat files found in {input_dir}")
        return
    
    print(f"\nFound {len(dat_files)} .dat files")
    print("=" * 60)
    
    success_count = 0
    for dat_file in dat_files:
        print(f"\nProcessing: {dat_file.relative_to(input_dir)}")
        print("-" * 60)
        if process_file(dat_file, output_format, output_dir):
            success_count += 1
    
    print("\n" + "=" * 60)
    print(f"Successfully processed {success_count}/{len(dat_files)} files")


def main():
    parser = argparse.ArgumentParser(
        description="Extract and decompress MedianXL game data from .dat files",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Extract single file to CSV
  %(prog)s resources/data/en/items.dat -f csv
  
  # Extract single file to TSV
  %(prog)s resources/data/en/props.dat -f tsv -o output/
  
  # Extract all files in directory to SQLite
  %(prog)s resources/data/en/ -f sqlite -o output/
  
  # Extract entire data directory
  %(prog)s resources/data/ -f csv -o extracted_data/
        """
    )
    
    parser.add_argument('input', type=Path,
                        help='Input .dat file or directory')
    parser.add_argument('-f', '--format', 
                        choices=['csv', 'tsv', 'sqlite'],
                        default='tsv',
                        help='Output format (default: tsv)')
    parser.add_argument('-o', '--output',
                        type=Path,
                        help='Output directory (default: same as input)')
    
    args = parser.parse_args()
    
    # Check if input exists
    if not args.input.exists():
        print(f"Error: Input path does not exist: {args.input}")
        sys.exit(1)
    
    # Process file or directory
    if args.input.is_file():
        if not args.input.suffix == '.dat':
            print(f"Warning: Input file doesn't have .dat extension: {args.input}")
        success = process_file(args.input, args.format, args.output)
        sys.exit(0 if success else 1)
    elif args.input.is_dir():
        process_directory(args.input, args.format, args.output)
    else:
        print(f"Error: Invalid input path: {args.input}")
        sys.exit(1)


if __name__ == '__main__':
    main()
