# Property Editor - Tính năng sửa thuộc tính Item

## Tổng quan

Tính năng Property Editor cho phép người dùng chỉnh sửa các thuộc tính (properties) của item trong MedianXL Offline Tools một cách trực quan và an toàn.

## Cách sử dụng

### 1. Mở Property Editor
1. Chọn item trong danh sách
2. Click nút **"Edit Properties"** trong tab hiển thị properties
3. Cửa sổ Property Editor sẽ mở ra

### 2. Chỉnh sửa Properties
- **Property dropdown**: Chọn loại thuộc tính (Enhanced Damage, Defense, Resistances, etc.)
- **Value field**: Nhập giá trị thuộc tính
- **Parameter field**: Nhập tham số bổ sung (cho các thuộc tính cần, như Skill ID)
- **Remove button**: Xóa thuộc tính

### 3. Thêm/Xóa Properties
- **Add Property**: Thêm thuộc tính mới
- **Remove**: Xóa thuộc tính hiện tại
- **Show all properties**: Hiển thị cả các thuộc tính ẩn

### 4. Áp dụng thay đổi
- **Apply Changes**: Lưu thay đổi vào item (không thể hoàn tác dễ dàng)
- **Revert Changes**: Hủy bỏ tất cả thay đổi

## Tính năng chính

### Validation
- **Bit limits**: Kiểm tra giá trị trong giới hạn bit
- **Range validation**: Đảm bảo giá trị hợp lệ cho từng loại thuộc tính
- **Duplicate detection**: Phát hiện thuộc tính trùng lặp
- **Parameter validation**: Kiểm tra tham số cho thuộc tính có parameter

### Special Property Handling
- **Enhanced Damage**: Xử lý đặc biệt (lưu 2 lần)
- **Elemental Damage**: Min/Max pairs với length
- **Skill Properties**: Validation skill ID
- **Defense Properties**: Kiểm tra không âm

### Safety Features
- **Backup & Restore**: Tự động backup properties gốc
- **Confirmation dialogs**: Xác nhận trước khi áp dụng thay đổi
- **Error handling**: Xử lý lỗi và rollback khi cần
- **Real-time validation**: Hiển thị warning ngay lập tức

### Overflow Protection (Bảo vệ tràn số)
- **Safe Value Ranges**: Tự động giới hạn giá trị trong phạm vi an toàn
- **Bit Overflow Prevention**: Ngăn chặn tràn bit gây corrupt save file
- **Property-specific Limits**: Giới hạn phù hợp với từng loại thuộc tính:
  - **Defense**: 0 đến 65535 (không thể âm)
  - **Enhanced Damage**: 0 đến 65535 (% không thể âm) 
  - **Damage Properties**: 0 đến 32767 (damage không thể âm)
  - **Life/Mana/Stamina**: -32767 đến 32767 (có thể âm cho penalty)
  - **Attributes (Str/Dex/Vit/Energy)**: -1000 đến 10000 (giới hạn hợp lý)
- **Pre-validation**: Kiểm tra tất cả giá trị trước khi áp dụng
- **Dynamic Tooltips**: Hiển thị range cụ thể cho từng property
- **Warning System**: Cảnh báo overflow risk và extreme values

## Cấu trúc code

### PropertyEditor (propertyeditor.h/.cpp)
- **Main UI class**: Giao diện chính cho editing
- **Validation logic**: Kiểm tra tính hợp lệ
- **UI management**: Quản lý rows, buttons, combos

### PropertyModificationEngine (propertymodificationengine.h/.cpp)
- **Bit manipulation**: Xử lý bit-level operations
- **Property reconstruction**: Tái tạo bit string
- **Special property handlers**: Xử lý thuộc tính đặc biệt
- **Validation engine**: Engine validation mạnh mẽ

### Integration với PropertiesViewerWidget
- **Seamless integration**: Tích hợp mượt mà
- **Signal connections**: Kết nối events
- **Auto refresh**: Tự động refresh hiển thị

