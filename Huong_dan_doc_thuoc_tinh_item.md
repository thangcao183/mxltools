# Hướng dẫn đọc thuộc tính Item trong MedianXL Offline Tools

## Tổng quan

MedianXL Offline Tools là một ứng dụng để xem và chỉnh sửa các file save của Diablo 2 MedianXL. Tài liệu này mô tả cách thức ứng dụng đọc và hiển thị các thuộc tính (properties) của item.

## 1. Cấu trúc cơ bản của thuộc tính item

### Thành phần chính của một thuộc tính:
- **ID thuộc tính**: Mã định danh duy nhất (9 bit)
- **Giá trị (Value)**: Giá trị số của thuộc tính
- **Tham số (Param)**: Tham số bổ sung cho thuộc tính đặc biệt
- **Chuỗi hiển thị**: Cách format thuộc tính để hiển thị cho người dùng

### Dữ liệu được lưu trữ:
- Format binary trong file .d2s
- Được parse bằng `ReverseBitReader` 
- Thuộc tính kết thúc bằng ID = 511 (End marker)

## 2. Quá trình Parse thuộc tính

### File source chính: `src/itemparser.cpp`

```cpp
PropertiesMultiMap ItemParser::parseItemProperties(ReverseBitReader &bitReader, ItemInfo::ParsingStatus *status)
{
    while (bitReader.pos() != -1) {
        // Đọc ID thuộc tính (9 bit)
        int id = bitReader.readNumber(CharacterStats::StatCodeLength);
        
        if (id == ItemProperties::End) {
            // Kết thúc đọc thuộc tính
            return props;
        }
        
        // Đọc tham số (nếu có)
        propToAdd->param = txtProperty->paramBits ? 
            bitReader.readNumber(txtProperty->paramBits) : 0;
            
        // Đọc giá trị thuộc tính
        propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
        
        // Xử lý đặc biệt cho từng loại thuộc tính
        createDisplayStringForPropertyWithId(id, propToAdd);
        
        props.insert(id, propToAdd);
    }
}
```

### Các bước chính:
1. **Đọc ID thuộc tính** (9 bit)
2. **Kiểm tra End marker** (ID = 511)
3. **Đọc param** (số bit tùy thuộc vào thuộc tính)
4. **Đọc value** (số bit + offset theo định nghĩa)
5. **Tạo display string** cho thuộc tính
6. **Lưu vào PropertiesMultiMap**

## 3. File cấu hình Properties.txt

### Cấu trúc file (từ Properties.txt.html):

| Trường | Mô tả |
|--------|--------|
| **code** | Mã định danh thuộc tính để dùng trong các file txt khác |
| **done** | 1 nếu thuộc tính đang active (boolean) |
| **stat1-stat7** | ID của stat trong ItemStatCost.txt |
| **func1-func7** | Hàm xử lý giá trị thuộc tính |
| **set1-set7** | Tham số cho hàm property |
| **val1-val7** | Tham số cho hàm property |

### Phụ thuộc:
- File này phụ thuộc vào **ItemStatCost.txt**
- Stat phải được định nghĩa trong ItemStatCost.txt mới sử dụng được

## 4. Các hàm xử lý thuộc tính (Property Functions)

| Func ID | Mô tả |
|---------|--------|
| 1 | Áp dụng giá trị trực tiếp cho stat, có thể dùng SetX param |
| 2 | Chức năng defensive, tương tự func 1 |
| 3 | Áp dụng cùng min-max range như function block trước |
| 5-6 | Liên quan đến Damage min/max |
| 7 | Liên quan đến Damage % |
| 8 | Tốc độ (IAS, FCR, FHR, etc.) |
| 9 | Áp dụng cùng param và value trong min-max range như block trước |
| 10 | Skill tab/skill group |
| 11 | Event-based skills |
| 12 | Random selection của parameters cho parameter-based stat |
| 15 | Chỉ sử dụng min field |
| 16 | Chỉ sử dụng max field |
| 17 | Chỉ sử dụng param field |
| 18 | Liên quan đến /time properties |
| 19 | Liên quan đến charged item |
| 20 | Boolean đơn giản (indestructible) |
| 21 | Thêm vào nhóm skills, nhóm được xác định bởi stat ID |
| 22 | Skill cá nhân, dùng param cho skill ID, random giữa min-max |
| 23 | Ethereal |
| 24 | Property áp dụng cho character hoặc target monster |

