# Unique Items Implementation Guide - MedianXL Offline Tools

## 🎯 Tổng quan Unique Items

Unique Items là một loại item đặc biệt trong Diablo 2 với tên riêng, properties cố định và thường có màu vàng (golden color). Trong MedianXL Offline Tools, unique items được quản lý qua hệ thống database phức tạp.

## 🏗️ Cấu trúc Unique Items

### UniqueItemInfo Structure
```cpp
struct SetOrUniqueItemInfo {
    quint16 rlvl;          // Required level to drop
    quint16 ilvl;          // Internal item level
    QByteArray imageName;   // Custom image file name
};

struct UniqueItemInfo : public SetOrUniqueItemInfo {
    QString name;          // Unique item name (e.g., "The Stone of Jordan")
};
```

### ItemInfo Integration
```cpp
class ItemInfo {
    // Basic item data
    QByteArray itemType;     // Base item type (e.g., "rin" for rings)
    quint8 quality;          // 7 = Unique quality
    int setOrUniqueId;       // ID to lookup UniqueItemInfo in database
    
    // Properties from unique item
    PropertiesMultiMap props; // Fixed properties from unique definition
    
    // Display
    QString getName() {
        if (quality == ItemQuality::Unique) {
            UniqueItemInfo *unique = ItemDataBase::Uniques()->value(setOrUniqueId);
            return unique ? unique->name : "Unknown Unique";
        }
    }
};
```

## 📊 Database Management

### UniqueItemInfo Database Loading
```cpp
QHash<uint, UniqueItemInfo *> *ItemDataBase::Uniques() {
    static QHash<uint, UniqueItemInfo *> allUniques;
    if (allUniques.isEmpty()) {
        // Load from compressed "uniques" data file
        QByteArray fileData = decompressedFileData(
            ResourcePathManager::localizedPathForFileName("uniques"), 
            tr("Uniques data not loaded.")
        );
        
        QBuffer buf(&fileData);
        buf.open(QIODevice::ReadOnly);
        
        while (!buf.atEnd()) {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty()) continue;
            
            UniqueItemInfo *uniqueItem = new UniqueItemInfo;
            uniqueItem->name = QString::fromUtf8(data.at(1));    // Column 1: Name
            uniqueItem->rlvl = data.at(2).toUShort();            // Column 2: Required Level
            uniqueItem->ilvl = data.at(3).toUShort();            // Column 3: Item Level  
            if (data.size() > 4) {
                uniqueItem->imageName = data.at(4);              // Column 4: Image
            }
            
            // Store by unique ID (Column 0)
            allUniques[data.at(0).toUInt()] = uniqueItem;
        }
    }
    return &allUniques;
}
```

### Data File Format
```
# uniques.dat (tab-separated)
ID    Name                   RLvl  ILvl  Image
123   "The Stone of Jordan"  39    39     "soj"
456   "Shako"               62    62     "shako"
789   "Enigma Base"         65    65     ""
```

## 🔍 Unique Item Detection & Validation

### Identifying Unique Items
```cpp
bool isUniqueItem(ItemInfo *item) {
    return item->quality == Enums::ItemQuality::Unique;
}

bool areBothItemsSetOrUnique(ItemInfo *a, ItemInfo *b) {
    return (a->quality == Enums::ItemQuality::Unique && b->quality == Enums::ItemQuality::Unique) || 
           (a->quality == Enums::ItemQuality::Set && b->quality == Enums::ItemQuality::Set);
}

UniqueItemInfo* getUniqueInfo(ItemInfo *item) {
    if (item->quality != Enums::ItemQuality::Unique) {
        return nullptr;
    }
    return ItemDataBase::Uniques()->value(item->setOrUniqueId);
}
```

### Validation Functions
```cpp
bool validateUniqueItem(ItemInfo *item, QString *error) {
    if (item->quality != ItemQuality::Unique) {
        *error = "Item is not unique quality";
        return false;
    }
    
    if (item->setOrUniqueId <= 0) {
        *error = "Invalid unique ID";
        return false;
    }
    
    UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId);
    if (!uniqueInfo) {
        *error = QString("Unique ID %1 not found in database").arg(item->setOrUniqueId);
        return false;
    }
    
    return true;
}
```

## 🎨 Display & Naming

