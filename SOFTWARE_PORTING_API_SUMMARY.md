# Software Porting API Summary

Purpose
- A concise developer-facing map of the non-UI C++ backend in this repo to help port the project to Python + web UI.
- Focus: functions, classes, data shapes, flows, files to port, and concrete suggestions for Python equivalents.

How to use this document
- Read top-to-bottom for an overview, then see the "File-by-file" section when implementing modules in Python.
- This intentionally excludes UI code; it documents parsing, databases, item representation, creation, property modification, socketables/runewords.

## High-level architecture (C++)
- Save file reading/parsing pipeline:
  D2 save binary -> ItemParser.parseItemsToBuffer / ItemParser.parseItem -> ItemInfo (in-memory)
- Databases and definitions live in `ItemDataBase` (items, properties, runewords, socketables, sets, uniques)
- Display / property formatting in `PropertiesDisplayManager` (constructs human-friendly strings)
- Core mutators:
  - `PropertyModificationEngine`: validate/replace item properties and reconstruct item bit-strings
  - Item creation (guides and ItemCreationEngine pattern in `ITEM_CREATION_GUIDE.md`): create ItemInfo + generate bitString
  - Socketing / Runeword logic: handled by ItemParser when parsing nested socketables and by socketable data in ItemDataBase
- Low-level bit ops: `ReverseBitReader` (reading bits from the JM bit-stream) and `ReverseBitWriter` (inserting/replacing bits)

## Key data structures (from `src/structs.h`)
- ItemProperty
  - fields: int bitStringOffset, int value, quint32 param, QString displayString
- PropertiesMultiMap
  - typedef QMultiMap<int, ItemProperty*> (maps propertyId -> ItemProperty*)
- ItemInfo
  - Flags: isQuest, isIdentified, isSocketed, isExtended, isRW, isEthereal, isPersonalized, isEar, etc.
  - Location/coords: location, whereEquipped, row, column, storage
  - Extended section: guid, socketablesNumber, ilvl, quality, variableGraphicIndex
  - Gameplay fields: defense, currentDurability, maxDurability, quantity, socketsNumber
  - props, rwProps: PropertiesMultiMap
  - socketablesInfo: ItemsList (nested ItemInfo for gems/runes inserted into the host item)
  - bitString: QString - the JM bit-string representation for writing back to save file
- ItemBase
  - Static per-item metadata from `items` data: name, width/height, genericType, isStackable, rlvl, rstr, types (QList<QByteArray>), socketableType (armor/shield/weapon), etc.
- RunewordInfo, SocketableItemInfo, SetItemInfo, UniqueItemInfo: small containers loaded from compressed txt/dat files

## File-by-file summary (non-UI code only)
- `src/itemparser.h/.cpp`
  - Responsibilities: full D2 item parse + writing. Key functions:
    - parseItemsToBuffer(itemsTotal, QDataStream&, bytes, corruptedItemFormat, itemsBuffer, plugyPage)
    - parseItem(QDataStream&, bytes, isLastItemOnPlugyPage)
    - parseItemProperties(ReverseBitReader&, ItemInfo::ParsingStatus*) -> PropertiesMultiMap
    - createDisplayStringForPropertyWithId(int id, ItemProperty*)
    - writeItems(const ItemsList&, QDataStream&) — serializes JM bit-strings to stream
    - Helpers: itemTypesInheritFromType / itemTypesInheritFromTypes, itemStorageAndCoordinatesString
  - Notes: parseItem reads the JM header, many boolean flags, extended data and then calls parseItemProperties.
  - Runeword detection: parseItem collects socketable itemTypes and uses ItemDataBase::RW() to map rune sequences to runewords.

- `src/itemdatabase.h/.cpp`
  - Responsibilities: load all text/binary datafiles used to interpret items and properties.
  - Primary providers (singletons returning static maps):
    - Items() -> QHash<itemCode, ItemBase*>
    - Properties() -> QHash<propId, ItemPropertyTxt*>
    - RW() -> RunewordHash
    - Socketables() -> QHash<socketableCode, SocketableItemInfo*>
    - Sets(), Uniques(), Skills(), Mystics, Monsters
  - Data loader patterns: files stored compressed; `decompressedFileData()` unwraps and validates CRCs.
  - Important: `Socketables()` expects a datafile with columns for Armor/Shield/Weapon property triples.

- `src/structs.h`
  - Central place with ItemInfo, ItemBase, ItemPropertyTxt, SocketableItemInfo, RunewordInfo typedefs.
  - Useful when mapping to Python dataclasses.

