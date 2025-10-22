````markdown
# Phân tích hoàn chỉnh cấu trúc Item (bit-level) — Tài liệu tham khảo

Tài liệu này tổng hợp phân tích từ mã nguồn của dự án (chủ yếu từ `src/itemparser.cpp`, `src/structs.h`, `src/enums.h`, `src/reversebitreader.*`, `src/reversebitwriter.*`, `src/itemdatabase.*`). Mục tiêu: giúp bạn hiểu toàn bộ cấu trúc một item (không chỉ properties) để có thể đọc/viết/patch an toàn.

## Mục tiêu / Hợp đồng ngắn
- Input: raw bytes cho một item (phần bắt đầu bằng header `JM` — 2 bytes).
- Output: `ItemInfo` với các trường đã populate (flags, vị trí, itemType, extended fields, `props`, `rwProps`, `socketablesInfo`, ...).
- Lỗi: parser ném/đánh dấu `Corrupted` nếu đọc vượt quá bitstring hoặc gặp ID property lỗi; parser cố gắng recover bằng cách seek đến item tiếp theo.

## Tóm tắt luồng parse chính (hàm parseItem)
1. Bỏ qua 2 byte đầu `JM` (tương đương 16 bit).
2. Xác định kích thước item: tìm offset của header tiếp theo (`JM`) trong toàn bộ file bytes; nếu không tìm thấy, dùng end of bytes; với Plugy page cần xử lý `STASH` header đặc thù.
3. Đọc từng byte của vùng item, tạo `itemBitData` — lưu ý code sử dụng `itemBitData.prepend(binaryStringFromNumber(aByte))`, do đó bit string được dựng theo thứ tự ngược phù hợp với `ReverseBitReader`.
4. Tạo `ReverseBitReader bitReader(itemBitData)`.
5. Tạo `ItemInfo` với `bitReader.notReadBits()` để lưu bitString gốc.
6. Đọc các flag (lần lượt bằng readBool / skip): isQuest, 3 bit skip, isIdentified, skip 5, duped, isSocketed, skips, isEar, isStarter, ...
7. Đọc định vị và kích thước: location (3 bits), whereEquipped (4 bits), column (4 bits), row (4 bits) — nếu location == Belt, row/column tính lại theo belt dims; storage (3 bits).
8. Nếu `isEar` → parse earInfo (classCode 3 bits, level 7 bits, tên 7-bit characters up to 18) rồi set itemType = "ear" và return.
9. Đọc `itemType` (4 bytes, đọc từng 8 bits → 4 ký tự ASCII).
10. Nếu `isExtended` == true → đọc extended fields:
    - socketablesNumber (3 bits)
    - guid (32 bits)
    - ilvl (7 bits)
    - quality (4 bits)
    - optional variableGraphicIndex
    - optional autoprefix bits
    - quality-dependent fields (nonMagicType / prefix/suffix / setOrUniqueId / rare names / honorific bits)
    - RW code if isRW (16 bits)
    - inscribedName (up to 16 chars, each read with 7 bits)
    - tome/scroll bits (1 bit + maybe 5 bits)
    - armor/weapon specific basic props (defense, durability etc.) — theo `ItemDataBase::Properties()` metadata
    - quantity if stackable (9 bits)
    - socketsNumber if isSocketed (4 bits)
11. Sau khi đọc các trường cơ bản và extended, parser gọi:
    item->props = parseItemProperties(bitReader, &status);
    → Vì vậy properties luôn nằm ngay sau các trường basic/extended và trước socketables.
12. Nếu item->quality == Set → parser có thể parse set list properties (kết quả của `parseItemProperties()` lại được gọi để đọc setProps).
13. Nếu item->isRW → parser tiếp tục parse `item->rwProps` bằng cách gọi `parseItemProperties()` một lần nữa.
14. Sau đó parse socketables (nếu socketablesNumber > 0) bằng gọi đệ quy `parseItem()` cho từng socketable; các socketable có thể góp vào `rwKey` dùng để lookup runeword.

