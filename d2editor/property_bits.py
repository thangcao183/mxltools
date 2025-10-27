"""Helpers to build and validate property bit encodings.

This centralizes the conventions used across the tools and parser:
- forward storage order: [value bits][param bits][ID bits]
- MSB-first encoding per-field (so each field uses MSB-first strings)
"""
from typing import Dict, Tuple
from bitutils import number_to_binary_msb


def get_param_bits_from_info(prop_info: Dict) -> int:
    """Determine parameter bits using same priority as other tools.

    Priority: h_saveParamBits > paramBits
    Handles numeric or string values stored in database.
    """
    h_save = prop_info.get('h_saveParamBits')
    if h_save is not None:
        h_str = str(h_save).strip()
        if h_str and h_str.isdigit():
            return int(h_str)

    param_bits = prop_info.get('paramBits')
    if param_bits is not None:
        p_str = str(param_bits).strip()
        if p_str and p_str.isdigit():
            return int(p_str)

    return 0


def build_forward_property_bits(prop_info: Dict, prop_id: int, value: int, param: int = 0,
                                value_bits_override: int = None, param_bits_override: int = None) -> Tuple[str,int,int,int,int]:
    """Build property bits in forward storage order.

    Returns tuple: (bits_str, value_bits, param_bits, total_bits, raw_value)

    - bits_str: a MSB-first string arranged as [value][param][ID]
    - value_bits/param_bits: resolved bit widths
    - total_bits: length of bits_str
    - raw_value: value + addv (if prop_info has addv)
    """
    addv = prop_info.get('addv', 0) or 0
    raw_value = value + addv

    value_bits = value_bits_override if value_bits_override is not None else int(prop_info.get('bits', 0))
    param_bits = param_bits_override if param_bits_override is not None else get_param_bits_from_info(prop_info)

    if value_bits <= 0:
        raise ValueError(f"Invalid value bits for property {prop_id}: {value_bits}")

    # Validate ranges (caller may choose to catch ValueError)
    max_value = (1 << value_bits) - 1
    if raw_value < 0 or raw_value > max_value:
        raise ValueError(f"Value out of range for property {prop_id}: storage={raw_value}, bits={value_bits}")

    if param_bits > 0:
        max_param = (1 << param_bits) - 1
        if param < 0 or param > max_param:
            raise ValueError(f"Param out of range for property {prop_id}: param={param}, bits={param_bits}")

    # Build forward-order bitstring: [value][param][ID]
    bits = number_to_binary_msb(raw_value, value_bits)
    if param_bits > 0:
        bits += number_to_binary_msb(param, param_bits)
    bits += number_to_binary_msb(prop_id, 9)

    total_bits = len(bits)
    return bits, value_bits, param_bits, total_bits, raw_value