## 5. Các loại thuộc tính phổ biến

### A. Damage thuộc tính:
```cpp
// Enhanced Damage
if (id == ItemProperties::EnhancedDamage) {
    // Format: "X% Enhanced Damage"
    propToAdd->displayString = kEnhancedDamageFormat().arg(propToAdd->value);
}

// Elemental Damage (Fire, Lightning, Cold, Poison, Magic)
if (id == ItemProperties::MinimumDamageFire) {
    // Đọc min damage
    // Đọc max damage tiếp theo
    // Format: "X-Y Fire Damage"
    // Cold và Poison có thêm length (duration)
}
```

### B. Defense thuộc tính:
```cpp
// Armor Defense
if (isArmor) {
    ItemPropertyTxt *defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
    item->defense = bitReader.readNumber(defenceProp->bits) - defenceProp->add;
}
```

### C. Resistances:
- Fire Resistance
- Cold Resistance  
- Lightning Resistance
- Poison Resistance
- All Resistances (sử dụng func 3)

### D. Skills:
- Individual Skills (func 22)
- Skill Tabs (func 10)
- Class Skills (func 21)

### E. Stats cơ bản:
- Strength
- Dexterity  
- Vitality
- Energy

## 6. Xử lý thuộc tính đặc biệt

### Poison Damage:
```cpp
if (id == ItemProperties::DurationPoison) {
    // Tính toán lại min/max poison damage với duration
    props.replace(id - 1, new ItemProperty(
        qRound(maxElementalDamageProp->value * length / 256.0), length));
    props.replace(id - 2, new ItemProperty(
        qRound(propToAdd->value * length / 256.0), length));
}
```

### Set Item Properties:
- Set items có thêm các thuộc tính khi đủ bộ
- Được parse riêng biệt sau thuộc tính cơ bản

### Runeword Properties:
- Item có runeword có thêm thuộc tính từ runeword
- Lưu trong `item->rwProps`

## 7. Hiển thị thuộc tính

### File source: `src/propertiesviewerwidget.cpp`

```cpp
void PropertiesViewerWidget::showItem(ItemInfo *item) {
    // Thu thập tất cả properties
    PropertiesMultiMap allProps;
    
    // Thêm properties từ item
    PropertiesMultiMap::const_iterator constIter = item->props.constBegin();
    while (constIter != item->props.constEnd()) {
        allProps.insert(constIter.key(), new ItemProperty(*constIter.value()));
        ++constIter;
    }
    
    // Thêm runeword properties nếu có
    if (item->isRW) {
        PropertiesDisplayManager::addProperties(&allProps, item->rwProps);
    }
    
    // Hiển thị HTML
    renderHtml(ui->allTextEdit, propertiesToHtml(allProps));
}
```

### Đặc điểm hiển thị:
- **Màu sắc**: Khác nhau cho từng loại thuộc tính
- **Thứ tự**: Theo priority được định nghĩa
- **Format**: Đặc biệt cho từng loại (%, flat value, ranges)
- **Nhóm**: Properties được nhóm theo loại

## 8. Debugging và Error Handling

### Trạng thái parse:
```cpp
enum ParsingStatus {
    Ok,           // Parse thành công
    Failed,       // Parse thất bại
    Corrupted     // Dữ liệu bị hỏng
};
```

### Error messages:
- "Error parsing item properties (status == failed)"
- "Error parsing RW properties" 
- "Error parsing item properties (exception == X)"

## 9. Cách sử dụng thực tế

### Bước 1: Mở file save
- File .d2s (character save)
- File .d2x (PlugY stash)

### Bước 2: Parse items
- `ItemParser::parseItem()` cho từng item
- Đọc từ inventory, stash, equipment

### Bước 3: Xem properties  
- Click vào item trong danh sách
- Properties hiển thị trong `PropertiesViewerWidget`
- Thông tin chi tiết bao gồm cả hidden properties

### Ứng dụng:
- **Modding**: Hiểu cách properties hoạt động
- **Analysis**: Xem hidden properties và exact values  
- **Debugging**: Kiểm tra properties không hiển thị đúng
- **Research**: Nghiên cứu mechanics của MedianXL

