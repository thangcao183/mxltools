import struct
import zlib
import os
import sys

def compress_file(input_path, output_path):
    if not os.path.exists(input_path):
        print(f"Error: Input file not found: {input_path}")
        return False

    with open(input_path, 'rb') as f:
        data = f.read()

    # Calculate original CRC (placeholder or best effort)
    original_crc = 0 # We don't have exact qChecksum implementation in python handy that matches Qt perfectly

    # Compress
    compressed_data = zlib.compress(data)
    
    # qCompress format: 4 bytes uncompressed size (Big Endian) + zlib stream
    uncompressed_size = len(data)
    q_compressed_data = struct.pack('>I', uncompressed_size) + compressed_data

    # Calculate compressed CRC
    compressed_crc = 0 # Placeholder

    # Write .dat file
    # Header: compressed_crc (2 bytes LE), original_crc (2 bytes LE)
    with open(output_path, 'wb') as f:
        f.write(struct.pack('<HH', compressed_crc, original_crc))
        f.write(q_compressed_data)
    
    print(f"Successfully created {output_path} from {input_path}")
    return True

if __name__ == "__main__":
    input_tsv = "utils/txt_parser/generated/en/props.tsv"
    output_dat = "resources/data/props.dat"
    
    # Allow overriding paths
    if len(sys.argv) > 1:
        input_tsv = sys.argv[1]
    if len(sys.argv) > 2:
        output_dat = sys.argv[2]

    compress_file(input_tsv, output_dat)