- `src/reversebitreader.h` (and implementation)
  - ReverseBitReader: bit-level reader used to parse item properties (reads numbers from a MSB->LSB string built for JM blocks).
  - API: readNumber(int length), readBool(), skip(), pos(), notReadBits()

- `src/reversebitwriter.h` (and implementation)
  - Utilities for in-place bit manipulations and small helpers for updating row/column bits in existing item strings.
  - API: replaceValueInBitString(...), updateItemRow(ItemInfo*), updateItemColumn(ItemInfo*), remove/insert/byteAlignBits

- `src/propertymodificationengine.h/.cpp`
  - Top-level engine to replace an ItemInfo's properties safely.
  - API (public): modifyItemProperties(ItemInfo *item, const PropertiesMultiMap &newProperties), reconstructItemBitString(ItemInfo*), validateProperty/validatePropertyCombination
  - Internal flow:
    - Validate property set using ItemDataBase::Properties() (bits/param limits)
    - Backup original props; clear them and insert new props
    - buildPropertiesBitString(...) to produce properties section bit-string
    - calculatePropertiesStartPosition(item) to find where to splice in new properties
    - assemble: beforeProperties + propertiesBits + afterProperties; byte align; assign to item->bitString
    - In case of significant discrepancy, error and restore backup
  - Important special-case handlers for properties like EnhancedDamage and elemental damage (min/max/length triplets)

- `src/propertiesdisplaymanager.h/.cpp`
  - Formatting and merging of properties for display. Also resolves socketableProperties where a socketable has a different effect in armor/shield/weapon contexts.
  - Key methods: completeItemDescription(ItemInfo*), constructPropertyStrings(const PropertiesMultiMap&, outMap), socketableProperties(ItemInfo*, socketableType), genericSocketableProperties(...)

- Item creation guidance (docs/ITEM_CREATION_GUIDE.md)
  - Not a single compiled `itemcreationengine.cpp` in repo, but the guide provides a full implementation pattern for:
    - ItemCreationTemplate struct
    - generateItemHeader() — building header bits, extended fields, inserting defense/durability/quantity, inserting socket count
    - generatePropertiesBitString() — iterate properties and produce 9-bit propId + paramBits + valueBits; handle EnhancedDamage duplication
    - generateUniqueGuid() helper
  - This guide is authoritative for porting the create-item flow.

- Socketables / Runewords
  - `ItemDataBase::Socketables()` loads socketable definitions used by `PropertiesDisplayManager::socketableProperties()` and by runeword formation.
  - Runewords: `ItemDataBase::RW()` -> RunewordHash where keys are concatenated socketable itemCodes; ItemParser uses that to detect RW names.

## Important file/data formats and invariants
- JM block representation (bit-string) used by parser and writer:
  - Starts with flags (booleans about identified, extended, socketed, RW, etc.)
  - Location/whereEquipped/column/row/storage bits
  - 4 chars itemCode (32 bits)
  - If extended: socketablesNumber (3 bits), guid (32 bits), ilvl (7), quality(4), variableGraphicIndex (opt), autoprefix bits, quality-specific segments, personalized name (7-bit-per-char), optional defense/durability/quantity, socket count
  - Properties section: repetition of records: [propId (9 bits)] [param (prop.paramBits)] [value (prop.bits + prop.add correction)] ... repeat until end marker propId == 511 (0x1FF represented as 9 bits of 1).
  - Byte alignment: final bit-string is padded to full bytes when writing to file.

- Properties rules
  - Each property has ItemPropertyTxt that defines paramBits and bits, and an 'add' offset. To write a property: write id (9 bits), write param (paramBits), write (value + add) in bits.
  - Special properties must be written with extra rules (enhanced damage stored twice; elemental damage has min/max and optional length)

