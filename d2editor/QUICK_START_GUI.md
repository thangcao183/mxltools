# Quick Start Guide - GUI Property Adder

## 🚀 Khởi động GUI

```bash
cd /home/wolf/CODE/C/mxltools/d2editor
python3 gui_property_adder.py
```

## 📖 Hướng dẫn sử dụng

### 1. Chọn file D2I
- Click nút **"Browse"** ở phần "File Selection"
- Chọn file `.d2i` muốn thêm properties (ví dụ: `d2i/rich_clean_magic.d2i` hoặc `d2i/rich_clean_unique.d2i`)

### 2. Thêm Properties (2 cách)

#### Cách 1: Add Selected as Column (Recommended)
1. Tìm property trong danh sách bên trái (có search box)
2. Click chọn property
3. Click **"Add Selected as Column"**
4. Nhập value cho property trong column
5. Lặp lại để thêm nhiều properties
6. Click **"Apply All Columns"** để apply tất cả cùng lúc

**Ưu điểm**: Có thể sắp xếp thứ tự properties, preview trước khi apply

#### Cách 2: Add trực tiếp
1. Nhập value trong ô "Value" ở trên
2. Chọn một hoặc nhiều properties trong danh sách
3. Click **"Add"**

**Ưu điểm**: Nhanh hơn cho single property

### 3. Các nút chức năng

| Nút | Chức năng |
|-----|-----------|
| **Browse** | Chọn file .d2i |
| **Add** | Thêm property đã chọn với value đã nhập |
| **Edit Values...** | Mở dialog để nhập riêng value cho từng property |
| **Add Selected as Column** | Thêm property vào columns area |
| **Info** | Xem thông tin file (bits, bytes, end marker position) |
| **Preview** | Xem trước kích thước file sau khi thêm properties |
| **Apply All Columns** | Apply tất cả properties trong columns |
| **Initialize property area** | (Không hoạt động - dùng create_clean_extended thay thế) |

### 4. Quản lý Columns
- **Remove**: Xóa column
- **< / >**: Di chuyển column sang trái/phải (thay đổi thứ tự)
- **Value entry**: Nhập value riêng cho từng column

### 5. Log Area
- Hiển thị chi tiết quá trình xử lý
- Các thông báo lỗi/thành công
- Output từ property_adder.py

## 💡 Tips

### Properties phổ biến

**Tổng cộng có 387 properties có thể sử dụng!**

Một số properties hay dùng:
```
ID   7 - maxhp                     # Mạng
ID  79 - item_goldbonus            # Tìm vàng %
ID  80 - item_magicbonus           # Tìm đồ xanh %
ID  93 - item_fasterattackrate     # Tốc độ đánh %
ID 127 - item_allskills            # +All Skills
ID   0 - strength                  # Sức mạnh
ID   1 - energy                    # Năng lượng
ID   2 - dexterity                 # Nhanh nhẹn
ID   3 - vitality                  # Sinh lực
ID  39 - fireresist                # Fire Resist %
ID  41 - lightresist               # Lightning Resist %
ID  43 - coldresist                # Cold Resist %
ID  45 - poisonresist              # Poison Resist %
```

**Xem tất cả properties:**
```bash
python3 property_adder.py --list-all
python3 property_adder.py --list-all | grep -i "attack"  # Tìm theo tên
```

### Ví dụ thực tế

#### Tạo charm Gold Find + Magic Find
1. Browse → chọn `d2i/rich_clean_magic.d2i`
2. Search "Gold" → chọn "79 - Gold Find" → Add as Column → nhập value `50`
3. Search "Magic" → chọn "80 - Magic Find" → Add as Column → nhập value `30`
4. Click **"Apply All Columns"**
5. File output: `d2i/rich_clean_magic.d2i.added`

#### Tạo charm +Life +All Skills
1. Browse → chọn `d2i/rich_clean_unique.d2i`
2. Tìm property "7 - Life" → Add as Column → value `20`
3. Tìm property "127 - All Skills" → Add as Column → value `1`
4. Preview để kiểm tra
5. Apply All Columns
6. File output: `d2i/rich_clean_unique.d2i.added`

## 🔧 Command Line Alternative

Nếu không muốn dùng GUI, có thể dùng command line:

```bash
# Add single property
python3 property_adder.py <file.d2i> <prop_id> <value>

# Add multiple properties
python3 property_adder.py <file.d2i> <prop_id1> <value1> <prop_id2> <value2> ...

# Examples:
python3 property_adder.py d2i/rich_clean_magic.d2i 7 10 79 50
python3 property_adder.py d2i/rich_clean_unique.d2i 127 1 7 20
```

## 📁 File Output

- File gốc: `filename.d2i`
- File output: `filename.d2i.added`
- File gốc **không bị thay đổi**

## ⚠️ Lưu ý quan trọng

1. **Chỉ dùng với extended items**
   - File phải được tạo bằng `create_clean_extended` hoặc đã có extended fields
   - Non-extended items sẽ không hoạt động

2. **Value encoding**
   - Value nhập vào sẽ được cộng thêm với `add` value từ props.tsv
   - Ví dụ: Life có add=500, nhập 10 → raw value = 510

3. **Bit encoding**
   - Sử dụng LSB-first encoding
   - Properties được insert trước end marker (511)
   - Tự động padding đến byte boundary

4. **Testing**
   - Luôn test file `.added` trong MedianXLOfflineTools trước
   - Kiểm tra properties hiển thị đúng
   - Kiểm tra item name không thay đổi (nếu dùng clean templates)

## 🐛 Troubleshooting

### GUI không mở
```bash
# Install tkinter
sudo apt-get install python3-tk
```

### "Cannot find end marker"
→ File không phải extended item hoặc bị corrupt
→ Dùng `create_clean_extended` để tạo file mới

### Property hiển thị sai ID
→ Có thể thiếu quality-specific fields
→ Kiểm tra file structure với Info button

### Item name bị đổi
→ File có guid hoặc uniqueId khác 0
→ Dùng clean templates (rich_clean_magic.d2i, rich_clean_unique.d2i)

## 📚 Related Files

- `property_adder.py` - Backend logic (Python version of C++)
- `gui_property_adder.py` - GUI application
- `data/props.db` - Property database (SQLite)
- `D2I_FORMAT_COMPLETE_GUIDE.md` - Technical documentation

## 🎯 Workflow hoàn chỉnh

```
1. create_clean_extended
   ↓
   Tạo rich_clean_magic.d2i / rich_clean_unique.d2i
   
2. gui_property_adder.py
   ↓
   Thêm properties → file.d2i.added
   
3. MedianXLOfflineTools
   ↓
   Import và test trong game
   
4. (Nếu muốn thêm properties nữa)
   ↓
   Dùng file.d2i.added làm input → tạo file.d2i.added.added
```

## ✅ Checklist trước khi import vào game

- [ ] File được tạo từ clean template hoặc có extended fields
- [ ] Đã test trong MedianXLOfflineTools (mở file thành công)
- [ ] Properties hiển thị đúng ID và value
- [ ] Item name không bị đổi (nếu dùng clean template)
- [ ] File size hợp lý (không quá lớn bất thường)
- [ ] Log không có error messages

---

**Version**: 1.0  
**Last Updated**: October 22, 2025  
**Author**: Property Adder Team
