# Game Data Extraction Tool

Python script để extract và decompress dữ liệu game từ các file `.dat` (MedianXL Offline Tools).

## Tính năng

- ✅ Decompress file `.dat` (Qt qCompress format)
- ✅ Export sang CSV, TSV, hoặc SQLite
- ✅ Xử lý từng file hoặc toàn bộ thư mục
- ✅ Verify CRC checksums
- ✅ Hỗ trợ Unicode

## Cài đặt

Chỉ cần Python 3.6+ (không cần thư viện bên ngoài):

```bash
chmod +x utils/extract_game_data.py
```

## Sử dụng

### 1. Extract một file sang TSV
```bash
python3 utils/extract_game_data.py resources/data/en/items.dat -f tsv
```

### 2. Extract một file sang CSV với output directory tùy chỉnh
```bash
python3 utils/extract_game_data.py resources/data/en/props.dat -f csv -o output/
```

### 3. Extract toàn bộ thư mục sang SQLite
```bash
python3 utils/extract_game_data.py resources/data/en/ -f sqlite -o extracted_data/
```

### 4. Extract tất cả data files
```bash
python3 utils/extract_game_data.py resources/data/ -f tsv -o all_data/
```

## Options

```
usage: extract_game_data.py [-h] [-f {csv,tsv,sqlite}] [-o OUTPUT] input

positional arguments:
  input                 Input .dat file hoặc directory

options:
  -f, --format          Output format: csv, tsv, sqlite (default: tsv)
  -o, --output          Output directory (default: cùng thư mục input)
```

## Các file data được hỗ trợ

### Resources/data/en/ (Localized)
- ✅ `items.dat` - Thông tin base items (2440 items)
- ✅ `props.dat` - Định nghĩa properties (401 properties)
- ✅ `uniques.dat` - Unique items (1823 items)
- ✅ `setitems.dat` - Set items (199 items)
- ✅ `rw.dat` - Runewords (229 runewords)
- ✅ `socketables.dat` - Gems, Runes (136 items)
- ✅ `skills.dat` - Skills (2969 skills)
- ✅ `monsters.dat` - Monster names (3740 monsters)
- ✅ `mercs.dat` - Mercenary names (153 mercs)
- ✅ `LowQualityItems.dat` - Low quality prefixes (11 entries)
- ⚠️ `string.dat` - String table (format khác, cần xử lý riêng)
- ⚠️ `patchstring.dat` - Patch strings (format khác)
- ⚠️ `expansionstring.dat` - Expansion strings (format khác)

### Resources/data/ (Non-localized)
- ✅ `itemtypes.dat` - Item type definitions (313 types)
- ✅ `basestats.dat` - Base character stats
- ✅ `sets.dat` - Set definitions
- ✅ `props.dat` - Additional properties

## Format File

### Input: .dat files
```
[2 bytes] - Compressed CRC (little-endian uint16)
[2 bytes] - Original CRC (little-endian uint16)
[4 bytes] - Original size (big-endian uint32, Qt qCompress header)
[n bytes] - zlib compressed data
```

### Output: TSV/CSV
```
Tab-separated hoặc comma-separated values
Mỗi dòng = 1 record
Dòng đầu = header (nếu có)
```

### Output: SQLite
```
Table name = tên file (vd: items, props)
Columns = col0, col1, col2, ... (generic names)
All columns = TEXT type
```

## Ví dụ Output

### items.dat → items.tsv
```
#code   name                    width   height  gentype ...
01!     Athulua's Hand (1)      2       2       0       ...
03!     Athulua's Hand (2)      2       2       0       ...
```

### props.dat → SQLite
```sql
SELECT * FROM props LIMIT 3;
-- Results: property definitions với các columns
-- col0=#id, col1=add, col2=bits, col3=paramBitsSave, ...
```

## Lưu ý

- **CRC Mismatch Warnings**: Một số file có CRC không khớp nhưng vẫn decompress được. Có thể do khác biệt trong thuật toán qChecksum hoặc file đã được modify.
- **String Tables**: Các file `string.dat`, `patchstring.dat`, `expansionstring.dat` có format đặc biệt (không phải TSV) nên cần parser riêng.
- **Unicode**: Script hỗ trợ UTF-8 encoding với error handling.

## Kết hợp với Analysis

Sau khi extract, có thể dùng các công cụ khác:

```bash
# View CSV/TSV
column -t -s $'\t' items.tsv | less

# Query SQLite
sqlite3 items.db "SELECT name, width, height FROM items WHERE gentype=1"

# Import vào pandas
python3 -c "import pandas as pd; df=pd.read_csv('items.tsv', sep='\t'); print(df.head())"
```

## Troubleshooting

**Q: Tại sao có CRC mismatch warnings?**
A: Thuật toán qChecksum của Qt có thể khác một chút so với implementation Python. File vẫn decompress đúng.

**Q: String files không extract được?**
A: Các file string.dat có format binary đặc biệt, không phải TSV. Cần parser riêng cho TBL format.

**Q: Làm sao extract tất cả files một lần?**
A: `python3 utils/extract_game_data.py resources/data/ -f tsv -o all_extracted/`