### Complete Item Name Generation
```cpp
QString ItemDataBase::completeItemName(ItemInfo *item, bool shouldUseColor, bool showQualityText) {
    QString itemName, specialName;
    
    // Get base item name
    ItemBase *itemBase = Items()->value(item->itemType);
    itemName = itemBase ? itemBase->name : QString("Unknown Item");
    
    // Get unique name
    if (item->quality == Enums::ItemQuality::Unique) {
        UniqueItemInfo *uniqueInfo = Uniques()->value(item->setOrUniqueId);
        specialName = uniqueInfo ? uniqueInfo->name : QString();
    }
    
    // Display logic
    if (shouldUseColor) {
        // HTML formatted with color codes
        QString colorCode = getColorCode(item->quality);
        if (!specialName.isEmpty()) {
            itemName.prepend(QString("<font color=\"%1\">%2</font><br>")
                           .arg(colorCode).arg(specialName));
        }
    } else {
        // Plain text
        if (!specialName.isEmpty() && specialName != itemName) {
            itemName += "\\n" + specialName;
        }
        
        if (showQualityText) {
            itemName += " (unique)";
        }
    }
    
    return itemName;
}
```

### Color Management
```cpp
ColorsManager::ColorIndex colorOfItem(ItemInfo *item) {
    switch (item->quality) {
        case ItemQuality::Unique:
            return ColorsManager::Gold;  // Golden color for uniques
        case ItemQuality::Set:
            return ColorsManager::Green;
        case ItemQuality::Rare:
            return ColorsManager::Yellow;
        case ItemQuality::Magic:
            return ColorsManager::Blue;
        default:
            return ColorsManager::White;
    }
}
```

## 🔧 Parsing & Bit String Structure

### Unique Item Parsing
```cpp
ItemInfo *ItemParser::parseItem(QDataStream &inputDataStream, const QByteArray &bytes) {
    // ... standard parsing ...
    
    switch (item->quality) {
        case Enums::ItemQuality::Set:
        case Enums::ItemQuality::Unique:
            // Read 15-bit unique/set ID
            item->setOrUniqueId = bitReader.readNumber(15);
            break;
        // ... other quality cases ...
    }
    
    // ... continue parsing properties ...
    return item;
}
```

### Bit String Format for Unique Items
```
Item Header:
[JM Header: 16 bits]
[Flags: 32 bits] - Including quality bits
[Extended Data:]
  [Socket Count: 3 bits]
  [GUID: 32 bits] 
  [Item Level: 7 bits]
  [Quality: 4 bits] - Value = 7 for Unique
  [Variable Graphic: 1+3 bits]
  [Auto Prefix: 1+11 bits if set]
  [Set/Unique ID: 15 bits] ← Key to lookup UniqueItemInfo
  
Properties Section:
[Property 1: ID(9) + Param(var) + Value(var)]
[Property 2: ID(9) + Param(var) + Value(var)]
...
[End Marker: 511 (9 bits)]
```

## 🎯 Item Creation für Unique Items

### Creating Unique Items
```cpp
ItemInfo* createUniqueItem(const QString &baseItemCode, int uniqueId) {
    // Validate unique ID exists
    UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(uniqueId);
    if (!uniqueInfo) {
        qWarning() << "Unique ID" << uniqueId << "not found";
        return nullptr;
    }
    
    // Create item template
    ItemCreationTemplate tmpl;
    tmpl.itemCode = baseItemCode;
    tmpl.quality = ItemQuality::Unique;
    tmpl.setOrUniqueId = uniqueId;
    tmpl.itemLevel = uniqueInfo->ilvl;
    tmpl.isIdentified = true;  // Uniques are usually identified
    tmpl.isExtended = true;
    
    // Generate unique GUID
    tmpl.guid = generateUniqueGuid();
    
    // Set location (default to inventory)
    tmpl.location = ItemLocation::Stored;
    tmpl.storage = ItemStorage::Inventory;
    
    // Load unique properties from database (if available)
    // Note: Properties are usually loaded from separate source
    // as UniqueItemInfo only contains basic metadata
    
    return ItemCreationEngine::createItem(tmpl);
}
```

### Batch Unique Item Creation
```cpp
QList<ItemInfo*> createUniqueItemSet(const QList<int> &uniqueIds) {
    QList<ItemInfo*> items;
    
    foreach (int uniqueId, uniqueIds) {
        UniqueItemInfo *info = ItemDataBase::Uniques()->value(uniqueId);
        if (!info) continue;
        
        // Determine base item type from unique definition
        // This would require additional mapping in real implementation
        QString baseCode = determineBaseItemCode(uniqueId);
        
        ItemInfo *item = createUniqueItem(baseCode, uniqueId);
        if (item) {
            items.append(item);
        }
    }
    
    return items;
}
```

## 🎮 Item Property Management

