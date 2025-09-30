# Diablo 2 Item Extraction Guide

## Tổng quan

Dựa vào script `extract_gems.pl` gốc, có thể extract các item từ save files của Diablo 2 / MedianXL thành file .d2i để sử dụng trong MedianXL Offline Tools.

## Structure của D2 Save Files

### 1. Character Files (.d2s, .d2x)
- Chứa thông tin character và inventory items
- Items thường bắt đầu từ offset cụ thể (ví dụ 3398 trong script gốc)
- Có page headers cần skip (7 bytes theo script gốc)

### 2. Stash Files (.stash, .shared)
- Chứa items trong stash
- Structure khác với character files
- Items có thể bắt đầu từ offset khác nhau

### 3. Item Format
Mỗi item có structure:
```
JM (2 bytes) - Item signature
[4 bytes] - Item type code (ví dụ: "gcr\0" cho gem)
[variable] - Item data (properties, affixes, etc.)
```

## Scripts đã tạo

### 1. `extract_items.pl` - Perl script mở rộng
- Tự động detect nhiều loại items
- Hỗ trợ nhiều categories (gems, runes, jewels, etc.)
- Command line options

### 2. `analyze_save.pl` - Phân tích structure
- Scan file tìm JM signatures
- Hiển thị hex dump để debug
- Phân tích khoảng cách giữa items

### 3. `d2_item_extractor.py` - Python extractor
- Approach khác nhau cho character vs stash files
- Tự động categorize items
- Dễ customize

### 4. `simple_item_extractor.py` - Đơn giản nhất
- Dựa trên logic của script gốc
- Auto-detect start offset
- Analysis mode để hiểu structure

## Cách sử dụng

### Phân tích file structure trước:
```bash
# Phân tích với Perl
perl scripts/analyze_save.pl -f save/character.d2s --verbose

# Phân tích với Python
python3 scripts/simple_item_extractor.py --analyze save/character.d2s
```

### Extract items:
```bash
# Với Python (tự động)
python3 scripts/d2_item_extractor.py save/character.d2s -o extracted_items

# Với script đơn giản (manual offset)
python3 scripts/simple_item_extractor.py save/character.d2s extracted_items 14 3398

# Với Perl (advanced)
perl scripts/extract_items.pl -f save/character.d2s -o extracted_items --verbose
```

## Customization

Để adapt cho save files cụ thể:

### 1. Tìm đúng offset
- Dùng analyze scripts để tìm vị trí items bắt đầu
- Look for consecutive JM signatures
- Note khoảng cách giữa items

### 2. Adjust item size
- Gems/Runes: thường 14 bytes
- Jewels: ~50 bytes  
- Armor/Weapons: có thể 100+ bytes

### 3. Modify item detection
- Update pattern matching trong scripts
- Add new item categories
- Adjust filename generation

## Ví dụ với save files hiện có

```bash
# Character file
python3 scripts/simple_item_extractor.py save/Pokemancer.d2s

# Shared stash
python3 scripts/d2_item_extractor.py save/_sharedstash.shared

# Analyze trước khi extract
python3 scripts/simple_item_extractor.py --analyze save/asdf.d2s
```

## Lưu ý

1. **Backup files**: Luôn backup save files trước khi thử nghiệm
2. **File formats**: Khác nhau giữa D2 classic, LOD, và MedianXL
3. **Item structure**: Complex items (rares, uniques) có structure khác gems/runes
4. **Validation**: Check extracted .d2i files bằng cách load vào game

## Troubleshooting

### Items không được detect:
- Check offset starting point
- Verify file format (character vs stash)
- Look for page headers/padding

### Item types sai:
- ASCII decode issues với special characters
- Null bytes trong item type
- Wrong offset calculation

### File sizes sai:
- Variable item sizes
- Overlapping reads
- Missing item boundaries

Với approach này, bạn có thể extract hầu hết các loại items từ save files và convert thành .d2i để dùng trong MedianXL Offline Tools.