## Bit-level: parseItemProperties() (chi tiết quan trọng)
- `parseItemProperties()` lặp đọc property entries theo thứ tự sau:
  1. Đọc property id bằng `bitReader.readNumber(CharacterStats::StatCodeLength)` — `CharacterStats::StatCodeLength == 9` (9 bits).
  2. Nếu id == `ItemProperties::End` (511 / 9 bits all ones) → dừng.
  3. Lưu offset bit: `propToAdd->bitStringOffset = bitReader.pos() + 16` (cộng 16 bit vì JM header không có trong bitString lưu trong ItemInfo).
  4. Tìm metadata: `ItemPropertyTxt *txtProperty = ItemDataBase::Properties()->value(id)` — metadata chứa `bits`, `paramBits`, `add` (offset to subtract), và desc.
  5. Nếu `txtProperty->paramBits > 0` thì `prop->param = bitReader.readNumber(txtProperty->paramBits)`.
  6. Đọc `rawValue = bitReader.readNumber(txtProperty->bits)`; sau đó `prop->value = rawValue - txtProperty->add`.
  7. Với một số id đặc biệt, parser đọc thêm bit(s):
     - EnhancedDamage: đọc thêm một giá trị nữa (min) và chọn giá trị nhỏ hơn giữa hai giá trị.
     - Elemental minimum damage (fire/lightning/magic/cold/poison): parser sẽ chèn cả max (id+1) và, nếu cold/poison, còn đọc thêm length (id+2) và có logic scale (poison uses length to scale values).
  8. Ghi property vào `PropertiesMultiMap` theo id (một id có thể có nhiều instances → MultiMap).

Ghi chú: `parseItemProperties()` trả về status `ItemInfo::Ok` khi gặp End; nếu có lỗi (ví dụ txtProperty null hoặc đọc vượt quá giới hạn), sẽ throw/catch và trả status `Corrupted` (kèm message trong props).

## Các trường hợp đặc biệt / rules
- Ear items: khác biệt hoàn toàn (được parse sớm và trả về ngay) — `itemType = "ear"`.
- isExtended flag: nếu false → item quality = Normal và nhiều trường extended bị bỏ qua.
- Quality ảnh hưởng nhiều tới layout: Low/High/Normal/Magic/Set/Rare/Unique/Crafted/Honorific → mỗi loại có phần dữ liệu khác nhau (số bit khác nhau hoặc các block optional). Parser xử lý theo switch(item->quality).
- isRW (runeword) → RW props nằm sau main properties (parser gọi parseItemProperties() thêm 1 lần) và socketables được đọc sau đó.
- Socketables (gems, runes, jewels): mỗi socketable là một item riêng (JM header + bytes) và được parse bằng đệ quy; socketablesNumber giới hạn số lần đệ quy (tối đa 6 trong structs.h).

## Offsets / vị trí hữu dụng (ReverseBitWriter helpers)
- `ReverseBitWriter` cung cấp các helper để tìm và sửa bit tại các vị trí biến:
  - updateItemRow/Column: dùng `replaceValueInBitString(..., Enums::ItemOffsets::Row/Column)`
  - updateIsIdentified / updateIsEthereal: dùng fixed offsets (ví dụ `identified` ở bit 4 after JM -> thay đổi bằng offset + 16)
  - updateItemQuality / updateItemLevel: quality là vị trí biến, `findQualityOffset()` giả định nó ở offset 64 (sau fixed 64-bit header) — thông tin này phản ánh layout đầu.
  - findDefenseOffset/findDurabilityOffset/findQuantityOffset/findSocketsOffset: các hàm mô phỏng parse để trả về vị trí hiện tại trong bit string, dùng `ReverseBitReader` để skip qua các trường variable để tìm nơi cần sửa.

Lưu ý: `ReverseBitWriter::byteAlignBits(QString &bitString)` giúp đảm bảo kết quả byte-aligned (quan trọng khi chuyển bit string về bytes để lưu file).

## Xử lý lỗi và recover
- `parseItem()` bọc phần đọc trong try/catch. Nếu có exception (ví dụ đọc quá giới hạn hoặc giá trị không hợp lệ), parser gán `status = Corrupted` và cố gắng set stream position tới `itemStartOffset - 2` (vị trí JM) và tăng `searchEndOffset` để tìm vị trí item tiếp theo.
- `parseItemProperties()` khi gặp lỗi sẽ insert một property cảnh báo tại `ItemProperties::Requirements` (để hiển thị thông báo) và trả status `Corrupted`.

