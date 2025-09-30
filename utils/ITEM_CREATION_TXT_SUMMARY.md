# ITEM CREATION / TXT PARSER SUMMARY

This short guide explains the purpose and key columns of the files under `utils/txt_parser` and `utils/txt_parser/generated/en` so you can create or edit items that the text-parser will produce into the `generated/` TSVs.

## Purpose
- `utils/txt_parser/txt/` contains the original human-editable text sources (gems, properties, itemtypes, etc.).
- `utils/txt_parser/generated/en/` contains parser outputs (TSV) used at runtime: `items.tsv`, `props.tsv`, `socketables.tsv`, `rw.tsv`, `uniques.tsv`, `gems.tsv`.

Edit the appropriate source file in `txt/` then run the txt-parser to regenerate `generated/en/` files.

## Key generated files and primary columns

- `items.tsv` (generated)
  - columns: code, name, width, height, gentype, stackable, rlvl, rstr, rdex, damage_min, damage_max, image, sockettype, class, ...
  - purpose: defines every item base used by the C++ ItemDataBase. Used to create ItemBase records.

- `props.tsv` (generated)
  - columns: id, add, bits, bitsParamSave, bitsSave, human_text
  - purpose: the crucial property meta: how many bits to save for a property's value, and the additive `add` offset used when encoding/decoding. This is required by PropertyModificationEngine.

- `socketables.tsv` (generated)
  - columns include: code (r01, gcb, skc etc), name, weapon1code/param/min/max, armor1code/param/min/max, helm1code/param/min/max, notes
  - purpose: maps socketable items (gems, runes, skulls) to the stat codes they apply when socketed into different host types (weapon, armor, helm/shield). Used to expand socketable properties for display and to compute runewords.

- `rw.tsv` (generated)
  - columns: name, allowedTypes, rune_sequence (rXX entries)
  - purpose: runeword definitions (which runes in what order make a runeword, and what host types are allowed).

- `uniques.tsv` (generated)
  - columns: unique_index, name, rlvl, ilvl, image, ...
  - purpose: unique item registry for mapping unique index to display name and base.

- `gems.tsv` (generated)
  - expanded gem/rune/skull definitions; easier to author gem/rune modifiers in the `txt/` sources and then parse into this table.

## The most important fields for property encoding
- `props.tsv` provides `id`, `add`, and `bits` for each property.
  - When constructing a property's saved bits you must: store (value - add) using exactly `bits` bits (unsigned) unless property uses `paramBits` (see next).
  - Many properties also include `paramBits` (for properties that have an extra parameter). That mapping is in the original `properties` source and in `props.tsv` columns.

Example (from `props.tsv`):
- id: 0, add: 200, bits: 11 -> Strength property stores (strength - 200) in 11 bits.

## Steps to add a new item base and ensure the parser produces it
1. Edit the appropriate source in `utils/txt_parser/txt/` (for new gem/rune/unique/item type). Follow existing row formats. Common files:
   - `itemtypes.tsv` — new base type definitions (width/height, socket limits)
   - `items.tsv` (source) — base item rows definition (if you author by hand in txt/)
   - `gems.tsv` / `properties.tsv` — update if adding new socketable or new property types

2. Run the txt parser tool to regenerate `generated/en` files. (Project has scripts in `scripts/` like `items_fetch.command` or custom perl/python txt parsers.)

3. Inspect `utils/txt_parser/generated/en/items.tsv` and `props.tsv` to confirm the new base appears and property metadata is correct (add/bits).

4. Smoke test in the application (or unit tests):
   - Use the parser to create an `ItemInfo` from an example JM bit-string that uses the new base. Or, create a new ItemInfo with the new `ItemBase` and properties, then call the PropertyModificationEngine to serialize and re-parse it.
   - Assert round-trip: parse(item_bits) -> modify or reserialize -> parse(serialized_bits) yields the same ItemInfo fields.

## Quick example: creating a simple unique-like base
1. Find similar base in `generated/en/items.tsv` for the desired slot/size.
2. Copy and adjust fields: code, name, width, height, gentype, stats, sockettype.
3. Re-run the txt parser and check `generated/en/items.tsv` contains your new code.
4. Add a `uniques` row linking unique index to the new base if needed (edit `txt/uniques` source).

## Testing tips
- Use small round-trip tests: take an existing item's bit-string (from save), parse to ItemInfo, change a non-critical field (name/inscribed), reserialize and parse again. Compare stable fields (type code, sockets, prop ids and values). This validates bit-IO and prop `add`/`bits` choices.
- Watch for special-case properties: enhanced damage (triplets), skill blocks (skill id + level), and end marker (9-bit 0x1FF). Ensure the parser's paramBits mapping matches `properties.tsv`.

## Next steps
- Run the parser locally to regenerate generated/en files and run a small python or C++ test harness to round-trip a few sample items.
- If you'd like, I can run the parser and a smoke test now (I can update the todo list to in-progress and run the scripts). Tell me if you prefer I do that now.

---
Generated by assistant to aid item creation and txt-parser workflow.
