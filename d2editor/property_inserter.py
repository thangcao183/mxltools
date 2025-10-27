from property_bits import build_forward_property_bits


def insert_property(item_bitstring: str, prop_id: int, value: int, value_bits: int) -> str:
    """Insert a property into the item bitstring using the canonical forward-order encoding.

    This uses build_forward_property_bits which returns a forward-order MSB-first
    string in the format: [value][param][ID]. For properties without parameters, param=0.
    """
    # Build property bits (no parameter)
    prop_info = {'bits': value_bits, 'addv': 0, 'paramBits': None, 'h_saveParamBits': None}
    prop_bits, vb, pb, total_bits, raw_value = build_forward_property_bits(prop_info, prop_id, value, 0)

    # Find the end marker in the bitstring and insert
    end_marker = '111111111'
    insertion_point = item_bitstring.find(end_marker)
    if insertion_point == -1:
        raise ValueError("End marker not found in the item bitstring")

    new_bitstring = item_bitstring[:insertion_point] + prop_bits + item_bitstring[insertion_point:]

    # Pad to byte boundary
    padding_length = (8 - len(new_bitstring) % 8) % 8
    new_bitstring += '0' * padding_length

    return new_bitstring

# Example usage
if __name__ == "__main__":
    # Example item bitstring
    example_bitstring = "00000000111111111100000000"
    prop_id = 42
    value = 5
    value_bits = 7

    updated_bitstring = insert_property(example_bitstring, prop_id, value, value_bits)
    print("Updated bitstring:", updated_bitstring)