## Checklist khi bạn muốn sửa/ghép/tao item an toàn
1. Parse item gốc để có `ItemInfo` và `item->bitString` (chuỗi bit như parser dùng).
2. Xác định vị trí properties: dùng `ReverseBitReader` để simulate parse tới chỗ parser gọi `parseItemProperties()` (hoặc sử dụng helper hiện có trong PropertyModificationEngine).
3. Xây `propertiesBits` bằng cách chèn từng property theo format: ID (9 bits) + param (txtProperty->paramBits) + value (txtProperty->bits with add applied) — và xử lý các special-case (EnhancedDamage, elemental damages, poison length).
4. Thêm end marker (ID 511, 9 bits of 1s).
5. Reconstruct bit string: `beforeProperties + propertiesBits + afterProperties` (afterProperties có thể bao gồm RW props, socketables encoding, v.v.).
6. Byte-align bit string (dùng `ReverseBitWriter::byteAlignBits`).
7. Convert bit string về bytes theo cùng quy tắc (8 bits -> byte, chú ý thứ tự prepend khi parser build chuỗi ban đầu) và ghi file.

## Ví dụ ngắn (conceptual)
- Property entry: [ID:9][param: paramBits][value:bits]
- End: [511 in 9 bits -> "111111111"]

## Tham khảo code chính (vị trí trong repo)
- `src/itemparser.cpp` — `parseItem()` và `parseItemProperties()` (core parsing flow).
- `src/structs.h` — định nghĩa `ItemInfo`, `ItemProperty`, `ItemBase`, `ItemPropertyTxt`.
- `src/enums.h` — `CharacterStats::StatCodeLength` (9), `ItemProperties` enum (IDs), `ItemQuality`.
- `src/reversebitreader.*` — cách đọc bit theo reverse order (`readNumber`, `pos()`).
- `src/reversebitwriter.*` — helper để chèn/sửa bit, byte alignment, và tìm offsets biến.
- `src/itemdatabase.*` — metadata cho properties (bits, add, paramBits), items, runewords, socketables.

## Next steps (gợi ý mở rộng)
- Thêm bảng mapping chi tiết: list các trường fixed header (tên và số bit) theo thứ tự (dễ dùng để build lại offsets chính xác).
- Sinh các snippets code helper để: build property bitstring từ JSON/YAML, convert bitstring->bytes và ngược lại, test round-trip với `parseItem()` để xác thực.
- Viết test tự động: tạo item sample, thay đổi property, convert -> parser đọc lại và so sánh `ItemInfo`.

---

Tôi có thể tiếp tục và:
- (A) Tạo file `ITEM_STRUCTURE_ANALYSIS.md` (đã xong) kèm các snippet code lấy trực tiếp từ file nguồn; hoặc
- (B) Tạo ví dụ test nhỏ (script C++ hoặc Python) để build một item bitstring ví dụ và chạy parser (phức tạp hơn vì cần link dự án) — nếu bạn muốn tôi sẽ chuẩn bị script/test nhỏ.

Cho tôi biết bạn muốn tiếp theo theo hướng (A) hay (B) hoặc muốn tôi thêm bảng offsets chi tiết ngay trong tài liệu.

## 🧭 Offsets (bit-level field list)
Dưới đây là bảng mô tả tuần tự các trường (theo thứ tự parser đọc) cùng số bit mà parser đọc — bao gồm các điều kiện (chỉ xuất hiện khi isExtended, là armor, là weapon, v.v.). Đây là bản tóm tắt tổng hợp từ `parseItem()` và helper trong `ReverseBitWriter`.

- Header
  - JM header: 16 bits (2 bytes) — skipped by parser

