#!/usr/bin/env python3
"""Bit utilities shared between parser and tools.

Provides canonical conversions between bytes and bitstrings and helpers for
MSB/LSB number encoding.
"""
from typing import List

def byte_to_binary_msb(byte: int) -> str:
    """Convert byte to MSB-first binary string"""
    return format(byte, '08b')


def byte_to_binary_lsb(byte: int) -> str:
    """Convert byte to LSB-first binary string"""
    s = ''
    for i in range(8):
        s += '1' if (byte >> i) & 1 else '0'
    return s


def create_bitstring_from_bytes(data: bytes) -> str:
    """Create bitstring from bytes using PREPEND MSB-per-byte logic.

    This matches the C++ parser's approach (binaryStringFromNumber + PREPEND)
    and the ReverseBitReader expectation.
    """
    bitstring = ""
    for i in range(2, len(data)):
        # PREPEND each MSB-order byte
        bitstring = byte_to_binary_msb(data[i]) + bitstring
    return bitstring


def bitstring_to_bytes(bitstring: str) -> bytes:
    """Convert canonical bitstring back to bytes (reverse of create_bitstring_from_bytes).

    Splits into 8-bit chunks (forward order), reverses chunk order to undo PREPEND,
    and converts each chunk from MSB-first to a byte value.
    """
    # Pad to byte boundary
    remainder = len(bitstring) % 8
    if remainder != 0:
        bitstring = bitstring + '0' * (8 - remainder)

    chunks: List[str] = [bitstring[i:i+8] for i in range(0, len(bitstring), 8)]
    chunks.reverse()

    bytes_list = bytearray()
    for chunk in chunks:
        bytes_list.append(int(chunk, 2))

    # Prepend JM header
    return b'JM' + bytes(bytes_list)


def number_to_binary_msb(value: int, num_bits: int) -> str:
    """Return MSB-first binary representation with fixed width."""
    if value < 0:
        raise ValueError("value must be non-negative")
    return format(value, f'0{num_bits}b')


def number_to_binary_lsb(value: int, num_bits: int) -> str:
    """Return LSB-first binary representation by reversing MSB string."""
    return number_to_binary_msb(value, num_bits)[::-1]
