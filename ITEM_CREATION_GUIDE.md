# Item Creation Guide - Tạo Item Mới Hoàn Toàn Hợp Lệ

## 🎯 Tổng quan

Dựa trên kiến thức đã tích lũy từ Property Editor implementation, hoàn toàn có thể tạo item mới từ đầu một cách an toàn và hợp lệ. Tài liệu này mô tả chi tiết cách thức implementation.

## 🏗️ Kiến trúc Item Creation

### Item Creation Engine
```cpp
class ItemCreationEngine {
    // Core creation methods
    ItemInfo* createItem(const QString &itemCode, int quality = Normal);
    ItemInfo* createItemWithProperties(const ItemCreationTemplate &template);
    
    // Bit string generation
    QString generateItemBitString(const ItemCreationData &data);
    QString generateItemHeader(const ItemCreationData &data);
    QString generatePropertiesBitString(const PropertiesMultiMap &properties);
    
    // Validation
    bool validateItemCreationData(const ItemCreationData &data, QString *error);
    bool validateItemCode(const QString &itemCode);
    bool validateQuality(int quality);
};
```

## 📋 Item Creation Template Structure

### ItemCreationTemplate
```cpp
struct ItemCreationTemplate {
    // Basic item data
    QString itemCode;           // "rin", "amu", "lsd", etc.
    int quality;               // 0=Normal, 1=Low, 2=High, 4=Magic, 5=Set, 6=Rare, 7=Unique, 8=Crafted
    int itemLevel;             // 1-99
    
    // Location data
    int location;              // 0=Stored, 1=Equipped, 2=Belt, 3=Cursor
    int whereEquipped;         // Body position (0-12)
    int row, column;           // Grid position
    int storage;               // Storage type
    
    // Extended item data
    bool isIdentified;
    bool isSocketed;
    bool isEthereal; 
    bool isPersonalized;
    bool isRuneword;
    
    // Quality-specific data
    int setOrUniqueId;         // For set/unique items
    int nonMagicType;          // For low/high quality
    QString inscribedName;     // For personalized items
    
    // Sockets and properties
    int socketCount;
    PropertiesMultiMap properties;
    ItemsList socketedItems;
    
    // Optional overrides
    int defense;               // For armor
    int durability;            // Current durability 
    int maxDurability;         // Max durability
    int quantity;              // For stackable items
};
```

## 🔧 Implementation Steps

### 1. Validate Input Data
```cpp
bool ItemCreationEngine::validateItemCreationData(const ItemCreationData &data, QString *error) {
    // Validate item code exists in database
    ItemBase *itemBase = ItemDataBase::Items()->value(data.itemCode);
    if (!itemBase) {
        *error = QString("Invalid item code: %1").arg(data.itemCode);
        return false;
    }
    
    // Validate quality range
    if (data.quality < 0 || data.quality > 8) {
        *error = QString("Invalid quality: %1").arg(data.quality);
        return false;
    }
    
    // Validate set/unique ID for set/unique items
    if ((data.quality == 5 || data.quality == 7) && data.setOrUniqueId <= 0) {
        *error = "Set/Unique items require valid ID";
        return false;
    }
    
    // Validate socket count
    if (data.socketCount < 0 || data.socketCount > 6) {
        *error = QString("Invalid socket count: %1").arg(data.socketCount);
        return false;
    }
    
    // Validate properties
    foreach (int propId, data.properties.keys()) {
        if (!ItemDataBase::Properties()->contains(propId)) {
            *error = QString("Invalid property ID: %1").arg(propId);
            return false;
        }
    }
    
    return true;
}
```

