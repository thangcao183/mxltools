#!/usr/bin/env python3
"""
Complete D2I Item Parser
========================
Full implementation of Diablo 2 item parsing logic matching C++ ItemParser.

This parser handles:
- Simple/Extended items
- All quality types (Normal, Magic, Rare, Set, Unique, Crafted, etc.)
- Item properties (both item and runeword properties)
- Personalization, sockets, gems
- Correct bit offsets for all item types
"""

import struct
from typing import List, Optional, Tuple, Dict
from dataclasses import dataclass
from enum import IntEnum


class ItemQuality(IntEnum):
    """Item quality enum matching C++ Enums::ItemQuality"""
    LowQuality = 1
    Normal = 2
    HighQuality = 3
    Magic = 4
    Set = 5
    Rare = 6
    Unique = 7
    Crafted = 8
    Honorific = 9


@dataclass
class ItemProperty:
    """Represents a single item property"""
    prop_id: int
    value: int
    param: int
    bit_offset: int  # Absolute position in bitstring where property starts (from START)
    prop_bits: int = 0  # Total bits for this property (9 + param_bits + value_bits)
    param_bits: int = 0  # Number of bits for parameter
    value_bits: int = 0  # Number of bits for value


@dataclass
class ParsedItem:
    """Complete parsed item structure"""
    # File info
    file_data: bytes
    bitstring: str
    
    # Basic flags
    is_quest: bool
    is_identified: bool
    is_socketed: bool
    is_ear: bool
    is_starter: bool
    is_extended: bool
    is_ethereal: bool
    is_personalized: bool
    is_runeword: bool
    
    # Item data
    item_type: str
    location: int
    equipped_location: int
    column: int
    row: int
    
    # Extended item data (only if is_extended == True)
    num_sockets: int = 0
    guid: int = 0
    ilvl: int = 0
    quality: ItemQuality = ItemQuality.Normal
    variable_graphic_index: int = 0
    
    # Quality-specific data
    set_or_unique_id: int = 0
    
    # Armor/Weapon fields
    defense: int = 0
    max_durability: int = 0
    current_durability: int = 0
    quantity: int = -1  # -1 means not stackable
    
    # Properties
    properties: List[ItemProperty] = None
    runeword_properties: List[ItemProperty] = None
    
    # Bit positions for editing
    properties_start_offset: int = 0  # Where properties section starts (from END)
    end_marker_position: int = 0  # Where end marker (0x1FF) is located (from END)
    
    def __post_init__(self):
        if self.properties is None:
            self.properties = []
        if self.runeword_properties is None:
            self.runeword_properties = []


class BitReader:
    """
    Reads bits from a bitstring like C++ ReverseBitReader
    Starts from END and reads BACKWARDS
    """
    
    def __init__(self, bitstring: str):
        self.bitstring = bitstring
        self.pos = len(bitstring)  # Start at END
    
    def read_bool(self) -> bool:
        """Read 1 bit as boolean"""
        if self.pos < 1:
            raise ValueError(f"BitReader: attempt to read past beginning (pos={self.pos})")
        self.pos -= 1
        bit = self.bitstring[self.pos] == '1'
        return bit
    
    def read_number(self, num_bits: int) -> int:
        """Read num_bits as an integer"""
        if self.pos < num_bits:
            raise ValueError(f"BitReader: attempt to read {num_bits} bits at pos {self.pos}")
        
        self.pos -= num_bits
        bits = self.bitstring[self.pos:self.pos + num_bits]
        
        # Bits are already in MSB order, just parse
        return int(bits, 2) if bits else 0
    
    def skip(self, num_bits: int = 1):
        """Skip num_bits"""
        self.pos -= num_bits
    
    def get_pos(self) -> int:
        """Get current position (from end)"""
        return len(self.bitstring) - self.pos
    
    def get_absolute_pos(self) -> int:
        """Get absolute position in bitstring"""
        return self.pos
    
    def seek(self, pos: int):
        """Seek to position (from start)"""
        self.pos = len(self.bitstring) - pos


def byte_to_binary_msb(byte: int) -> str:
    """Convert byte to MSB-first binary string (matching C++ binaryStringFromNumber)"""
    return format(byte, '08b')