## Advanced Features

### Bit-level Operations
```cpp
// Ví dụ: Enhanced Damage có cấu trúc đặc biệt
if (propertyId == ItemProperties::EnhancedDamage) {
    // Lưu 2 lần giá trị giống nhau
    appendBits(bitString, adjustedValue, propTxt->bits);
    appendBits(bitString, adjustedValue, propTxt->bits);
}
```

### Property Dependencies
- **Elemental Damage**: Min damage kéo theo Max damage
- **Cold/Poison**: Có thêm length/duration
- **Set Properties**: Xử lý thuộc tính set items

### Error Recovery
- **Exception handling**: Catch mọi lỗi bit manipulation
- **Rollback mechanism**: Tự động khôi phục khi lỗi
- **User feedback**: Thông báo lỗi rõ ràng

## Validation Rules

### Bit Limits
```cpp
// Mỗi property có giới hạn bit khác nhau
int maxValue = (1 << propTxt->bits) - 1 - propTxt->add;
int minValue = -propTxt->add;
```

### Special Cases
- **Enhanced Damage**: 0-32767
- **Defense**: >= 0
- **Skill Level**: 0-99
- **Skill ID**: Valid skill trong database

### Parameter Validation
```cpp
// Parameter có giới hạn riêng
if (propTxt->paramBits > 0) {
    quint32 maxParam = (1 << propTxt->paramBits) - 1;
}
```

## Troubleshooting

### Common Issues
1. **"Property not found"**: Property ID không hợp lệ
2. **"Value out of range"**: Giá trị vượt quá giới hạn bit
3. **"Duplicate property"**: Thuộc tính bị trùng lặp
4. **"Bit manipulation error"**: Lỗi khi tái tạo bit string

### Solutions
1. Kiểm tra property ID trong ItemStatCost.txt
2. Xem documentation về giới hạn của từng property
3. Xóa property trùng lặp
4. Backup và restore item

## Safety Recommendations

### Before Using
1. **Backup save file**: Sao lưu file save trước khi sửa
2. **Test với item không quan trọng**: Test trên item ít giá trị trước
3. **Hiểu về properties**: Đọc hiểu về thuộc tính muốn sửa

### During Use
1. **Validate values**: Kiểm tra giá trị hợp lệ
2. **Check warnings**: Để ý các warning màu đỏ
3. **Start small**: Bắt đầu với thay đổi nhỏ

### After Use
1. **Test in game**: Kiểm tra item trong game
2. **Verify properties**: Đảm bảo thuộc tính hiển thị đúng
3. **Keep backup**: Giữ backup cho đến khi chắc chắn

## Technical Notes

### Property Storage Format
- **Header**: 'JM' (16 bit) + item data
- **Properties**: [ID:9bit][Param:variable][Value:variable]
- **End marker**: ID = 511

### Bit String Operations
- **Reverse bit reading**: LSB first
- **Byte alignment**: Căn chỉnh theo byte sau modify
- **Offset calculation**: Tính từ cuối string về đầu

### Memory Management
- **Proper cleanup**: Auto delete objects
- **Exception safety**: RAII patterns
- **No memory leaks**: Careful pointer management

## Future Enhancements

### Possible Improvements
1. **Undo/Redo**: Multi-level undo system
2. **Property templates**: Save/load property combinations
3. **Batch editing**: Edit multiple items at once
4. **Advanced filters**: Filter properties by type
5. **Import/Export**: Import properties from text file

### Integration Ideas
1. **Character editor**: Edit character stats
2. **Item generator**: Generate items with specific properties
3. **Property calculator**: Calculate optimal property values
4. **Database tools**: Manage property definitions

Tính năng Property Editor cung cấp một cách mạnh mẽ và an toàn để chỉnh sửa thuộc tính item, với validation đầy đủ và error handling để đảm bảo tính toàn vẹn của dữ liệu.