### 2. Create ItemInfo Object
```cpp
ItemInfo* ItemCreationEngine::createItem(const ItemCreationTemplate &tmpl) {
    QString error;
    if (!validateItemCreationData(tmpl, &error)) {
        qWarning() << "Item creation failed:" << error;
        return nullptr;
    }
    
    // Create new ItemInfo with generated bit string
    QString bitString = generateItemBitString(tmpl);
    ItemInfo *item = new ItemInfo(bitString);
    
    // Set basic properties
    item->itemType = tmpl.itemCode.toLatin1();
    item->quality = tmpl.quality;
    item->ilvl = tmpl.itemLevel;
    item->location = tmpl.location;
    item->whereEquipped = tmpl.whereEquipped;
    item->row = tmpl.row;
    item->column = tmpl.column;
    item->storage = tmpl.storage;
    
    // Set flags
    item->isIdentified = tmpl.isIdentified;
    item->isSocketed = tmpl.isSocketed;
    item->isEthereal = tmpl.isEthereal;
    item->isPersonalized = tmpl.isPersonalized;
    item->isRW = tmpl.isRuneword;
    item->isExtended = true; // Always use extended format
    
    // Set quality-specific data
    if (tmpl.quality == 5 || tmpl.quality == 7) { // Set or Unique
        item->setOrUniqueId = tmpl.setOrUniqueId;
    }
    if (tmpl.quality == 1 || tmpl.quality == 3) { // Low or High quality
        item->nonMagicType = tmpl.nonMagicType;
    }
    if (tmpl.isPersonalized) {
        item->inscribedName = tmpl.inscribedName.toLatin1();
    }
    
    // Generate unique GUID
    item->guid = generateUniqueGuid();
    
    // Set sockets
    item->socketablesNumber = tmpl.socketCount;
    
    // Copy properties
    foreach (int propId, tmpl.properties.keys()) {
        foreach (ItemProperty *prop, tmpl.properties.values(propId)) {
            ItemProperty *newProp = new ItemProperty(prop->value, prop->param);
            item->props.insert(propId, newProp);
        }
    }
    
    // Set durability for items that have it
    ItemBase *itemBase = ItemDataBase::Items()->value(tmpl.itemCode);
    if (itemBase->genericType != Enums::ItemTypeGeneric::Misc) {
        item->maxDurability = tmpl.maxDurability > 0 ? tmpl.maxDurability : calculateBaseDurability(itemBase);
        item->currentDurability = tmpl.durability > 0 ? tmpl.durability : item->maxDurability;
    }
    
    // Set defense for armor
    if (itemBase->genericType == Enums::ItemTypeGeneric::Armor) {
        item->defense = tmpl.defense > 0 ? tmpl.defense : calculateBaseDefense(itemBase);
    }
    
    // Set quantity for stackable items
    if (itemBase->isStackable) {
        item->quantity = tmpl.quantity > 0 ? tmpl.quantity : 1;
    }
    
    item->status = ItemInfo::Ok;
    item->hasChanged = true;
    
    return item;
}
```