## Recommended Python port structure
- Core packages/modules (one .py file each)
  - models.py
    - dataclasses: ItemProperty, ItemBase, ItemInfo, ItemPropertyTxt, RunewordInfo, SocketableItemInfo
  - databank.py
    - functions: load_items(), load_props(), load_rw(), load_socketables(), accessors: get_item(code), get_property(id), get_socketable(code)
    - Keep same in-memory data shapes (dicts instead of QHash)
  - bitio.py
    - ReverseBitReader and ReverseBitWriter equivalents; use `bitarray` or `bitstring` for efficiency
    - read/write helpers: read_number(msb-first), write_number, byte_align
  - parser.py
    - parse_items(bytes) -> list[ItemInfo]
    - parse_item(bitstream) -> ItemInfo
    - parse_properties(reader) -> dict/list of ItemProperty
    - write_items(items) -> bytes
  - modify.py
    - functions: validate_property(propId, value, param), modify_item_properties(item, new_props), reconstruct_item_bitstring(item)
  - create.py
    - item creation templates and bitstring generation (from ITEM_CREATION_GUIDE.md)
  - socketable.py
    - socketable property expansion and runeword checks
  - webapi.py
    - FastAPI app exposing endpoints
      - POST /parse_save -> returns parsed items JSON
      - POST /modify_item -> accepts item JSON + new properties -> returns updated item and modified save bytes
      - POST /create_item -> returns new item JSON (and bitstring)
      - POST /socket_item -> socket a gem/rune into an item

- Data storage
  - Keep the repo's compressed txt/dat resources. Create a small loader that mirrors `decompressedFileData()` behavior (zlib qUncompress equivalent) and CSV/TSV parsing.

- Libraries to use
  - bit handling: `bitstring` (pip install bitstring) or `bitarray`
  - web framework: `FastAPI` (async-friendly) or `Flask` for simpler usage
  - data modeling: `pydantic` (data validation) or Python `dataclasses`
  - background tasks: `Celery` or `ThreadPoolExecutor` for heavy operations like writing large save files

## API contracts (recommended)
- parse_save
  - Input: raw save file bytes or base64; optional: plugy page index
  - Output: list of items with full ItemInfo fields (flags, coords, bitString hex/base64, props array)
- create_item
  - Input: JSON describing ItemCreationTemplate
  - Output: created ItemInfo JSON with bitstring
- modify_item_properties
  - Input: ItemInfo JSON (or reference to item in save) + new properties array
  - Output: success flag, updated ItemInfo JSON, updated partial save bytes for replacement
- socket_item
  - Input: host item id/ref, socketable item JSON
  - Output: updated host ItemInfo and bitstring

## Edge cases & gotchas to handle while porting
- Bit ordering: C++ code builds bitstrings with string prepend/append conventions; ensure bit-endian consistency. Use an established bit library.
- Param and add offsets: every property has an 'add' offset recorded in `props` data; value storage uses (value + add).
- Special properties: enhanced damage, elemental damage (3-field groups), skill-related properties (contain param for skill id)
- Socketables: nested ItemInfo objects — the parser treats socketables as separate items nested inside the host JM block
- Runeword detection: keys are concatenated socketable codes in parse order; order matters
- Data CRC/decompression: data files in resources are compressed + stored CRCs; keep loader that verifies integrity
- Defensive programming: the current C++ code attempts fallbacks when structure detection fails (PropertyModificationEngine has a fallback propertiesStart). Preserve that resilience to avoid corrupting saves.

## Suggested minimal test matrix (automated)
- Unit tests:
  - parse single known JM item (test bitstring round-trip: parse -> serialize -> parse equals original)
  - write/read property edge cases (max/min values, special properties)
  - socketing/unsocketing round-trip
  - runeword detection for known patterns
- Integration tests:
  - parse a full known save file (small) and compare item counts/names
  - modify a property and ensure final save bytes are valid (write to file and reload)

## Prioritized porting plan (short)
1. Implement models.py + databank.py (load compressed resources) and a simple REPL to query items
2. Implement bitio.py with ReverseBitReader equivalent and a test reading an item bitstring from existing saves
3. Implement parser.parse_item and parse_properties (core) + tests
4. Implement write_items and bitwriter utilities
5. Implement modify.modify_item_properties using the same flow as PropertyModificationEngine and test roundtrip
6. Implement create.create_item (guided by ITEM_CREATION_GUIDE.md)
7. Expose web endpoints via FastAPI and small frontend for managing saves

## Contacts / references in the repo
- Read: `ITEM_CREATION_GUIDE.md`, `UNIQUE_ITEMS_GUIDE.md`, `COMPLETE_IMPLEMENTATION_GUIDE.md`, and `SOCKETABLE_ITEMS_GUIDE.md` for more in-depth design notes and concrete code snippets.
- Key C++ files to open while porting: `src/itemparser.*`, `src/itemdatabase.*`, `src/structs.h`, `src/propertymodificationengine.*`, `src/reversebitreader.*`, `src/reversebitwriter.*`, `src/propertiesdisplaymanager.*`.

---
Generated by automated code analysis on repository `mxltools` to help port the backend to Python + web. Keep this file as the single-source summary while implementing the Python modules.
