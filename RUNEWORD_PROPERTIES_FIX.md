# Runeword Properties Support Fix

## 🔍 Phát hiện vấn đề (Issue Discovered)

Khi mở Property Editor cho items runewords thì không thấy properties nào. Vấn đề là PropertyEditor chỉ load properties từ `item->props` mà bỏ qua `item->rwProps` (runeword properties).

### Nguyên nhân (Root Cause)
- **Runeword properties** được lưu riêng trong `item->rwProps`  
- **Item properties** được lưu trong `item->props`
- PropertyEditor chỉ load từ `props` mà không load từ `rwProps`
- Khi apply changes, code chỉ tìm property trong `props` không tìm trong `rwProps`

## ✅ Files đã sửa (Fixed Files)

### `/src/propertyeditor.h`
**Added new fields to PropertyEditorRow:**
```cpp
QLabel *typeLabel;           // Shows "Item" or "RW" indicator
bool isRunewordProperty;     // Tracks property type
```

**Updated method signature:**
```cpp
void addPropertyRow(int propertyId = -1, int value = 0, quint32 parameter = 0, 
                   bool isNew = true, bool isRunewordProperty = false);
```

### `/src/propertyeditor.cpp`

#### 1. **setItem()** - Load runeword properties:
```cpp
// Add item properties (existing only)
for (auto it = _item->props.constBegin(); it != _item->props.constEnd(); ++it) {
    int displayValue = getDisplayValueForProperty(it.key(), it.value());
    addPropertyRow(it.key(), displayValue, it.value()->param, false, false);
}

// Add runeword properties (existing only) - NEW!
for (auto it = _item->rwProps.constBegin(); it != _item->rwProps.constEnd(); ++it) {
    int displayValue = getDisplayValueForProperty(it.key(), it.value());
    addPropertyRow(it.key(), displayValue, it.value()->param, false, true);
}
```

#### 2. **applyChanges()** - Find properties in both places:
```cpp
// Find existing property in item (check both props and rwProps)
ItemProperty *existingProp = _item->props.value(propertyId);
bool isRunewordProperty = false;

if (!existingProp) {
    existingProp = _item->rwProps.value(propertyId);
    isRunewordProperty = true;
}

if (!existingProp) {
    // Property doesn't exist in either props or rwProps - skip for safety
    continue;
}
```

#### 3. **revertChanges()** - Restore runeword properties:
```cpp
// Restore original runeword properties - NEW!
qDeleteAll(_item->rwProps);
_item->rwProps.clear();

for (auto it = _originalRwProperties.constBegin(); it != _originalRwProperties.constEnd(); ++it) {
    _item->rwProps.insert(it.key(), new ItemProperty(*it.value()));
}
```

#### 4. **addPropertyRow()** - Visual indicator:
```cpp
// Type label (Item/RW indicator)
row->typeLabel = new QLabel(isRunewordProperty ? tr("RW") : tr("Item"));
row->typeLabel->setStyleSheet(isRunewordProperty ? 
                             "QLabel { color: green; font-weight: bold; }" :
                             "QLabel { color: blue; font-weight: bold; }");
row->typeLabel->setToolTip(isRunewordProperty ? 
                          tr("Runeword Property") : 
                          tr("Item Property"));
```

#### 5. **Status label** - Shows counts:
```cpp
int totalProps = _item->props.size() + _item->rwProps.size();
_statusLabel->setText(tr("Loaded %1 existing properties (%2 item + %3 runeword) - modify values only")
                     .arg(totalProps).arg(_item->props.size()).arg(_item->rwProps.size()));
```

## 🎯 Kết quả sau khi fix (Results After Fix):

### ✅ **What now works:**
- **Runeword items** show all properties in Property Editor (both item + runeword)
- **Visual distinction**: 
  - 🔵 **"Item"** label (blue) for item properties
  - 🟢 **"RW"** label (green) for runeword properties  
- **Full editing support** for runeword properties
- **Proper apply/revert** for both property types
- **Accurate status** showing breakdown of property counts

### 🖼️ **UI Changes:**
```
[RW] Property: +2 to All Skills        Value: 2    [Remove]
[Item] Property: Enhanced Damage        Value: 150  [Remove] 
[RW] Property: 25% Faster Cast Rate     Value: 25   [Remove]
```

### 📊 **Status Examples:**
- `"Loaded 5 existing properties (2 item + 3 runeword) - modify values only"`
- `"Loaded 3 existing properties (3 item + 0 runeword) - modify values only"`

## 🧪 Testing Checklist

- [x] Load character with runeword items  
- [x] Open Property Editor for runeword item
- [x] ✅ **Now shows both item AND runeword properties**
- [x] Visual indicators work correctly (Item/RW labels)
- [x] Can edit values for both types
- [x] Apply changes works for both types  
- [x] Revert changes works for both types
- [x] Status label shows accurate counts
- [x] No regression with regular items

## 📋 Technical Details

### Property Storage Structure:
```cpp
class ItemInfo {
    PropertiesMultiMap props;    // Item base properties  
    PropertiesMultiMap rwProps;  // Runeword properties (was ignored!)
    // ...
}
```

### Before vs After:

| Scenario | Before | After |
|----------|---------|-------|
| Regular Item | ✅ Shows props | ✅ Shows props |
| Runeword Item | ❌ Shows nothing | ✅ Shows props + rwProps |
| Visual Distinction | ❌ No indication | ✅ "Item"/"RW" labels |
| Property Count | ❌ Wrong count | ✅ Accurate count |

---
**Date:** 2025-09-29  
**Status:** ✅ FIXED  
**Impact:** 🔧 Major functionality improvement - runeword editing now works!  
**Type:** Feature completion (was partially implemented)

## 🎉 Now you can fully edit runeword properties! 
Select any runeword item and open Property Editor to see both item and runeword properties with clear visual distinction.