### 3. Generate Bit String
```cpp
QString ItemCreationEngine::generateItemBitString(const ItemCreationTemplate &tmpl) {
    QString bitString;
    
    // Generate header section
    bitString += generateItemHeader(tmpl);
    
    // Generate properties section
    bitString += generatePropertiesBitString(tmpl.properties);
    
    // Add properties end marker (511 = 0x1FF)
    bitString += binaryStringFromNumber(511, 9);
    
    // Byte align the bit string
    while (bitString.length() % 8 != 0) {
        bitString += "0";
    }
    
    return bitString;
}

QString ItemCreationEngine::generateItemHeader(const ItemCreationTemplate &tmpl) {
    QString header;
    
    // Flags section (32 bits total)
    header += tmpl.isQuest ? "1" : "0";          // isQuest
    header += "000";                              // padding
    header += tmpl.isIdentified ? "1" : "0";     // isIdentified  
    header += "00000";                           // padding
    header += "0";                               // isDuped
    header += tmpl.isSocketed ? "1" : "0";       // isSocketed
    header += "00";                              // padding
    header += "00";                              // illegal equip + unknown
    header += tmpl.isEar ? "1" : "0";           // isEar
    header += tmpl.isStarter ? "1" : "0";       // isStarter
    header += "00";                              // padding  
    header += "0";                               // padding
    header += "0";                               // isExtended (inverted)
    header += tmpl.isEthereal ? "1" : "0";      // isEthereal
    header += "0";                               // padding
    header += tmpl.isPersonalized ? "1" : "0";   // isPersonalized
    header += "0";                               // padding
    header += tmpl.isRuneword ? "1" : "0";      // isRuneword
    header += "00000";                           // padding
    
    // Version (8 bits) - always 101
    header += binaryStringFromNumber(101, 8);
    
    // Unknown (2 bits)
    header += "00";
    
    // Location data
    header += binaryStringFromNumber(tmpl.location, 3);      // location
    header += binaryStringFromNumber(tmpl.whereEquipped, 4);  // whereEquipped  
    header += binaryStringFromNumber(tmpl.column, 4);        // column
    header += binaryStringFromNumber(tmpl.row, 4);           // row
    header += binaryStringFromNumber(tmpl.storage, 3);       // storage
    
    // Item code (32 bits = 4 characters * 8 bits)
    QByteArray itemCode = tmpl.itemCode.toLatin1();
    itemCode = itemCode.leftJustified(4, '\0'); // Pad to 4 chars
    for (int i = 0; i < 4; ++i) {
        header += binaryStringFromNumber(static_cast<quint8>(itemCode[i]), 8);
    }
    
    // Extended item data (if isExtended)
    if (tmpl.isExtended) {
        header += binaryStringFromNumber(tmpl.socketCount, 3);    // socketablesNumber
        header += binaryStringFromNumber(tmpl.guid, 32);          // guid
        header += binaryStringFromNumber(tmpl.itemLevel, 7);      // ilvl
        header += binaryStringFromNumber(tmpl.quality, 4);        // quality
        
        // Variable graphic
        if (tmpl.variableGraphicIndex > 0) {
            header += "1";
            header += binaryStringFromNumber(tmpl.variableGraphicIndex - 1, 3);
        } else {
            header += "0";
        }
        
        // Auto prefix
        header += "0"; // No auto prefix
        
        // Quality-specific data
        switch (tmpl.quality) {
            case 1: case 3: // Low/High quality
                header += binaryStringFromNumber(tmpl.nonMagicType, 3);
                break;
                
            case 4: // Magic
                header += binaryStringFromNumber(0, 11); // prefix
                header += binaryStringFromNumber(0, 11); // suffix
                break;
                
            case 5: case 7: // Set/Unique
                header += binaryStringFromNumber(tmpl.setOrUniqueId, 15);
                break;
                
            case 6: case 8: // Rare/Crafted
                header += binaryStringFromNumber(0, 8);  // first name
                header += binaryStringFromNumber(0, 8);  // second name
                // 6 possible affixes - all empty for now
                for (int i = 0; i < 6; ++i) {
                    header += "0"; // No affix
                }
                break;
        }
        
        // Personalized name
        if (tmpl.isPersonalized) {
            QByteArray name = tmpl.inscribedName.toLatin1();
            name = name.left(15); // Max 15 characters
            
            for (int i = 0; i < name.length(); ++i) {
                header += binaryStringFromNumber(static_cast<quint8>(name[i]), 7);
            }
            // Null terminator
            header += binaryStringFromNumber(0, 7);
        }
        
        // Type-specific data
        ItemBase *itemBase = ItemDataBase::Items()->value(tmpl.itemCode);
        if (itemBase) {
            if (itemBase->genericType == Enums::ItemTypeGeneric::Armor) {
                header += binaryStringFromNumber(tmpl.defense, 11); // defense
            }
            
            if (itemBase->genericType != Enums::ItemTypeGeneric::Misc) {
                header += binaryStringFromNumber(tmpl.maxDurability, 8);    // max durability
                if (tmpl.maxDurability > 0) {
                    header += binaryStringFromNumber(tmpl.currentDurability, 8); // current durability
                }
            }
            
            if (itemBase->isStackable) {
                header += binaryStringFromNumber(tmpl.quantity, 9); // quantity
            }
        }
        
        // Sockets
        if (tmpl.isSocketed && tmpl.socketCount > 0) {
            header += binaryStringFromNumber(tmpl.socketCount, 4); // socket count
        }
    }
    
    return header;
}

QString ItemCreationEngine::generatePropertiesBitString(const PropertiesMultiMap &properties) {
    QString propBits;
    
    foreach (int propId, properties.keys()) {
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propId);
        if (!propTxt) continue;
        
        foreach (ItemProperty *prop, properties.values(propId)) {
            // Property ID (9 bits)
            propBits += binaryStringFromNumber(propId, 9);
            
            // Parameter (if exists)
            if (propTxt->paramBits > 0) {
                propBits += binaryStringFromNumber(prop->param, propTxt->paramBits);
            }
            
            // Value (with offset)
            int adjustedValue = prop->value + propTxt->add;
            propBits += binaryStringFromNumber(adjustedValue, propTxt->bits);
            
            // Special handling for Enhanced Damage (stored twice)
            if (propId == ItemProperties::EnhancedDamage) {
                propBits += binaryStringFromNumber(adjustedValue, propTxt->bits);
            }
        }
    }
    
    return propBits;
}
```