def create_bitstring_from_bytes(data: bytes) -> str:
    """
    Create bitstring from bytes (skip JM header)
    Uses PREPEND logic like C++ to match ReverseBitReader behavior
    """
    bitstring = ""
    # Process bytes after JM header, PREPENDING each byte (reverse order)
    for i in range(2, len(data)):
        bitstring = byte_to_binary_msb(data[i]) + bitstring  # PREPEND!
    return bitstring


def number_to_binary_msb(value: int, num_bits: int, addv: int) -> str:
    """Convert number to MSB-first binary string with specified bit length"""
    if value < 0:
        raise ValueError(f"Value must be non-negative: {value}")
    
    max_value = (1 << num_bits) - 1 - addv  # 2^num_bits - 1
    if value > max_value:
        raise ValueError(f"Value {value} exceeds maximum for {num_bits} bits ({max_value})")
    
    return format(value, f'0{num_bits}b')


# Item database cache
_ITEM_TYPES_CACHE = {}
_ITEM_TYPE_HIERARCHY_CACHE = {}
_PROPERTY_METADATA_CACHE = {}


def get_item_types(item_code: str, db_path: str = '../d2_items.db') -> List[str]:
    """Get item types from database"""
    global _ITEM_TYPES_CACHE
    
    if item_code in _ITEM_TYPES_CACHE:
        return _ITEM_TYPES_CACHE[item_code]
    
    try:
        import sqlite3
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT type FROM items WHERE code = ?", (item_code,))
        row = cursor.fetchone()
        conn.close()
        
        if row and row[0]:
            types = [t.strip() for t in row[0].split(',')]
            _ITEM_TYPES_CACHE[item_code] = types
            return types
    except Exception as e:
        print(f"Warning: Could not load item types for {item_code}: {e}")
    
    _ITEM_TYPES_CACHE[item_code] = []
    return []


def get_item_stackable(item_code: str, db_path: str = '../d2_items.db') -> bool:
    """Check if item is stackable from database"""
    try:
        import sqlite3
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT stackable FROM items WHERE code = ?", (item_code,))
        row = cursor.fetchone()
        conn.close()
        
        if row and row[0]:
            return bool(int(row[0]))
    except Exception as e:
        print(f"Warning: Could not check stackable for {item_code}: {e}")
    
    return False


def item_types_inherit_from_type(item_types: List[str], parent_type: str) -> bool:
    """
    Check if any of the item types inherits from parent_type.
    
    This is a simplified implementation that checks:
    1. Direct match in types list
    2. Known armor parent types: armo, tors, helm, shie, glov, boot, belt
    3. Known weapon parent types: weap, swor, axe, mace, pole, bow, xbow, staf, wand, knif, spea, etc.
    
    For full hierarchy resolution, would need to recursively check ItemTypes table.
    """
    if parent_type in item_types:
        return True
    
    # Define known type hierarchies
    armor_types = ['armo', 'tors', 'helm', 'shie', 'glov', 'boot', 'belt', 'pelt', 'phlm', 
                   'ashd', 'pala', 'head', 'circ', 'cr', 'ba', 'dr', 'bhlm', 'bshi', 'btor',
                   'hlms', 'shld', 'tow', 'kite', 'smal']
    weapon_types = ['weap', 'swor', 'axe', 'mace', 'pole', 'bow', 'xbow', 'staf', 'wand',
                    'knif', 'spea', 'jave', 'club', 'scep', 'hamm', 'h2h', 'orb',
                    'bswd', 'baxe', 'bmac', 'bpol', 'bowq', 'xboq', 'bstf', 'rod',
                    'tkni', 'taxe', 'jav', 'abow', 'aspe']
    
    if parent_type == 'armo':
        return any(t in armor_types for t in item_types)
    elif parent_type == 'weap':
        return any(t in weapon_types for t in item_types)
    
    return False


def get_property_metadata(prop_id: int, db_path: str = 'data/props.db') -> Optional[Dict]:
    """Get property metadata from database"""
    global _PROPERTY_METADATA_CACHE
    
    if prop_id in _PROPERTY_METADATA_CACHE:
        return _PROPERTY_METADATA_CACHE[prop_id]
    
    try:
        import sqlite3
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT addv, bits FROM props WHERE code = ?", (prop_id,))
        row = cursor.fetchone()
        conn.close()
        
        if row:
            metadata = {
                'add': row[0] if row[0] is not None else 0,
                'bits': row[1] if row[1] is not None else 0
            }
            _PROPERTY_METADATA_CACHE[prop_id] = metadata
            return metadata
    except Exception as e:
        print(f"Warning: Could not load property metadata for {prop_id}: {e}")
    
    return None


