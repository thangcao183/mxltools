# Property Loading Issues & Debug Guide

## Vấn đề

Khi edit properties, một vài properties load không đúng với thực tế, tức là giá trị hiển thị trong PropertyEditor khác với giá trị thực sự của item.

## Nguyên nhân

### 1. **Property Value Encoding**
Trong Diablo 2 save format, properties được lưu trữ theo công thức:
```
bitStringValue = actualValue + propTxt->add
```

Khi đọc từ bitString:
```cpp
propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
```

### 2. **Multiple Representations**
- **Stored Value**: `property->value` (đã trừ `add`)  
- **BitString Value**: `storedValue + add` (trong file save)
- **Display Value**: Giá trị hiển thị cho user (có thể khác stored value)

### 3. **Special Properties**
Một số properties có logic đặc biệt:
- **Enhanced Damage**: Được lưu 2 lần (min/max)
- **Elemental Damage**: Min/Max pairs với length
- **Defense**: Có thể có multiple sources
- **Skills**: Parameter encoding phức tạp

## Giải pháp đã implement

### 1. **Value Conversion Functions**
```cpp
// Chuyển từ stored value → display value
int getDisplayValueForProperty(int propertyId, const ItemProperty *property) const;

// Chuyển từ display value → stored value  
int getStorageValueFromDisplay(int propertyId, int displayValue) const;
```

### 2. **Improved Property Loading**
```cpp
// Thay vì load trực tiếp
addPropertyRow(it.key(), it.value()->value, it.value()->param, false);

// Bây giờ sử dụng conversion
int displayValue = getDisplayValueForProperty(it.key(), it.value());
addPropertyRow(it.key(), displayValue, it.value()->param, false);
```

### 3. **Correct Property Saving**
```cpp
// Convert display → storage trước khi lưu
int storageValue = getStorageValueFromDisplay(propertyId, displayValue);
existingProp->value = storageValue;

// Update bitString với đúng format
int bitStringValue = storageValue + propTxt->add;
ReverseBitWriter::replaceValueInBitString(...);
```

### 4. **Debug Logging**
Thêm logging để trace conversion:
```cpp
qDebug() << QString("PropertyEditor: Property %1 - stored: %2, display: %3, add: %4, bits: %5")
            .arg(propertyId).arg(property->value).arg(result).arg(propTxt->add).arg(propTxt->bits);
```

## Testing & Validation

### Để kiểm tra properties load đúng:

1. **Mở file save D2**
2. **Chọn item có properties** 
3. **Note giá trị hiển thị trong game**
4. **Mở PropertyEditor**
5. **So sánh giá trị trong editor với game**
6. **Check debug console** cho conversion logging

### Debug Console Output:
```
PropertyEditor: Property 31 - stored: 150, display: 150, add: 32, bits: 8
PropertyEditor: Property 16 - stored: 45, display: 45, add: 0, bits: 7  
```

## Properties cần chú ý đặc biệt

### Enhanced Damage (31)
- Có logic đặc biệt trong parser
- Được lưu 2 lần (min = max)
- Format: `+X% Enhanced Damage`

### Defense (16) 
- Có thể có base defense + bonus defense
- Armor items có defense riêng
- Format: `Defense: X`

### Elemental Damages (48-57)
- Min/Max pairs
- Cold/Poison có length parameter
- Format: `+X-Y Fire Damage`

### Skills (83, 107, 204)
- Parameter = skill ID hoặc class + skill
- Complex encoding cho charged skills
- Format: `+X to Skill Name`

## Kết quả mong đợi

✅ **Properties load với giá trị chính xác**
✅ **Display values match game values** 
✅ **Changes persist sau save/reload**
✅ **No corruption từ incorrect encoding**
✅ **Debug logging giúp troubleshoot**

## Troubleshooting

Nếu vẫn có properties sai:
1. Check debug console output
2. So sánh stored vs display values  
3. Verify property definition trong `props.txt`
4. Check special handling trong `getDisplayValueForProperty()`
5. Test với simple properties trước (như resistances)