### 4. Generate Unique GUID
```cpp
quint32 ItemCreationEngine::generateUniqueGuid() {
    // Simple GUID generation - in production, use more sophisticated method
    static quint32 guidCounter = 1;
    quint32 timestamp = QDateTime::currentDateTime().toTime_t();
    return timestamp ^ (guidCounter++);
}
```

## 🎨 Predefined Item Templates

### Common Item Templates
```cpp
class ItemTemplateManager {
public:
    // Utility methods for common item types
    static ItemCreationTemplate createRingTemplate() {
        ItemCreationTemplate tmpl;
        tmpl.itemCode = "rin";
        tmpl.quality = ItemQuality::Normal;
        tmpl.itemLevel = 1;
        tmpl.location = ItemLocation::Stored;
        tmpl.isIdentified = true;
        tmpl.isExtended = true;
        return tmpl;
    }
    
    static ItemCreationTemplate createAmuletTemplate() {
        ItemCreationTemplate tmpl;
        tmpl.itemCode = "amu";
        tmpl.quality = ItemQuality::Normal;
        tmpl.itemLevel = 1;
        tmpl.location = ItemLocation::Stored;
        tmpl.isIdentified = true;
        tmpl.isExtended = true;
        return tmpl;
    }
    
    static ItemCreationTemplate createArmorTemplate(const QString &armorCode) {
        ItemCreationTemplate tmpl;
        tmpl.itemCode = armorCode;
        tmpl.quality = ItemQuality::Normal;
        tmpl.itemLevel = 1;
        tmpl.location = ItemLocation::Stored;
        tmpl.isIdentified = true;
        tmpl.isExtended = true;
        
        // Set base defense
        ItemBase *itemBase = ItemDataBase::Items()->value(armorCode);
        if (itemBase) {
            tmpl.defense = calculateBaseDefense(itemBase);
            tmpl.maxDurability = calculateBaseDurability(itemBase);
            tmpl.currentDurability = tmpl.maxDurability;
        }
        
        return tmpl;
    }
    
    static ItemCreationTemplate createWeaponTemplate(const QString &weaponCode) {
        ItemCreationTemplate tmpl;
        tmpl.itemCode = weaponCode;
        tmpl.quality = ItemQuality::Normal;
        tmpl.itemLevel = 1;
        tmpl.location = ItemLocation::Stored;
        tmpl.isIdentified = true;
        tmpl.isExtended = true;
        
        // Set base durability
        ItemBase *itemBase = ItemDataBase::Items()->value(weaponCode);
        if (itemBase) {
            tmpl.maxDurability = calculateBaseDurability(itemBase);
            tmpl.currentDurability = tmpl.maxDurability;
        }
        
        return tmpl;
    }
    
    // Advanced templates
    static ItemCreationTemplate createMagicItemTemplate(const QString &itemCode) {
        ItemCreationTemplate tmpl = createBaseTemplate(itemCode);
        tmpl.quality = ItemQuality::Magic;
        tmpl.itemLevel = qrand() % 50 + 10; // Level 10-60
        return tmpl;
    }
    
    static ItemCreationTemplate createRareItemTemplate(const QString &itemCode) {
        ItemCreationTemplate tmpl = createBaseTemplate(itemCode);
        tmpl.quality = ItemQuality::Rare;
        tmpl.itemLevel = qrand() % 40 + 30; // Level 30-70
        
        // Add some random properties for rare items
        addRandomProperties(tmpl, 4, 6); // 4-6 properties
        
        return tmpl;
    }
    
    static ItemCreationTemplate createUniqueItemTemplate(const QString &itemCode, int uniqueId) {
        ItemCreationTemplate tmpl = createBaseTemplate(itemCode);
        tmpl.quality = ItemQuality::Unique;
        tmpl.setOrUniqueId = uniqueId;
        tmpl.itemLevel = 50; // Default unique level
        
        // Load unique properties from database
        UniqueItemInfo *uniqueInfo = ItemDataBase::UniqueItems()->value(uniqueId);
        if (uniqueInfo) {
            // Add unique properties
            foreach (const auto &prop, uniqueInfo->properties) {
                ItemProperty *newProp = new ItemProperty(prop.value, prop.param);
                tmpl.properties.insert(prop.propertyId, newProp);
            }
        }
        
        return tmpl;
    }
    
private:
    static void addRandomProperties(ItemCreationTemplate &tmpl, int minProps, int maxProps);
    static int calculateBaseDefense(ItemBase *itemBase);
    static int calculateBaseDurability(ItemBase *itemBase);
};
```