### Loading Unique Properties
```cpp
// Note: Actual property loading would require additional database files
// as UniqueItemInfo only contains metadata
PropertiesMultiMap loadUniqueProperties(int uniqueId) {
    PropertiesMultiMap properties;
    
    // This would typically load from UniqueItems.txt or similar
    // which contains the actual property definitions
    
    // Example: Stone of Jordan properties
    if (uniqueId == 123) { // SoJ
        properties.insert(ItemProperties::Mana, new ItemProperty(20, 0));           // +20 Mana
        properties.insert(ItemProperties::ManaRegen, new ItemProperty(25, 0));      // +25% Mana Regen
        properties.insert(ItemProperties::LightningResist, new ItemProperty(30, 0)); // +30% Lightning Resist
        properties.insert(ItemProperties::AllSkills, new ItemProperty(1, 0));       // +1 All Skills
    }
    
    return properties;
}

void applyUniqueProperties(ItemInfo *item, int uniqueId) {
    if (item->quality != ItemQuality::Unique) return;
    
    PropertiesMultiMap uniqueProps = loadUniqueProperties(uniqueId);
    
    // Clear existing properties and apply unique ones
    qDeleteAll(item->props);
    item->props.clear();
    
    // Copy unique properties
    foreach (int propId, uniqueProps.keys()) {
        foreach (ItemProperty *prop, uniqueProps.values(propId)) {
            ItemProperty *newProp = new ItemProperty(prop->value, prop->param);
            item->props.insert(propId, newProp);
        }
    }
    
    // Update bit string to reflect new properties
    // This would use PropertyModificationEngine
    
    // Cleanup
    qDeleteAll(uniqueProps);
}
```

## 📈 Comparison & Sorting

### Sorting Unique Items
```cpp
bool compareItemsByRlvl(ItemInfo *a, ItemInfo *b) {
    if (areBothItemsSetOrUnique(a, b) && isSacred(a) && isSacred(b)) {
        // For unique items, compare by ilvl from unique database
        if (UniqueItemInfo *ua = ItemDataBase::Uniques()->value(a->setOrUniqueId)) {
            if (UniqueItemInfo *ub = ItemDataBase::Uniques()->value(b->setOrUniqueId)) {
                return ua->ilvl < ub->ilvl;
            }
        }
        // Fallback to ID comparison
        return a->setOrUniqueId < b->setOrUniqueId;
    }
    
    // For non-unique items, compare by base item rlvl
    ItemBase *baseA = ItemDataBase::Items()->value(a->itemType);
    ItemBase *baseB = ItemDataBase::Items()->value(b->itemType);
    return baseA->rlvl < baseB->rlvl;
}
```

### Unique Item Filtering
```cpp
QList<ItemInfo*> filterUniqueItems(const ItemsList &items) {
    QList<ItemInfo*> uniques;
    
    foreach (ItemInfo *item, items) {
        if (item->quality == ItemQuality::Unique) {
            uniques.append(item);
        }
    }
    
    return uniques;
}

QHash<int, QList<ItemInfo*>> groupUniquesByType(const ItemsList &items) {
    QHash<int, QList<ItemInfo*>> grouped;
    
    foreach (ItemInfo *item, items) {
        if (item->quality == ItemQuality::Unique) {
            grouped[item->setOrUniqueId].append(item);
        }
    }
    
    return grouped;
}
```

## 🛡️ Validation & Error Handling

### Comprehensive Unique Item Validation
```cpp
enum UniqueValidationError {
    NoError,
    InvalidQuality,
    InvalidUniqueId,
    UniqueNotFound,
    BaseItemMismatch,
    PropertyMismatch
};

UniqueValidationError validateUniqueItem(ItemInfo *item, QString *errorDetails) {
    if (item->quality != ItemQuality::Unique) {
        *errorDetails = "Item quality is not unique";
        return InvalidQuality;
    }
    
    if (item->setOrUniqueId <= 0) {
        *errorDetails = "Invalid unique ID";
        return InvalidUniqueId;
    }
    
    UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId);
    if (!uniqueInfo) {
        *errorDetails = QString("Unique ID %1 not found in database").arg(item->setOrUniqueId);
        return UniqueNotFound;
    }
    
    // Validate item level matches unique requirements
    if (item->ilvl < uniqueInfo->ilvl) {
        *errorDetails = QString("Item level %1 below required %2").arg(item->ilvl).arg(uniqueInfo->ilvl);
        return PropertyMismatch;
    }
    
    return NoError;
}
```

## 🎪 Special Unique Item Types