## 10. Files quan trọng

### Source code:
- `src/itemparser.cpp` - Parse item và properties chính
- `src/itemparser.h` - Interface và constants
- `src/propertiesviewerwidget.cpp` - Hiển thị properties
- `src/itemdatabase.cpp` - Load dữ liệu properties từ file

### Data files:
- `resources/data/props` - Properties data (binary)
- Properties.txt - Cấu hình properties (text format)
- ItemStatCost.txt - Stat definitions

### Documentation:
- `d2 docs/txt guides/Properties.txt.html` - Hướng dẫn chi tiết
- `d2 docs/1.10/D2S File Format.html` - Format file save

## 11. Chi tiết kỹ thuật về Offset và Bit-level Parsing

### A. Cấu trúc ItemProperty:
```cpp
struct ItemProperty {
    int bitStringOffset;  // Vị trí của property trong bit string
    int value;           // Giá trị của property
    quint32 param;       // Tham số (nếu có)
    QString displayString; // Chuỗi hiển thị custom
};
```

### B. Bit String và Offset System:

#### Item Bit String:
- Mỗi item được lưu dưới dạng một chuỗi bit dài
- Header 'JM' (16 bit) không được lưu trong bitString
- Các offset tính từ cuối chuỗi bit (reverse reading)

#### Offset Calculation:
```cpp
// Formula tính offset trong ReverseBitWriter
int startOffset(const QString &bitString, int offset, int length, bool isItemHeaderSkipped = true) {
    // 16 là 'JM' offset không được lưu trong bitString
    return bitString.length() - (offset - 16 * isItemHeaderSkipped) - length;
}
```

### C. Item Offsets cố định:
```cpp
enum ItemOffsetsEnum {
    Ethereal = 0x26,        // 1 bit
    IsPersonalized = 0x28,  // 1 bit
    Location = 0x3A,        // 3 bit
    EquipIndex = 0x3D,      // 4 bit
    Column = 0x41,          // 4 bit
    Row = 0x45,             // 4 bit
    Storage = 0x49,         // 3 bit
    Type = 0x4C             // 32 bit (4 chars x 8 bit)
};
```

### D. Property Parsing Process:

#### 1. Đọc Property ID:
```cpp
int id = bitReader.readNumber(CharacterStats::StatCodeLength); // 9 bit
if (id == ItemProperties::End) {
    return props; // Kết thúc danh sách properties
}
```

#### 2. Lưu Offset:
```cpp
propToAdd = new ItemProperty;
// Lưu vị trí hiện tại + 16 bit header 'JM'
propToAdd->bitStringOffset = bitReader.pos() + 16;
```

#### 3. Đọc Parameters và Value:
```cpp
ItemPropertyTxt *txtProperty = ItemDataBase::Properties()->value(id);

// Đọc param nếu có
propToAdd->param = txtProperty->paramBits ? 
    bitReader.readNumber(txtProperty->paramBits) : 0;

// Đọc value với offset correction
propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
```

### E. ReverseBitReader Operations:

#### Cơ chế đọc ngược:
```cpp
class ReverseBitReader {
private:
    QString _bitString;
    int _pos; // Vị trí hiện tại từ cuối chuỗi
    
public:
    qint64 readNumber(int length) {
        _pos -= length;
        return _bitString.mid(_pos, length).toLongLong(0, 2);
    }
    
    int pos() const { 
        return _bitString.length() - _pos; // Vị trí từ đầu
    }
};
```

#### Ví dụ parsing Enhanced Damage:
```cpp
if (id == ItemProperties::EnhancedDamage) {
    // Đọc value đầu tiên
    propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
    // Enhanced Damage có 2 values giống nhau
    qint16 minEnhDamage = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
    // Lấy giá trị nhỏ hơn (safety check)
    if (minEnhDamage < propToAdd->value)
        propToAdd->value = minEnhDamage;
}
```

### F. Bit Writing và Modification:

#### ReverseBitWriter cho việc chỉnh sửa:
```cpp
// Thay đổi giá trị tại offset cụ thể
QString &replaceValueInBitString(QString &bitString, int offset, int newValue, int length = -1) {
    return bitString.replace(startOffset(bitString, offset, length), 
                           length, 
                           binaryStringFromNumber(newValue, false, length));
}
```