## 💡 Usage Examples

### Basic Usage
```cpp
void createBasicItems() {
    ItemCreationEngine engine;
    
    // Create a simple ring
    ItemCreationTemplate ringTmpl = ItemTemplateManager::createRingTemplate();
    ItemInfo *ring = engine.createItem(ringTmpl);
    
    // Create ring with properties
    ringTmpl.properties.insert(ItemProperties::Strength, new ItemProperty(10, 0)); // +10 Strength
    ringTmpl.properties.insert(ItemProperties::Life, new ItemProperty(20, 0));     // +20 Life
    ItemInfo *magicRing = engine.createItem(ringTmpl);
    
    // Create ethereal weapon
    ItemCreationTemplate swordTmpl = ItemTemplateManager::createWeaponTemplate("lsd"); // Long Sword
    swordTmpl.isEthereal = true;
    swordTmpl.quality = ItemQuality::Magic;
    swordTmpl.properties.insert(ItemProperties::EnhancedDamage, new ItemProperty(200, 0)); // +200% ED
    ItemInfo *ethSword = engine.createItem(swordTmpl);
    
    // Create socketed armor
    ItemCreationTemplate armorTmpl = ItemTemplateManager::createArmorTemplate("lea"); // Leather Armor
    armorTmpl.isSocketed = true;
    armorTmpl.socketCount = 3;
    ItemInfo *socketedArmor = engine.createItem(armorTmpl);
}
```

### Advanced Item Creation
```cpp
void createAdvancedItems() {
    ItemCreationEngine engine;
    
    // Create rare amulet with multiple properties
    ItemCreationTemplate amuTmpl = ItemTemplateManager::createRareItemTemplate("amu");
    amuTmpl.properties.insert(ItemProperties::Strength, new ItemProperty(15, 0));
    amuTmpl.properties.insert(ItemProperties::Dexterity, new ItemProperty(12, 0));
    amuTmpl.properties.insert(ItemProperties::Life, new ItemProperty(50, 0));
    amuTmpl.properties.insert(ItemProperties::Mana, new ItemProperty(30, 0));
    amuTmpl.properties.insert(ItemProperties::AllResistances, new ItemProperty(20, 0));
    ItemInfo *rareAmulet = engine.createItem(amuTmpl);
    
    // Create unique item
    ItemCreationTemplate uniqueTmpl = ItemTemplateManager::createUniqueItemTemplate("rin", 123); // Unique ring ID 123
    ItemInfo *uniqueRing = engine.createItem(uniqueTmpl);
    
    // Create runeword item
    ItemCreationTemplate rwTmpl = ItemTemplateManager::createWeaponTemplate("9ws"); // War Sword
    rwTmpl.isRuneword = true;
    rwTmpl.rwName = "Spirit";
    rwTmpl.isSocketed = true;
    rwTmpl.socketCount = 4;
    // Add runeword properties
    rwTmpl.rwProperties.insert(ItemProperties::FasterCastRate, new ItemProperty(35, 0));
    rwTmpl.rwProperties.insert(ItemProperties::FasterHitRecovery, new ItemProperty(55, 0));
    rwTmpl.rwProperties.insert(ItemProperties::Mana, new ItemProperty(112, 0));
    ItemInfo *runewordWeapon = engine.createItem(rwTmpl);
}
```

## 🛡️ Validation & Safety

### Comprehensive Validation
```cpp
class ItemCreationValidator {
public:
    static bool validateCreationTemplate(const ItemCreationTemplate &tmpl, QString *error) {
        // Basic validation
        if (!validateItemCode(tmpl.itemCode, error)) return false;
        if (!validateQuality(tmpl.quality, error)) return false;
        if (!validateLocation(tmpl.location, tmpl.whereEquipped, error)) return false;
        
        // Properties validation
        if (!validateProperties(tmpl.properties, error)) return false;
        
        // Quality-specific validation
        if (!validateQualitySpecificData(tmpl, error)) return false;
        
        // Logical consistency validation
        if (!validateLogicalConsistency(tmpl, error)) return false;
        
        return true;
    }
    
private:
    static bool validateItemCode(const QString &code, QString *error);
    static bool validateQuality(int quality, QString *error);
    static bool validateLocation(int location, int equipped, QString *error);
    static bool validateProperties(const PropertiesMultiMap &props, QString *error);
    static bool validateQualitySpecificData(const ItemCreationTemplate &tmpl, QString *error);
    static bool validateLogicalConsistency(const ItemCreationTemplate &tmpl, QString *error);
};
```