### MedianXL Special Uniques
```cpp
// MedianXL specific unique items detection
bool isUberCharm(ItemInfo *item) {
    // Check if item is an uber charm (special unique type)
    return item->quality == ItemQuality::Unique && 
           item->itemType.startsWith("cm") && // Charm base type
           isSpecialUberCharmId(item->setOrUniqueId);
}

bool isTiered(ItemInfo *item) {
    if (!item) return false;
    ItemBase *base = ItemDataBase::Items()->value(item->itemType);
    return base && base->types.contains("tier");
}

bool isSacred(ItemInfo *item) {
    return !isTiered(item);
}

// Special unique charms
bool isCrystallineFlameMedallion(ItemInfo *item) {
    return item->quality == ItemQuality::Unique && 
           item->setOrUniqueId == 1234; // Specific unique ID
}

bool isMoonOfSpider(ItemInfo *item) {
    return item->quality == ItemQuality::Unique && 
           item->setOrUniqueId == 5678; // Specific unique ID
}
```

## 🎯 Integration với Property Editor

### Modifying Unique Item Properties
```cpp
void PropertyEditor::setupForUniqueItem(ItemInfo *item) {
    if (item->quality != ItemQuality::Unique) return;
    
    UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId);
    if (!uniqueInfo) return;
    
    // Display unique name in editor
    setWindowTitle(QString("Edit Properties: %1").arg(uniqueInfo->name));
    
    // Load existing properties (both base and unique)
    populatePropertiesFromItem(item);
    
    // Add note about unique item modifications
    addWarningLabel(tr("Warning: Modifying unique item properties may make it invalid"));
}

bool PropertyEditor::validateUniqueItemModification(ItemInfo *item) {
    if (item->quality != ItemQuality::Unique) return true;
    
    // Check if modifications maintain unique item integrity
    UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId);
    if (!uniqueInfo) return false;
    
    // Validate against unique item constraints
    return validateUniqueConstraints(item, uniqueInfo);
}
```

## 📚 Database Integration Examples

### Loading Unique Database
```cpp
void loadUniqueItemDatabase() {
    qDebug() << "Loading unique items database...";
    
    QHash<uint, UniqueItemInfo *> *uniques = ItemDataBase::Uniques();
    
    if (!uniques || uniques->isEmpty()) {
        qWarning() << "Failed to load unique items database";
        return;
    }
    
    qDebug() << "Loaded" << uniques->size() << "unique items";
    
    // Log some examples
    foreach (uint id, uniques->keys()) {
        UniqueItemInfo *info = uniques->value(id);
        qDebug() << "Unique ID:" << id << "Name:" << info->name 
                 << "RLvl:" << info->rlvl << "ILvl:" << info->ilvl;
        
        if (id > 10) break; // Just show first few
    }
}
```

### Finding Unique Items
```cpp
QList<UniqueItemInfo*> findUniquesByName(const QString &searchName) {
    QList<UniqueItemInfo*> results;
    QHash<uint, UniqueItemInfo *> *uniques = ItemDataBase::Uniques();
    
    foreach (UniqueItemInfo *unique, uniques->values()) {
        if (unique->name.contains(searchName, Qt::CaseInsensitive)) {
            results.append(unique);
        }
    }
    
    return results;
}

UniqueItemInfo* findUniqueByExactName(const QString &name) {
    QHash<uint, UniqueItemInfo *> *uniques = ItemDataBase::Uniques();
    
    foreach (UniqueItemInfo *unique, uniques->values()) {
        if (unique->name == name) {
            return unique;
        }
    }
    
    return nullptr;
}
```

## 🎯 Kết luận

### Key Points về Unique Items:
1. **Database Driven** - Unique items được load từ compressed data files
2. **ID Based Lookup** - Sử dụng `setOrUniqueId` để lookup trong database
3. **Metadata Only** - UniqueItemInfo chỉ chứa metadata (name, levels), không chứa properties
4. **Quality = 7** - Unique items có quality value = 7
5. **15-bit ID Space** - Unique IDs được stored trong 15 bits
6. **Color Coded** - Hiển thị màu vàng (golden) trong UI
7. **Special Handling** - Cần xử lý đặc biệt cho MedianXL uber charms và tiered items

### Files quan trọng:
- **`src/structs.h`** - Định nghĩa UniqueItemInfo structure
- **`src/itemdatabase.cpp`** - Database loading và management  
- **`src/itemparser.cpp`** - Parsing unique items từ save files
- **Data files** - `uniques.dat` chứa unique item definitions

Unique items là một phần phức tạp của MedianXL system nhưng với implementation này có thể handle đầy đủ! 🌟