class D2ItemParser:
    """Complete D2 item parser"""
    
    def __init__(self, property_db: Dict):
        """
        Args:
            property_db: Dictionary of property definitions
                Format: {prop_id: {'name': ..., 'bits': ..., 'paramBits': ..., 'h_saveParamBits': ..., 'addv': ...}}
        """
        self.property_db = property_db
    
    def parse_file(self, filename: str) -> ParsedItem:
        """Parse a D2I file completely"""
        with open(filename, 'rb') as f:
            data = f.read()
        
        # Check JM header
        if len(data) < 2 or data[0:2] != b'JM':
            raise ValueError("Invalid D2I file: missing JM header")
        
        # Create bitstring
        bitstring = create_bitstring_from_bytes(data)
        
        # Create BitReader
        reader = BitReader(bitstring)
        
        # Parse item
        return self.parse_item(data, bitstring, reader)
    
    def save_file(self, item: ParsedItem, filename: str):
        """
        Save a ParsedItem to a D2I file
        Simply writes the current bitstring back to file
        """
        # Convert bitstring to bytes (reverse PREPEND)
        data = self.bitstring_to_bytes(item.bitstring)
        
        # Write to file with JM header
        with open(filename, 'wb') as f:
            f.write(b'JM')
            f.write(data)
        
        print(f"✅ Saved to {filename}")
    

    
    def modify_property(self, item: ParsedItem, prop_index: int, new_value: int = None, new_param: int = None, runeword: bool = False):
        """
        Modify a property's value and/or param inline in the bitstring.
        Re-parses the bitstring to find accurate property position.
        """
        # Choose which property list to operate on
        prop_list = item.runeword_properties if runeword else item.properties
        if prop_index < 0 or prop_index >= len(prop_list):
            raise ValueError(f"Invalid property index: {prop_index} ({'runeword' if runeword else 'item'})")

        prop = prop_list[prop_index]
        prop_info = self.property_db.get(prop.prop_id, {})
        if not prop_info:
            raise ValueError(f"Property {prop.prop_id} not found in database")
        
        # Use existing values if not specified
        if new_value is None:
            new_value = prop.value
        if new_param is None:
            new_param = prop.param
        prop_bits_total = prop.prop_bits
        prop_start = getattr(prop, 'bit_offset', None)

        if prop_start is None:
            raise ValueError(f"Property {prop.prop_id} missing bit_offset; cannot modify reliably")

        # Validate range
        if not (0 <= prop_start <= len(item.bitstring) - prop_bits_total):
            raise ValueError(f"Property bit_offset out of range: {prop_start} (bits needed: {prop_bits_total}, bitstring len: {len(item.bitstring)})")
        # Build new property bitstring using centralized helper
        from property_bits import build_forward_property_bits

        # Prefer per-property recorded bit sizes when available (safer if DB differs)
        value_bits_override = prop.value_bits if getattr(prop, 'value_bits', 0) else None
        param_bits_override = prop.param_bits if getattr(prop, 'param_bits', 0) else None

        new_prop_bits, vb, pb, total_bits, raw_value = build_forward_property_bits(
            prop_info, prop.prop_id, new_value, new_param,
            value_bits_override=value_bits_override,
            param_bits_override=param_bits_override
        )

        # Verify length matches recorded prop length
        if total_bits != prop_bits_total:
            raise ValueError(f"Property bit length mismatch: expected {prop_bits_total}, got {total_bits}")

        # Replace bits in bitstring (use stored python start index)
        item.bitstring = item.bitstring[:prop_start] + new_prop_bits + item.bitstring[prop_start + prop_bits_total:]

        # Update property object
        prop.value = new_value
        prop.param = new_param

        print(f"Modified property {prop.prop_id} ({prop_info.get('name', 'unknown')}): value={new_value}, param={new_param}")

    
    def add_property_to_item(self, item: ParsedItem, prop_id: int, value: int, param: int = 0):
        """
        Add a new property to item (like add_property_with_param.py)
        Inserts before end marker
        """
        prop_info = self.property_db.get(prop_id)
        if not prop_info:
            raise ValueError(f"Property {prop_id} not found in database")
        
        # Build bits using shared helper to ensure consistent encoding
        from property_bits import build_forward_property_bits

        prop_bits, value_bits, param_bits, total_bits, raw_value = build_forward_property_bits(
            prop_info, prop_id, value, param
        )

        # Find end marker position (start index in bitstring)
        insert_pos = item.end_marker_position

        # prop_bits was already validated inside build_forward_property_bits

        # Determine insertion index in the properties list so we keep the
        # same ordering as parse_properties (parser appends properties while
        # reading backwards from end). That results in a list ordered by
        # descending bit_offset (largest first). To maintain that order,
        # find the first property with bit_offset < insert_pos and insert
        # before it; if none found, append at the end.
        insertion_index = len(item.properties)
        for i, p in enumerate(item.properties):
            if p.bit_offset < insert_pos:
                insertion_index = i
                break

        # Shift offsets of properties (item + runeword) that come after the insertion point
        total_bits = 9 + param_bits + value_bits
        for p in item.properties:
            if p.bit_offset >= insert_pos:
                p.bit_offset += total_bits

        for p in item.runeword_properties:
            if p.bit_offset >= insert_pos:
                p.bit_offset += total_bits

        # Insert the bits into the bitstring
        item.bitstring = item.bitstring[:insert_pos] + prop_bits + item.bitstring[insert_pos:]

        # Create ItemProperty object and insert at correct position
        new_prop = ItemProperty(
            prop_id=prop_id,
            value=value,
            param=param,
            bit_offset=insert_pos,
            prop_bits=total_bits,
            param_bits=param_bits,
            value_bits=value_bits
        )

        item.properties.insert(insertion_index, new_prop)

        # Update end marker position (moved by property length)
        item.end_marker_position += total_bits

        print(f"Added property {prop_id} ({prop_info['name']}): value={value}, param={param}")
    
    def delete_property_from_item(self, item: ParsedItem, prop_index: int):
        """
        Delete a property from item by removing its bits
        """
        if prop_index < 0 or prop_index >= len(item.properties):
            raise ValueError(f"Invalid property index: {prop_index}")

        prop = item.properties[prop_index]

        # Capture deleted bits and offset before modifying lists
        deleted_bits = prop.prop_bits
        deleted_offset = prop.bit_offset

        # Calculate position from start
        bit_pos_from_start = deleted_offset

        # Remove the property bits from the bitstring
        new_bitstring = (item.bitstring[:bit_pos_from_start] + 
                         item.bitstring[bit_pos_from_start + deleted_bits:])

        item.bitstring = new_bitstring

        # Remove from property list
        del item.properties[prop_index]

        # Update end marker position (moved back by property length)
        item.end_marker_position -= deleted_bits

        # BUGFIX: update offsets for remaining properties (both item and runeword)
        for p in item.properties:
            if p.bit_offset > deleted_offset:
                p.bit_offset -= deleted_bits

        for p in item.runeword_properties:
            if p.bit_offset > deleted_offset:
                p.bit_offset -= deleted_bits

        print(f"Deleted property at index {prop_index}")
    

    def bitstring_to_bytes(self, bitstring: str) -> bytes:
        """
        Convert bitstring back to bytes (reverse of create_bitstring_from_bytes)
        ...
        """
        # Pad to byte boundary
        remainder = len(bitstring) % 8
        if remainder != 0:
            # SỬA LỖI: Pad vào ĐẦU bitstring (tương ứng với byte CUỐI của file)
            bitstring = '0' * (8 - remainder) + bitstring
        
        # Split into 8-bit chunks (forward order)
        chunks = []
        for i in range(0, len(bitstring), 8):
            chunks.append(bitstring[i:i+8])
        
        # Reverse chunk order (to match PREPEND)
        chunks.reverse()
        
        # Convert each chunk to byte
        bytes_list = []
        for chunk in chunks:
            byte_val = int(chunk, 2)
            bytes_list.append(byte_val)
        
        return bytes(bytes_list)
    
    
    def parse_item(self, data: bytes, bitstring: str, reader: BitReader) -> ParsedItem:
        """Parse item from bitstring"""
        
        item = ParsedItem(
            file_data=data,
            bitstring=bitstring,
            is_quest=False,
            is_identified=False,
            is_socketed=False,
            is_ear=False,
            is_starter=False,
            is_extended=False,
            is_ethereal=False,
            is_personalized=False,
            is_runeword=False,
            item_type="",
            location=0,
            equipped_location=0,
            column=0,
            row=0
        )
        
        # Parse flags (first ~102 bits)
        item.is_quest = reader.read_bool()
        reader.skip(3)
        item.is_identified = reader.read_bool()
        reader.skip(5)
        reader.skip()  # is_duped
        item.is_socketed = reader.read_bool()
        reader.skip(2)
        reader.skip(2)  # illegal_equip + unk
        item.is_ear = reader.read_bool()
        item.is_starter = reader.read_bool()
        reader.skip(2)
        reader.skip()
        
        # Extended vs simple
        simple_item = reader.read_bool()
        item.is_extended = not simple_item
        
        item.is_ethereal = reader.read_bool()
        reader.skip()
        item.is_personalized = reader.read_bool()
        reader.skip()
        item.is_runeword = reader.read_bool()
        reader.skip(5)
        reader.skip(8)  # version - should be 101
        reader.skip(2)
        
        # Location info
        item.location = reader.read_number(3)
        item.equipped_location = reader.read_number(4)
        item.column = reader.read_number(4)
        item.row = reader.read_number(4)
        reader.skip(3)  # storage field
        
        # Check for ear
        if item.is_ear:
            print("Item is an ear - not supported")
            return item
        
        # Read item type (4 characters)
        item_type_chars = []
        for i in range(4):
            char_code = reader.read_number(8)
            if char_code != 0:  # Skip null bytes
                item_type_chars.append(chr(char_code))
        item.item_type = ''.join(item_type_chars).strip()
        
        print(f"Item type: '{item.item_type}', is_extended: {item.is_extended}")
        
        if item.is_extended:
            # Extended item - parse additional fields
            item.num_sockets = reader.read_number(3)
            item.guid = reader.read_number(32)
            item.ilvl = reader.read_number(7)
            item.quality = ItemQuality(reader.read_number(4))
            
            print(f"Extended item: sockets={item.num_sockets}, guid={item.guid}, ilvl={item.ilvl}, quality={item.quality.name}")
            
            # Variable graphic
            has_graphic = reader.read_bool()
            if has_graphic:
                item.variable_graphic_index = reader.read_number(3) + 1
            
            # Autoprefix
            has_autoprefix = reader.read_bool()
            if has_autoprefix:
                reader.skip(11)
            
            # Quality-specific parsing
            if item.quality == ItemQuality.Normal:
                pass  # No extra data
            
            elif item.quality in [ItemQuality.LowQuality, ItemQuality.HighQuality]:
                reader.skip(3)  # non_magic_type
            
            elif item.quality == ItemQuality.Magic:
                reader.skip(22)  # prefix & suffix
            
            elif item.quality in [ItemQuality.Set, ItemQuality.Unique]:
                item.set_or_unique_id = reader.read_number(15)
                print(f"Set/Unique ID: {item.set_or_unique_id}")
            
            elif item.quality in [ItemQuality.Rare, ItemQuality.Crafted]:
                reader.skip(16)  # first & second names
                for i in range(6):
                    if reader.read_bool():
                        reader.skip(11)  # prefix or suffix
            
            elif item.quality == ItemQuality.Honorific:
                reader.skip(16)
            
            # Runeword
            if item.is_runeword:
                reader.skip(16)  # RW code
            
            # Personalization
            if item.is_personalized:
                for i in range(16):
                    char_code = reader.read_number(7)
                    if char_code == 0:
                        break
            
            # Tome of ID bit (always present)
            reader.skip()
            
            # Item-specific fields (defense, durability, quantity)
            # Get item types from database to determine what fields to parse
            item_types = get_item_types(item.item_type)
            is_armor = item_types_inherit_from_type(item_types, 'armo')
            is_weapon = item_types_inherit_from_type(item_types, 'weap')
            is_stackable = get_item_stackable(item.item_type)
            
            # Defense (armor only)
            if is_armor:
                defense_prop = get_property_metadata(31)  # Defense property ID = 31
                if defense_prop:
                    raw_defense = reader.read_number(defense_prop['bits'])
                    item.defense = raw_defense - defense_prop['add']
                    print(f"Armor defense: {item.defense} (raw: {raw_defense}, bits: {defense_prop['bits']}, add: {defense_prop['add']})")
            
            # Durability (armor and weapon)
            if is_armor or is_weapon:
                max_dur_prop = get_property_metadata(73)  # MaxDurability property ID = 73
                if max_dur_prop:
                    raw_max_dur = reader.read_number(max_dur_prop['bits'])
                    item.max_durability = raw_max_dur - max_dur_prop['add']
                    print(f"Max durability: {item.max_durability} (raw: {raw_max_dur}, bits: {max_dur_prop['bits']})")
                    
                    # Current durability (only if max_durability > 0)
                    if item.max_durability > 0:
                        dur_prop = get_property_metadata(72)  # Durability property ID = 72
                        if dur_prop:
                            raw_dur = reader.read_number(dur_prop['bits'])
                            item.current_durability = raw_dur - dur_prop['add']
                            print(f"Current durability: {item.current_durability} (raw: {raw_dur}, bits: {dur_prop['bits']})")
                            
                            # Sanity check
                            if item.max_durability < item.current_durability:
                                item.max_durability = item.current_durability
            
            # Quantity (stackable items only)
            if is_stackable:
                item.quantity = reader.read_number(9)
                print(f"Stackable item quantity: {item.quantity}")
            
            # Sockets number (if socketed)
            if item.is_socketed:
                actual_sockets = reader.read_number(4)
                print(f"Socketed item: actual sockets = {actual_sockets}")
            
            # Set lists (if set quality)
            if item.quality == ItemQuality.Set:
                reader.skip(5)  # 5 bool flags for set bonuses
        
        else:
            # Simple item - just has item code
            print("Simple item (not extended)")
        
        # Now we should be at properties section
        item.properties_start_offset = reader.get_absolute_pos()
        print(f"Properties should start at bit offset: {reader.get_pos()} (absolute pos from end: {item.properties_start_offset})")
        
        # Parse item properties
        self._last_end_marker_pos = None
        item.properties = self.parse_properties(reader, "Item")
        if self._last_end_marker_pos is not None:
            item.end_marker_position = self._last_end_marker_pos
        
        # Parse runeword properties (if runeword)
        if item.is_runeword:
            item.runeword_properties = self.parse_properties(reader, "Runeword")
        
        return item
    
    def parse_properties(self, reader: BitReader, prop_type: str) -> List[ItemProperty]:
        """Parse property list"""
        properties = []
        
        print(f"\n=== Parsing {prop_type} Properties ===")
        print(f"Starting at bit offset: {reader.get_pos()}")
        
        while True:
            # Save position before reading property ID
            prop_start_pos = reader.get_absolute_pos()
            
            # Read property ID (9 bits)
            if reader.get_absolute_pos() < 9:
                print("Not enough bits for property ID - end of properties")
                break
            
            prop_id = reader.read_number(9)
            
            # Check for end marker (0x1FF = 511)
            if prop_id == 0x1FF:
                end_marker_pos = prop_start_pos
                print(f"Found end marker at bit offset {reader.get_pos() - 9} (absolute: {end_marker_pos})")
                # Store end marker position in item (will be set by caller)
                self._last_end_marker_pos = end_marker_pos
                break
            
            # Get property info from database
            if prop_id not in self.property_db:
                print(f"⚠️  Unknown property ID {prop_id} at offset {reader.get_pos() - 9}")
                break
            
            prop_info = self.property_db[prop_id]
            
            # Read parameter (if exists)
            param_bits = self.get_param_bits(prop_info)
            param = 0
            
            if param_bits > 0:
                param = reader.read_number(param_bits)
            
            # Read value
            value_bits = prop_info['bits']
            raw_value = reader.read_number(value_bits)
            
            # Convert to display value
            display_value = raw_value - prop_info['addv']
            
            # Calculate total bits for this property
            total_prop_bits = 9 + param_bits + value_bits
            
            prop = ItemProperty(
                prop_id=prop_id,
                value=display_value,
                param=param,
                bit_offset=reader.get_absolute_pos(),  # absolute start position of this property (from START)
                prop_bits=total_prop_bits,
                param_bits=param_bits,
                value_bits=value_bits
            )
            
            properties.append(prop)
            
            param_str = f", param={param}" if param_bits > 0 else ""
            print(f"Property {prop_id} ({prop_info['name']:30s}): value={display_value:4d} (raw={raw_value}){param_str} @ bit {prop_start_pos}")
        
        print(f"Total {prop_type} properties: {len(properties)}\n")

        return properties
    
    def get_param_bits(self, prop_info: Dict) -> int:
        """Get parameter bits for a property"""
        # Try h_saveParamBits first (for runeword properties)
        h_save = prop_info.get('h_saveParamBits')
        if h_save is not None:
            h_save_str = str(h_save).strip()
            if h_save_str and h_save_str != '' and h_save_str.isdigit():
                return int(h_save_str)
        return 0


