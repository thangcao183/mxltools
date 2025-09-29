# MedianXL Offline Tools - Complete Implementation Guide

## 📋 Tổng quan dự án

**MedianXL Offline Tools** là ứng dụng C++ dùng Qt framework để chỉnh sửa character và quản lý item cho Diablo 2 - Median XL mod.

### Thông tin cơ bản
- **Ngôn ngữ**: C++ (compatible với C++03/C++98)
- **Framework**: Qt 4.8.7+ hoặc Qt 5.15.x+
- **Build system**: CMake 3.18+ hoặc QMake
- **Platforms**: Windows, Linux, macOS

## 🏗️ Kiến trúc ứng dụng

### Core Components
```
ItemParser          → Parse D2 save file structure
PropertyEditor      → UI for property modification  
PropertyModificationEngine → Safe property manipulation
ReverseBitReader/Writer → Bit-level operations
PropertiesViewerWidget → Display properties
ItemDatabase        → Property definitions and rules
```

### Data Flow
```
D2 Save File → ItemParser → ItemInfo → PropertyEditor → PropertyModificationEngine → Modified Save File
```

## 📁 Cấu trúc D2 Save File

### Item Structure trong Save File
```
Header Section:
[0-15]  : JM signature (0x4D4A)
[16-17] : Unknown/Padding  
[18-20] : Version (3 bits)
[21-29] : Location flags
[30-31] : Position on body/cursor
[32-63] : Item code (4 characters)
...     : Other item data

Properties Section:
[Property ID: 9 bits][Parameter: variable][Value: variable]
...
[End Marker: 511 (0x1FF)]
```

### Property Format
- **Property ID**: 9 bits identifying property type
- **Parameter**: Variable bits for additional data (skill ID, etc.)  
- **Value**: Variable bits for property value
- **End marker**: Property ID = 511 signals end of properties

## 🔧 Property Editor Implementation

### 1. PropertyEditor Class (`src/propertyeditor.h/.cpp`)

#### Key Features
- **Dynamic property rows** with add/remove capability
- **Real-time validation** with visual feedback
- **Parameter handling** for properties requiring additional data
- **Value range enforcement** based on bit limits
- **Duplicate detection** to prevent conflicting properties
- **Backup/restore** functionality for safe editing

#### Main Methods
```cpp
class PropertyEditor : public QDialog {
    // UI Management
    void addPropertyRow();
    void removePropertyRow(PropertyEditorRow *row);
    void populatePropertyCombo(QComboBox *combo);
    
    // Validation
    void validatePropertyValue(PropertyEditorRow *row);
    QPair<int, int> getValueRange(int propertyId);
    bool isDuplicateProperty(int propertyId, quint32 parameter);
    
    // Property Operations
    void applyPropertyChanges();
    void revertChanges();
    void backupOriginalProperties();
    void restoreOriginalProperties();
    
    // Value Conversion  
    int getDisplayValueFromStorage(int propertyId, int storageValue);
    int getStorageValueFromDisplay(int propertyId, int displayValue);
};
```

### 2. PropertyModificationEngine (`src/propertymodificationengine.h/.cpp`)

#### Purpose
Replace "bit guessing" approach with precise ItemParser logic for safe property modification.

#### Key Methods
```cpp
class PropertyModificationEngine {
    // Main entry point
    bool modifyItemProperties(ItemInfo *item, const PropertiesMultiMap &newProperties);
    
    // Item structure parsing
    bool skipItemBasicData(ReverseBitReader *bitReader);
    int calculatePropertiesStartPosition(ItemInfo *item);
    
    // Bit string operations
    QString reconstructItemBitString(ItemInfo *originalItem, const PropertiesMultiMap &newProperties);
    void appendPropertyToBitString(QString &bitString, int propertyId, ItemProperty *property);
    
    // Validation
    bool validateProperty(int propertyId, int value, quint32 parameter, QString *error);
    int calculateBitLength(int propertyId, int value, quint32 parameter);
};
```

#### Implementation Approach
1. **Parse original item** using ItemParser logic
2. **Skip item header** to reach properties section  
3. **Validate new properties** before applying
4. **Reconstruct bit string** with new properties
5. **Update ItemInfo object** with modified data

## 🔍 Property Types & Validation

### Properties Supporting Negative Values
- **Character Stats**: Strength, Dexterity, Vitality, Energy
- **Life/Mana/Stamina**: Can have penalties
- **Resistances**: Can have vulnerabilities (negative resist)
- **Enhanced Properties**: Defense, Attack Rating can be negative

### Properties Requiring Non-negative Values  
- **Damage**: Enhanced Damage, Min/Max Damage, Elemental Damage
- **Base Defense**: Defense value itself
- **Durability & Quantities**: Cannot be negative