## 🎯 Integration với UI

### Item Creation Dialog
```cpp
class ItemCreationDialog : public QDialog {
    Q_OBJECT
    
public:
    ItemCreationDialog(QWidget *parent = nullptr);
    ItemInfo* getCreatedItem() const { return _createdItem; }
    
private slots:
    void onItemCodeChanged();
    void onQualityChanged();
    void onCreateClicked();
    void onAddPropertyClicked();
    void onRemovePropertyClicked();
    
private:
    void setupUI();
    void populateItemCodes();
    void populateQualityCombo();
    void updateQualitySpecificFields();
    void addPropertyRow(int propertyId = -1);
    
    QComboBox *_itemCodeCombo;
    QComboBox *_qualityCombo;
    QSpinBox *_itemLevelSpin;
    QCheckBox *_etherealCheck;
    QCheckBox *_socketedCheck;
    QSpinBox *_socketCountSpin;
    
    QVBoxLayout *_propertiesLayout;
    QList<PropertyEditorRow*> _propertyRows;
    
    ItemInfo *_createdItem;
    ItemCreationEngine _engine;
};
```

## 🚀 Performance Optimizations

### Batch Item Creation
```cpp
class BatchItemCreationEngine {
public:
    QList<ItemInfo*> createMultipleItems(const QList<ItemCreationTemplate> &templates) {
        QList<ItemInfo*> items;
        
        foreach (const ItemCreationTemplate &tmpl, templates) {
            ItemInfo *item = _engine.createItem(tmpl);
            if (item) {
                items.append(item);
            }
        }
        
        return items;
    }
    
    // Create items from JSON template
    QList<ItemInfo*> createItemsFromJSON(const QString &jsonData) {
        // Parse JSON and create items
        // Useful for importing item sets
    }
    
private:
    ItemCreationEngine _engine;
};
```

## 🎨 Future Enhancements

### Advanced Features
1. **Item Set Creation** - Tạo cả bộ items cùng lúc
2. **Template Import/Export** - JSON/XML templates
3. **Random Item Generation** - Tạo items ngẫu nhiên theo rules
4. **Item Morphing** - Chuyển đổi item type giữ properties
5. **Bulk Operations** - Tạo hàng loạt items với variations

### Template System
```cpp
class ItemTemplateSystem {
    // Save/Load templates
    void saveTemplate(const ItemCreationTemplate &tmpl, const QString &name);
    ItemCreationTemplate loadTemplate(const QString &name);
    
    // Template categories
    QStringList getTemplatesByCategory(const QString &category);
    
    // Share templates
    QString exportTemplate(const QString &name);
    bool importTemplate(const QString &templateData);
};
```

## 🎯 Kết luận

Việc tạo item mới hoàn toàn khả thi với:

### ✅ **Advantages**
- **Complete Control** - Kiểm soát hoàn toàn mọi aspect của item
- **Flexible Templates** - Dễ dàng tạo variations
- **Safe Generation** - Validation đảm bảo items hợp lệ
- **Integration Ready** - Dễ tích hợp với existing codebase

### 🛠️ **Implementation Priority**
1. **Core Engine** - ItemCreationEngine với basic functionality  
2. **Validation System** - Comprehensive validation
3. **Template Manager** - Predefined templates
4. **UI Integration** - Dialog cho user-friendly creation
5. **Advanced Features** - Batch operations, import/export

### 📚 **Key Files to Implement**
- `src/itemcreationengine.h/.cpp` - Core creation logic
- `src/itemcreationtemplate.h` - Template structures
- `src/itemcreationdialog.h/.cpp` - UI for item creation
- `src/itemtemplatemanager.h/.cpp` - Template management

Với kiến thức từ Property Editor implementation, việc tạo ItemCreationEngine sẽ tương đối straightforward và an toàn! 🚀