def main():
    """Test the parser"""
    import sqlite3
    
    # Load property database
    property_db = {}
    conn = sqlite3.connect('data/props.db')
    cursor = conn.cursor()
    cursor.execute("SELECT code, name, h_descNegative, h_descStringAdd, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0")
    
    for row in cursor.fetchall():
        code, name, h_descNegative, h_descStringAdd, addv, bits, paramBits, h_save_param_bits = row
        
        # Clean up empty strings
        if paramBits == '':
            paramBits = None
        if h_save_param_bits == '':
            h_save_param_bits = None
        
        property_db[code] = {
            'name': (name or '') + " " + (h_descNegative or '') + " " + (h_descStringAdd or '') or f'prop_{code}',
            'addv': addv if addv is not None else 0,
            'bits': bits,
            'h_saveParamBits': h_save_param_bits
        }
    
    conn.close()
    print(f"✅ Loaded {len(property_db)} properties from database\n")
    
    # Parse test file or provided file from command line
    import sys
    parser = D2ItemParser(property_db)

    if len(sys.argv) < 2:
        test_file = 'd2i/complete/relic_fungus_modified.d2i'
        print(f"No file provided, using default: {test_file}")
    else:
        test_file = sys.argv[1]

    print(f"Parsing: {test_file}")
    print("=" * 80)
    
    try:
        item = parser.parse_file(test_file)
        
        print("\n" + "=" * 80)
        print("PARSING RESULTS:")
        print("=" * 80)
        print(f"Item Type: {item.item_type}")
        print(f"Extended: {item.is_extended}")
        print(f"Quality: {item.quality.name if item.is_extended else 'N/A'}")
        print(f"Identified: {item.is_identified}")
        print(f"Ethereal: {item.is_ethereal}")
        print(f"Socketed: {item.is_socketed} (sockets: {item.num_sockets})")
        print(f"Runeword: {item.is_runeword}")
        
        # Display armor/weapon stats
        if item.defense > 0:
            print(f"Defense: {item.defense}")
        if item.max_durability > 0:
            print(f"Durability: {item.current_durability}/{item.max_durability}")
        if item.quantity >= 0:
            print(f"Quantity: {item.quantity}")
        
        print(f"\nItem Properties: {len(item.properties)}")
        
        for i, prop in enumerate(item.properties):
            prop_info = property_db.get(prop.prop_id, {})
            param_str = f", param={prop.param}" if prop.param > 0 else ""
            print(f"  {i+1}. ID {prop.prop_id} ({prop_info.get('name', 'unknown'):30s}): value={prop.value}{param_str}")
        
        if item.is_runeword and item.runeword_properties:
            print(f"\nRuneword Properties: {len(item.runeword_properties)}")
            for i, prop in enumerate(item.runeword_properties):
                prop_info = property_db.get(prop.prop_id, {})
                param_str = f", param={prop.param}" if prop.param > 0 else ""
                print(f"  {i+1}. ID {prop.prop_id} ({prop_info.get('name', 'unknown'):30s}): value={prop.value}{param_str}")

        # parser.modify_property(item, 0, new_value=50)
        # parser.save_file(item, "d2i/complete/relic_fungus_modified2.d2i")
        
        
    except Exception as e:
        print(f"\n❌ Error parsing file: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