### Special Property Handling
```cpp
// Enhanced Damage - stored twice  
if (propertyId == ItemProperties::EnhancedDamage) {
    appendBits(bitString, adjustedValue, propTxt->bits);
    appendBits(bitString, adjustedValue, propTxt->bits); // Duplicate
}

// Elemental Damage - Min/Max pairs with length
if (isElementalDamage(propertyId)) {
    // Handle min damage, max damage, and length fields
}

// Cold/Poison - Include duration
if (propertyId == ColdDamage || propertyId == PoisonDamage) {
    // Additional length/duration field
}
```

### Value Range Calculation
```cpp
QPair<int, int> getValueRange(int propertyId) {
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    int maxValue = (1 << propTxt->bits) - 1 - propTxt->add;
    int minValue = -propTxt->add;
    
    // Special cases for damage (non-negative)
    if (isDamageProperty(propertyId)) {
        minValue = qMax(0, minValue);
    }
    
    return QPair<int, int>(minValue, maxValue);
}
```

## ⚙️ Bit Manipulation Details

### ReverseBitReader/Writer
```cpp
class ReverseBitReader {
private:
    int _pos; // Position counting from end backwards
    
public:
    int pos() const { return _bitString.length() - _pos; }     // Forward position
    int absolutePos() const { return _pos; }                   // Reverse position
    quint32 readBits(int numBits);                            // Read and advance
};

class ReverseBitWriter {
public:
    static void replaceValueInBitString(QString &bitString, int offset, 
                                      quint32 value, int numBits);
    static void byteAlignBits(QString &bitString);
};
```

### Offset Interpretation (Critical Understanding)
```cpp
// IMPORTANT: bitStringOffset from ItemParser points to parameter+value section
// It does NOT point to the property ID (which is 9 bits before it)

// Property structure: [9-bit PropertyID][Parameter][Value]
//                                      ^
//                                      bitStringOffset points here

// Correct offset usage:
int parameterOffset = bitStringOffset;                    // Parameter at offset  
int valueOffset = bitStringOffset + propTxt->paramBits;   // Value after parameter
```

## 🛡️ Safety Features & Error Handling

### Validation Layers
1. **UI Validation**: Real-time range checking in spinboxes
2. **Property Validation**: Check duplicates, parameter validity  
3. **Bit Validation**: Ensure values fit in allocated bits
4. **Overflow Protection**: Prevent bit string corruption

### Error Recovery
```cpp
try {
    // Property modification operations
    return true;
} catch (const std::exception &e) {
    // Restore original state
    restoreOriginalProperties();
    QMessageBox::critical(this, tr("Error"), 
                         tr("Failed to apply changes: %1").arg(e.what()));
    return false;
}
```

### Memory Management
```cpp
// Qt containers with pointers need manual cleanup
PropertiesMultiMap properties;
// ... populate map with new ItemProperty*

// CRITICAL: Always cleanup
qDeleteAll(properties);  // Delete all ItemProperty* objects
properties.clear();      // Clear the map
```

## 🔧 Build & Integration

### CMake Integration
```cmake
# Add to CMakeLists.txt
set(SOURCES
    src/propertyeditor.cpp
    src/propertymodificationengine.cpp
    # ... other sources
)

set(HEADERS  
    src/propertyeditor.h
    src/propertymodificationengine.h
    # ... other headers
)
```

### UI Integration
```cpp
// In PropertiesViewerWidget
void PropertiesViewerWidget::openPropertyEditor() {
    if (!_currentItem) return;
    
    PropertyEditor *editor = new PropertyEditor(_currentItem, this);
    connect(editor, &PropertyEditor::propertiesChanged, 
            this, &PropertiesViewerWidget::refreshProperties);
    editor->show();
}
```

## 🧪 Testing Strategies

### Unit Tests
```cpp
void testPropertyModification() {
    // Create test item with known properties
    ItemInfo testItem;
    testItem.bitString = createTestBitString();
    ItemParser::parseItem(&testItem);
    
    // Modify property using engine
    PropertyModificationEngine engine;
    PropertiesMultiMap newProps;
    newProps.insert(17, new ItemProperty(100, 0)); // +100 Strength
    
    // Verify modification
    QVERIFY(engine.modifyItemProperties(&testItem, newProps));
    QCOMPARE(testItem.props.value(17)->value, 100);
    
    // Cleanup
    qDeleteAll(newProps);
}
```

### Integration Tests
- Load real save files
- Modify properties through UI
- Save and reload to verify persistence
- Test edge cases (max values, negative values)

## 🚨 Common Issues & Solutions

### 1. Bit String Length Changes
**Problem**: Reconstruction changes bit string length (e.g., 344→328 bits)
```cpp
// AVOID: Full reconstruction
item->bitString = reconstructItemBitString(item, newProps);

// USE: Direct bit updates  
ReverseBitWriter::replaceValueInBitString(item->bitString, offset, value, bits);
```