- Fixed flags / header area (đọc bằng các readBool/skip trước itemType)
  - isQuest: 1 bit
  - unknown / skip: 3 bits
  - isIdentified: 1 bit
  - skip: 5 bits
  - isDuped: 1 bit
  - isSocketed: 1 bit
  - skip: 2 bits
  - skip (illegal equip + unk): 2 bits
  - isEar: 1 bit
  - isStarter: 1 bit
  - skip: 2 bits
  - skip: 1 bit
  - isExtended: 1 bit (note: parser uses !readBool() to set isExtended)
  - isEthereal: 1 bit
  - skip: 1 bit
  - isPersonalized: 1 bit
  - skip: 1 bit
  - isRW: 1 bit
  - skip: 5 bits
  - version: 8 bits
  - skip: 2 bits

- Position / storage
  - location: 3 bits
  - whereEquipped: 4 bits
  - column: 4 bits
  - row: 4 bits
  - storage: 3 bits

- Ear handling (if isEar)
  - classCode: 3 bits
  - level: 7 bits
  - name: up to 18 * 7 bits (stop on zero)
  - sets itemType = "ear" and parser returns early

- itemType
  - 4 bytes = 4 * 8 bits (four ASCII chars)

- Extended section (only if isExtended)
  - socketablesNumber: 3 bits
  - guid: 32 bits
  - ilvl: 7 bits
  - quality: 4 bits
  - variableGraphicIndex (optional): 1 bit flag then 3 bits if present (+1 applied)
  - autoprefix flag: 1 bit then skip 11 bits if true

- Quality-dependent fields (only if isExtended)
  - Normal: none
  - LowQuality / HighQuality: nonMagicType: 3 bits
  - Magic: skip 22 bits (prefix & suffix)
  - Set / Unique: setOrUniqueId: 15 bits
  - Rare / Crafted: skip 16 bits (first & second names) and up to 6 prefix/suffix slots — for each slot: 1 bit + if set skip 11 bits
  - Honorific: skip 16 bits

- RW and personalization (isExtended)
  - If isRW: RW code: skip 16 bits
  - inscribedNameOffset: stored at current bitReader.pos() (used later)
  - If isPersonalized: up to 16 chars * `kInscribedNameCharacterLength` bits (7 bits each) — stop on 0
  - Tome of ID flag: 1 bit; if book then skip 5 bits

- Armor / Weapon basic fields (if extended)
  - If armor (item types inherit from "armo"):
      - Defense: `ItemPropertyTxt->bits` bits (read and subtract `ItemPropertyTxt->add`)
  - If armor or weapon:
      - DurabilityMax: `ItemPropertyTxt->bits` bits (read and subtract add)
      - If DurabilityMax > 0: Durability: `ItemPropertyTxt->bits` bits

- Quantity / Sockets
  - If stackable (from ItemBase.isStackable): quantity: 9 bits
  - If isSocketed: socketsNumber: 4 bits

- Set lists (if quality == Set)
  - kSetListsNumber (5) bits indicating presence of set lists
  - For each present set list: parseItemProperties(bitReader, &status) (list-level properties)

- Properties (main)
  - Repeated entries:
      - Property ID: `CharacterStats::StatCodeLength` (9 bits)
      - If ID == End (511) → stop
      - param: `ItemPropertyTxt.paramBits` (if >0)
      - value: `ItemPropertyTxt.bits`
      - Special: EnhancedDamage reads an extra value, elemental min/max read pairs (+ length for cold/poison)

- RW properties (if isRW)
  - parseItemProperties(bitReader, &status) — after main properties

- Socketables (if socketablesNumber > 0)
  - For each socketable: call parseItem() recursively (each socketable is a complete item with its own JM header/bytes)

### Ghi chú chi tiết
- `propToAdd->bitStringOffset = bitReader.pos() + 16` vì `ItemInfo::bitString` không bao gồm 16-bit JM header — khi cần update trực tiếp bit offsets phải cộng 16 để có offset tương ứng với file-level bits.
- Nhiều vị trí (quality, defense, durability, quantity, sockets) là variable — `ReverseBitWriter` dùng parsing-simulation (ReverseBitReader skip/read) để tìm chính xác offset trong chuỗi bit trước khi sửa.
- Byte-alignment: sau khi chèn/sửa properties, luôn gọi `ReverseBitWriter::byteAlignBits()` trước khi chuyển chuỗi bit về bytes.