#### Ví dụ thay đổi vị trí item:
```cpp
QString &updateItemRow(ItemInfo *item) {
    return replaceValueInBitString(item->bitString, 
                                 Enums::ItemOffsets::Row, 
                                 item->row);
}
```

### G. Property Value Lookup:

#### Tìm property theo ID và param:
```cpp
int indexOfPropertyValue(int id, const PropertiesMultiMap *props, quint32 param = 0) {
    ItemProperty *prop = getProperty(id, param, props);
    ItemPropertyTxt *property = ItemDataBase::Properties()->value(id);
    
    // Tính toán value với add offset
    qulonglong value = prop->value + property->add;
    int bits = property->bits;
    
    // Tìm trong bit string
    int idIndex = _item->bitString.indexOf(
        binaryStringFromNumber(id, false, CharacterStats::StatCodeLength));
    
    int paramIndex = idIndex - property->paramBits;
    int valueIndex = paramIndex - bits;
    
    return valueIndex;
}
```

### H. Xử lý Properties đặc biệt:

#### Elemental Damage (có min/max):
```cpp
// Min damage
props.insert(id++, propToAdd);

// Max damage  
ItemProperty *maxDamageProp = new ItemProperty;
maxDamageProp->bitStringOffset = bitReader.pos() + 16;
maxDamageProp->value = bitReader.readNumber(txtProp->bits) - txtProp->add;
props.insert(id, maxDamageProp);

// Length cho Cold/Poison
if (hasLength) {
    ItemPropertyTxt *lengthProp = ItemDataBase::Properties()->value(++id);
    qint16 length = bitReader.readNumber(lengthProp->bits) - lengthProp->add;
    
    // Tính toán damage/second cho poison
    if (id == ItemProperties::DurationPoison) {
        props.replace(id - 1, new ItemProperty(
            qRound(maxDamageProp->value * length / 256.0), length));
        props.replace(id - 2, new ItemProperty(
            qRound(propToAdd->value * length / 256.0), length));
    }
}
```

### I. Byte Alignment:

#### Đảm bảo chuỗi bit aligned theo byte:
```cpp
QString &byteAlignBits(QString &bitString) {
    const int kBitsInByte = 8;
    int extraBits = bitString.length() % kBitsInByte;
    if (extraBits) {
        int zerosBeforeFirst1 = bitString.indexOf('1');
        int zerosToAppend = kBitsInByte - extraBits;
        if (zerosBeforeFirst1 + zerosToAppend < kBitsInByte)
            bitString.prepend(QString(zerosToAppend, '0'));
        else
            bitString.remove(0, extraBits);
    }
    return bitString;
}
```

### J. Debugging Bit Operations:

#### Công cụ debug properties:
1. **bitStringOffset**: Vị trí chính xác trong bit string
2. **pos()**: Vị trí hiện tại của bit reader  
3. **notReadBits()**: Phần chưa đọc của bit string
4. **Exception handling**: Catch lỗi khi đọc qua giới hạn

#### Validation:
- Kiểm tra `_pos >= 0` khi đọc
- Throw exception nếu đọc quá giới hạn
- Status tracking: `Ok`, `Failed`, `Corrupted`

## 12. Practical Applications

### Sử dụng offset information để:
1. **Locate properties**: Tìm chính xác vị trí property trong file
2. **Modify values**: Thay đổi giá trị mà không ảnh hưởng phần khác
3. **Insert/Remove**: Thêm hoặc xóa properties
4. **Validate integrity**: Kiểm tra tính toàn vẹn của data
5. **Debug parsing**: Trace quá trình đọc properties

### Tools và utilities:
- `ReverseBitReader`: Đọc bit theo thứ tự ngược
- `ReverseBitWriter`: Ghi/sửa bit tại offset cụ thể  
- `ItemDataBase::Properties()`: Load property definitions
- `indexOfPropertyValue()`: Tìm property trong bit string

## Ghi chú

Tài liệu này được tạo dựa trên phân tích chi tiết source code của MedianXL Offline Tools và tài liệu có sẵn. Thông tin có thể thay đổi theo các phiên bản mới của ứng dụng.