### 2. Property ID = 0 Handling  
**Problem**: Strength property (ID=0) being filtered out
```cpp
// WRONG:
if (propertyId > 0) { /* process */ }

// CORRECT:
if (propertyId >= 0) { /* process */ }
```

### 3. Offset Calculation Errors
**Problem**: Misunderstanding bitStringOffset meaning
```cpp
// WRONG: Thinking offset points to property ID
int valueOffset = bitStringOffset + 9; // Skip property ID

// CORRECT: Offset already points to parameter section
int paramOffset = bitStringOffset;
int valueOffset = bitStringOffset + paramBits;
```

## 📈 Performance Optimizations

### Batch Operations
```cpp
// Collect all changes before applying
QMap<int, PropertyChange> allChanges;
collectAllChanges(allChanges);

// Apply all at once instead of individual updates
applyBatchChanges(item, allChanges);
```

### Avoid Repeated Parsing
```cpp
// BAD: Parse item multiple times
for (each property) {
    ItemParser::parseItem(item); // Expensive!
    modifyProperty(item, prop);
}

// GOOD: Parse once, modify many
ItemParser::parseItem(item);
for (each property) {
    modifyPropertyDirect(item, prop);
}
```

## 🔮 Future Enhancements

### Property Templates
```cpp
class PropertyTemplate {
    QString name;
    QMap<int, PropertyValue> properties;
    void applyToItem(ItemInfo *item);
};
```

### Undo/Redo System
```cpp  
class PropertyModificationHistory {
    QStack<ItemSnapshot> undoStack;
    QStack<ItemSnapshot> redoStack;
    void saveSnapshot(ItemInfo *item);
    void undo();
    void redo();
};
```

### Advanced Validation
- Cross-property validation rules
- Item type specific restrictions  
- Logical consistency checks

## 🎯 Key Implementation Lessons

1. **Understand data structure thoroughly** before manipulation
2. **Reuse existing parsing logic** instead of guessing
3. **Implement comprehensive error handling** for robustness
4. **Memory management is critical** with Qt containers
5. **Thorough testing prevents data corruption**
6. **Direct bit updates are safer** than full reconstruction  
7. **Property ID 0 is valid** and must be handled properly

## 📚 Critical Files Reference

### Source Files
- `src/itemparser.cpp` - Core parsing logic
- `src/propertyeditor.cpp` - UI and validation
- `src/propertymodificationengine.cpp` - Safe modification engine
- `src/reversebitreader.cpp` - Bit reading operations
- `src/reversebitwriter.cpp` - Bit writing operations
- `src/propertiesviewerwidget.cpp` - Integration point

### Data Files  
- `Properties.txt` - Property definitions
- `ItemStatCost.txt` - Stat definitions and rules
- Character save files (`.d2s`) - Binary data format

## 🎨 Advanced Feature: Item Creation

Dựa trên kiến thức Property Editor, có thể implement tính năng **Item Creation** để tạo item mới hoàn toàn hợp lệ.

### ItemCreationEngine Architecture
```cpp
class ItemCreationEngine {
    ItemInfo* createItem(const ItemCreationTemplate &template);
    QString generateItemBitString(const ItemCreationData &data);
    QString generateItemHeader(const ItemCreationData &data);
    QString generatePropertiesBitString(const PropertiesMultiMap &properties);
    bool validateItemCreationData(const ItemCreationData &data, QString *error);
};
```

### Item Creation Process
1. **Validate Template** - Check item code, quality, properties
2. **Generate Header** - Create item header section with flags, location, extended data
3. **Generate Properties** - Build properties bit string using same logic as Property Editor
4. **Create ItemInfo** - Populate ItemInfo object with generated data
5. **Set Relationships** - Link with database objects (ItemBase, PropertyTxt, etc.)

### Key Implementation Points
- **Reuse Property Logic** - Same bit manipulation as Property Editor
- **Template System** - Predefined templates for common item types  
- **Comprehensive Validation** - Ensure all created items are valid
- **GUID Generation** - Unique identifiers for each created item
- **Integration Ready** - Works seamlessly with existing ItemParser/PropertyEditor

### Example Usage
```cpp
ItemCreationTemplate tmpl = ItemTemplateManager::createRingTemplate();
tmpl.quality = ItemQuality::Magic;
tmpl.properties.insert(ItemProperties::Strength, new ItemProperty(10, 0));
tmpl.properties.insert(ItemProperties::Life, new ItemProperty(20, 0));

ItemCreationEngine engine;
ItemInfo *newRing = engine.createItem(tmpl);
```

*See `ITEM_CREATION_GUIDE.md` for complete implementation details.*

---

**Document Created**: September 30, 2025  
**Author**: GitHub Copilot  
**Project**: MedianXL Offline Tools  
**Purpose**: Complete implementation reference for future development in any programming language