## 📌 Code snippets (trích dẫn từ source)
Dưới đây là các đoạn code quan trọng (nguyên văn, đã rút gọn) để tham khảo nhanh:

1) parseItem (khởi đầu & flags + gọi parseItemProperties)

```cpp
// from src/itemparser.cpp
inputDataStream.skipRawData(2); // skip 'JM'
// build bitstring from bytes (note: prepend order)
for (int i = 0; i < itemSize; ++i) { quint8 aByte; inputDataStream >> aByte; itemBitData.prepend(binaryStringFromNumber(aByte)); }
ReverseBitReader bitReader(itemBitData);
item = new ItemInfo(bitReader.notReadBits());
item->isQuest = bitReader.readBool();
bitReader.skip(3);
item->isIdentified = bitReader.readBool();
... // many flags & reads
for (int i = 0; i < 4; ++i) item->itemType += static_cast<quint8>(bitReader.readNumber(8));
item->itemType = item->itemType.trimmed();
if (item->isExtended) {
  item->socketablesNumber = bitReader.readNumber(3);
  item->guid = bitReader.readNumber(32);
  item->ilvl = bitReader.readNumber(7);
  item->quality = bitReader.readNumber(4);
  ...
  item->props = parseItemProperties(bitReader, &status);
}
```

2) parseItemProperties (core loop)

```cpp
// from src/itemparser.cpp
PropertiesMultiMap ItemParser::parseItemProperties(ReverseBitReader &bitReader, ItemInfo::ParsingStatus *status)
{
  using namespace Enums;
  PropertiesMultiMap props;
  while (bitReader.pos() != -1)
  {
    ItemProperty *propToAdd = 0;
    try {
      int id = bitReader.readNumber(CharacterStats::StatCodeLength); // 9 bits
      if (id == ItemProperties::End) { *status = ItemInfo::Ok; return props; }
      propToAdd = new ItemProperty;
      propToAdd->bitStringOffset = bitReader.pos() + 16;
      ItemPropertyTxt *txtProperty = ItemDataBase::Properties()->value(id);
      if (!txtProperty) throw 6;
      propToAdd->param = txtProperty->paramBits ? bitReader.readNumber(txtProperty->paramBits) : 0;
      int rawValue = bitReader.readNumber(txtProperty->bits);
      propToAdd->value = rawValue - txtProperty->add;
      // handle special cases like EnhancedDamage and elemental damage
      props.insert(id, propToAdd);
    } catch (int exceptionCode) {
      delete propToAdd; *status = ItemInfo::Corrupted; props.insert(ItemProperties::Requirements, new ItemProperty(tr("Error parsing item properties..."))); return props;
    }
  }
  *status = ItemInfo::Failed;
  return PropertiesMultiMap();
}
```

3) ItemPropertyTxt struct (metadata for properties)

```cpp
// from src/structs.h
struct ItemPropertyTxt {
  quint16 add;        // value to subtract
  quint8 bits, paramBits; // number of bits for value and param
  quint8 bitsSave, paramBitsSave; // special save bits
  QList<quint16> groupIDs;
  QString descGroupNegative, descGroupPositive, descGroupStringAdd;
  QString descNegative, descPositive, descStringAdd;
  quint8 descFunc, descPriority, descVal;
  quint8 descGroupFunc, descGroupPriority, descGroupVal;
  QByteArray stat;
};
```

4) ReverseBitReader::readNumber (how bits are read)

```cpp
// from src/reversebitreader.cpp
qint64 ReverseBitReader::readNumber(int length, bool *ok /*= 0*/)
{
  if (_pos - length >= 0) {
    if (ok) *ok = true;
    _pos -= length;
    return _bitString.mid(_pos, length).toLongLong(ok, 2);
  } else {
    if (ok) *ok = false;
    qWarning("attempt to read past bitstring length");
    _pos = _bitString.length() + 1; throw 1; return 0;
  }
}
```

---

Những snippet này giúp hiểu trực tiếp cách parser đọc dữ liệu và metadata nào cần dùng khi viết lại properties.

````