Để hiểu sâu hơn về bit-level operations và offset calculations, nên đọc kết hợp với source code và test với các item thực tế trong game. Đặc biệt chú ý đến cách `ReverseBitReader` và `ReverseBitWriter` hoạt động để hiểu rõ cơ chế đọc/ghi properties.

## 13. Sự khác biệt trong cách lưu Offset theo từng loại Property

### CÂU TRẢ LỜI: **Có, mỗi thuộc tính có cách lưu offset khác nhau dựa trên:**

### A. Cấu trúc ItemPropertyTxt định nghĩa format:
```cpp
struct ItemPropertyTxt {
    quint16 add;              // Offset value để tính toán
    quint8 bits;              // Số bit cho value trên item
    quint8 paramBits;         // Số bit cho parameter
    quint8 bitsSave;          // Số bit khi lưu vào character stats  
    quint8 paramBitsSave;     // Số bit cho param khi lưu character stats
    QByteArray stat;          // Tên stat trong ItemStatCost.txt
    // ... other display fields
};
```

### B. Các loại Offset khác nhau:

#### 1. **Item Properties vs Character Stats**:
```cpp
// Item properties sử dụng bits/paramBits
propToAdd->param = txtProperty->paramBits ? 
    bitReader.readNumber(txtProperty->paramBits) : 0;
propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;

// Character stats sử dụng bitsSave/paramBitsSave
if (txtProp->paramBitsSave)
    statData << bitReader.readNumber(txtProp->paramBitsSave);
qint64 statValue = bitReader.readNumber(txtProp->bitsSave);
```

#### 2. **Enhanced Damage - Trường hợp đặc biệt**:
```cpp
if (id == ItemProperties::EnhancedDamage) {
    // Đọc 2 lần với cùng số bit
    propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
    qint16 minEnhDamage = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
    
    // Offset calculation cho Enhanced Damage khác
    qulonglong oldValue = value;
    value <<= property->bits;  // Shift left
    value += oldValue;         // Combine values
    bits *= 2;                 // Double bit length
}
```

#### 3. **Elemental Damage - Min/Max pairs**:
```cpp
if (id == MinimumDamageFire || id == MinimumDamageLightning ||
    id == MinimumDamageMagic || id == MinimumDamageCold || 
    id == MinimumDamagePoison) {
    
    // Min damage
    props.insert(id++, propToAdd);
    
    // Max damage - offset tiếp theo
    ItemProperty *maxDamageProp = new ItemProperty;
    maxDamageProp->bitStringOffset = bitReader.pos() + 16;
    props.insert(id, maxDamageProp);
    
    // Length cho Cold/Poison (thêm 1 property nữa)
    if (hasLength) {
        ItemPropertyTxt *lengthProp = ItemDataBase::Properties()->value(++id);
        qint16 length = bitReader.readNumber(lengthProp->bits) - lengthProp->add;
        
        // Poison damage tính toán đặc biệt
        if (id == ItemProperties::DurationPoison) {
            // Recalculate damage với length
            props.replace(id - 1, new ItemProperty(
                qRound(maxDamageProp->value * length / 256.0), length));
            props.replace(id - 2, new ItemProperty(
                qRound(propToAdd->value * length / 256.0), length));
        }
    }
}
```

#### 4. **Properties có Parameter**:
```cpp
// Skill properties (class skills, individual skills)
if (txtProperty->paramBits > 0) {
    propToAdd->param = bitReader.readNumber(txtProperty->paramBits);
    // Offset layout: [ID][PARAM][VALUE]
    // bitStringOffset chỉ đến VALUE, không tính PARAM
}

// Properties không có parameter  
else {
    propToAdd->param = 0;
    // Offset layout: [ID][VALUE]
}
```

### C. Offset Calculation dựa trên Property Type:

#### 1. **Fixed Item Offsets** (trong item header):
```cpp
enum ItemOffsetsEnum {
    Ethereal = 0x26,        // 1 bit - boolean
    IsPersonalized = 0x28,  // 1 bit - boolean  
    Location = 0x3A,        // 3 bit - enum
    EquipIndex = 0x3D,      // 4 bit - number
    Column = 0x41,          // 4 bit - coordinate
    Row = 0x45,             // 4 bit - coordinate
    Storage = 0x49,         // 3 bit - enum
    Type = 0x4C             // 32 bit - string (4 chars)
};

// Mỗi offset có độ dài cố định
int offsetLength(int offset) {
    switch (offset) {
    case Ethereal: case IsPersonalized: return 1;
    case Location: case Storage: return 3;  
    case EquipIndex: case Column: case Row: return 4;
    case Type: case (Type + 8): case (Type + 16): case (Type + 24): 
        return 8; // 4 chars x 8 bit each
    }
}
```

#### 2. **Variable Property Offsets** (trong property list):
```cpp
// Mỗi property có offset riêng dựa trên txtProperty definition
propToAdd->bitStringOffset = bitReader.pos() + 16; // Current position + JM header

// Layout trong bit string:
// [Property ID: 9 bit][Parameter: paramBits][Value: bits]
//                     ^-- bitStringOffset points here
```

### D. Lookup và Modification theo Property Type:

#### 1. **Tìm Property trong Bit String**:
```cpp
int indexOfPropertyValue(int id, quint32 param = 0) {
    ItemPropertyTxt *property = ItemDataBase::Properties()->value(id);
    
    // Tìm ID (9 bit)
    int idIndex = _item->bitString.indexOf(
        binaryStringFromNumber(id, false, 9), idIndex + 1);
    
    // Tính offset param và value
    int paramIndex = idIndex - property->paramBits;
    int valueIndex = paramIndex - property->bits;
    
    // Verify value và param
    qulonglong valueInString = _item->bitString.mid(valueIndex, bits).toULongLong(0, 2);
    quint32 paramInString = _item->bitString.mid(paramIndex, paramBits).toUInt(0, 2);
    
    return valueIndex;
}
```

#### 2. **Modification với Property-specific Logic**:
```cpp
// Standard property
ReverseBitWriter::replaceValueInBitString(
    item->bitString, 
    prop->bitStringOffset,     // Offset tới value
    newValue + txtProp->add,   // Add offset correction
    txtProp->bits);            // Bit length

// Enhanced Damage (double value)
ReverseBitWriter::replaceValueInBitString(
    item->bitString,
    prop->bitStringOffset,
    (newValue << txtProp->bits) + newValue,  // Combined value
    txtProp->bits * 2);                       // Double bit length
```

### E. Trường hợp đặc biệt cần lưu ý:

#### 1. **Character Stats vs Item Properties**:
- **Character stats**: Dùng `bitsSave`/`paramBitsSave`
- **Item properties**: Dùng `bits`/`paramBits`
- **Cùng stat ID** nhưng **khác bit length**!

#### 2. **Compound Properties**:
- **Elemental Damage**: 3 values (min, max, length)
- **Enhanced Damage**: 2 identical values  
- **Skill Properties**: ID + skill ID parameter
- **Mystic Orb Properties**: Có logic compression riêng

#### 3. **Alignment Requirements**:
```cpp
// Sau khi modify, cần byte alignment
ReverseBitWriter::byteAlignBits(item->bitString);

// Đảm bảo bit string chia hết cho 8
int extraBits = bitString.length() % 8;
if (extraBits) {
    // Add hoặc remove bits để align
}
```

### F. Debugging Offset Issues:

#### Common Problems:
1. **Sai bit length**: Dùng `bits` thay vì `bitsSave`
2. **Missing parameter**: Không tính `paramBits`
3. **Wrong alignment**: Không byte-align sau modify
4. **Offset calculation**: Quên +16 cho JM header

#### Validation Tools:
```cpp
// Kiểm tra offset hợp lệ
bool isValidOffset = (bitStringOffset >= 16 && 
                     bitStringOffset < item->bitString.length());

// Verify bit boundaries  
bool isAligned = (item->bitString.length() % 8 == 0);

// Check property integrity
ItemProperty *prop = item->props.value(id);
bool isValid = (prop && prop->bitStringOffset > 0);
```

**KẾT LUẬN**: Mỗi loại property có cách lưu offset riêng biệt dựa trên:
- **Bit length** (bits vs bitsSave)
- **Parameter presence** (paramBits)  
- **Special formatting** (Enhanced Damage, Elemental Damage)
- **Context** (Item vs Character Stats)
- **Alignment requirements** (byte boundaries)