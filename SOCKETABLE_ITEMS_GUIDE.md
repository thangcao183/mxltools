# Socketable Items Creation Guide - Runes, Gems, Skulls

## 🎯 Tổng quan Socketable Items

Socketable Items bao gồm **Runes**, **Gems**, và **Skulls** - những item có thể được socket vào weapons, armor, shields để cung cấp properties đặc biệt. Đây là hệ thống phức tạp trong MedianXL với properties khác nhau tùy theo loại gear.

## 🏗️ Cấu trúc Socketable Items

### SocketableItemInfo Structure
```cpp
struct SocketableItemInfo {
    struct Properties {
        int code;       // Property ID
        int param;      // Property parameter
        int value;      // Property value

        Properties() : code(-1), param(0), value(0) {}
        Properties(int c, int p, int v) : code(c), param(p), value(v) {}
    };

    enum PropertyType {
        Armor = 0,      // Properties when socketed in armor
        Shield = 1,     // Properties when socketed in shields  
        Weapon = 2      // Properties when socketed in weapons
    };

    QString name;       // Display name (e.g., "Perfect Ruby", "El Rune")
    QString letter;     // Single letter for runeword formation (runes only)
    QHash<PropertyType, QList<Properties>> properties; // Properties by gear type
};
```

### ItemInfo Integration
```cpp
class ItemInfo {
    // Basic socketable data
    QByteArray itemType;          // "r01", "gcv", "skz", etc.
    bool isSocketed;              // Always false for socketables themselves
    ItemsList socketablesInfo;    // For items that CONTAIN socketables (0-6)
    
    // Socketable identification
    bool isGenericSocketable() {
        return ItemDataBase::Socketables()->contains(itemType);
    }
    
    bool isRune() {
        ItemBase *base = ItemDataBase::Items()->value(itemType);
        return base && ItemParser::itemTypesInheritFromType(base->types, "rune");
    }
    
    bool isGem() {
        ItemBase *base = ItemDataBase::Items()->value(itemType);
        return base && ItemParser::itemTypesInheritFromType(base->types, "gem");
    }
};
```

## 📊 Database Management

### Socketables Database Loading
```cpp
QHash<QByteArray, SocketableItemInfo *> *ItemDataBase::Socketables() {
    static QHash<QByteArray, SocketableItemInfo *> allSocketables;
    if (allSocketables.isEmpty()) {
        // Load from compressed "socketables" data file  
        QByteArray fileData = decompressedFileData(
            ResourcePathManager::localizedPathForFileName("socketables"),
            tr("Socketables data not loaded.")
        );
        
        QBuffer buf(&fileData);
        buf.open(QIODevice::ReadOnly);
        
        while (!buf.atEnd()) {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty()) continue;
            
            SocketableItemInfo *item = new SocketableItemInfo;
            item->name = QString::fromUtf8(data.at(1));      // Column 1: Name
            item->letter = QString::fromUtf8(data.at(2));    // Column 2: Letter (runes only)
            
            // Load properties for each gear type (Armor, Shield, Weapon)
            for (int i = SocketableItemInfo::Armor, firstCol = 3; i <= SocketableItemInfo::Weapon; ++i) {
                QList<SocketableItemInfo::Properties> props;
                
                // Each gear type has 3 property slots (9 columns total)
                for (int j = 0; j < 3; ++j, firstCol += 3) {
                    if (firstCol >= data.size()) break;
                    
                    QByteArray codeString = data.at(firstCol);
                    if (codeString.isEmpty()) continue;
                    
                    int param = data.at(firstCol + 1).toInt();
                    int value = data.at(firstCol + 2).toInt();
                    
                    // Handle comma-separated property codes
                    foreach (const QByteArray &code, codeString.split(',')) {
                        props += SocketableItemInfo::Properties(code.toInt(), param, value);
                    }
                }
                
                item->properties[static_cast<SocketableItemInfo::PropertyType>(i)] = props;
            }
            
            // Store by item code (Column 0)
            allSocketables[data.at(0)] = item;
        }
    }
    return &allSocketables;
}
```

### Data File Format
```
# socketables.dat (tab-separated)
Code  Name          Letter  ArmorProp1  ArmorParam1  ArmorVal1  ArmorProp2  ArmorParam2  ArmorVal2  ArmorProp3  ArmorParam3  ArmorVal3  ShieldProp1  ShieldParam1  ShieldVal1  ...
r01   "El Rune"     "El"    7           0            15         31          0            50         0           0            0          7            0            15         ...
gcv   "Chipped Sapphire"  ""  39     0            10         0           0            0          0           0            0          39           0            10         ...
skz   "Skull"       ""      89          0            2          0           0            0          0           0            0          89           0            2          ...
```

## 🎨 Socketable Item Types

### 1. Runes (Runeword Components)
```cpp
// Common rune item codes
QStringList commonRunes = {
    "r01", "r02", "r03", "r04", "r05",  // El, Eld, Tir, Nef, Eth
    "r06", "r07", "r08", "r09", "r10",  // Ith, Tal, Ral, Ort, Thul
    "r11", "r12", "r13", "r14", "r15",  // Amn, Sol, Shael, Dol, Hel
    "r16", "r17", "r18", "r19", "r20",  // Io, Lum, Ko, Fal, Lem
    "r21", "r22", "r23", "r24", "r25",  // Pul, Um, Mal, Ist, Gul
    "r26", "r27", "r28", "r29", "r30",  // Vex, Ohm, Lo, Sur, Ber
    "r31", "r32", "r33"                 // Jah, Cham, Zod
};

// MedianXL specific runes
QStringList medianXLRunes = {
    "rx1", "rx2", "rx3", "rx4", "rx5",  // Xis runes (special MedianXL runes)
    "erun",                             // Enchanted runes
    "xrun"                              // Elemental runes
};

ItemCreationTemplate createRuneTemplate(const QString &runeCode) {
    ItemCreationTemplate tmpl;
    tmpl.itemCode = runeCode;
    tmpl.quality = ItemQuality::Normal;     // Runes are always normal quality
    tmpl.itemLevel = 1;                     // Basic item level
    tmpl.location = ItemLocation::Stored;
    tmpl.isIdentified = true;               // Runes are always identified
    tmpl.isExtended = true;
    
    // Runes are stackable in some cases
    ItemBase *runeBase = ItemDataBase::Items()->value(runeCode);
    if (runeBase && runeBase->isStackable) {
        tmpl.quantity = 1;  // Default quantity
    }
    
    return tmpl;
}
```

### 2. Gems (Enhancement Socketables)
```cpp
// Gem quality levels
enum GemQuality {
    Chipped = 0,    // Lowest tier
    Flawed = 1,     
    Normal = 2,     
    Flawless = 3,   
    Perfect = 4     // Highest tier
};

// Gem types and their codes
QHash<QString, QStringList> gemCodes = {
    {"Ruby", {"gcv", "gfv", "gsv", "gzv", "gpv"}},           // Chipped->Perfect Ruby
    {"Sapphire", {"gcb", "gfb", "gsb", "glb", "gpb"}},       // Chipped->Perfect Sapphire  
    {"Topaz", {"gcg", "gfg", "gsg", "gly", "gpy"}},          // Chipped->Perfect Topaz
    {"Emerald", {"gcr", "gfr", "gsr", "glr", "gpr"}},        // Chipped->Perfect Emerald
    {"Diamond", {"gcw", "gfw", "gsw", "glw", "gpw"}},        // Chipped->Perfect Diamond
    {"Amethyst", {"gca", "gfa", "gsa", "gla", "gpa"}},       // Chipped->Perfect Amethyst
    {"Skull", {"skc", "skf", "sks", "skl", "skz"}}           // Chipped->Perfect Skull
};

ItemCreationTemplate createGemTemplate(const QString &gemType, GemQuality quality) {
    QStringList codes = gemCodes.value(gemType);
    if (codes.isEmpty() || quality >= codes.size()) {
        qWarning() << "Invalid gem type or quality";
        return ItemCreationTemplate();
    }
    
    ItemCreationTemplate tmpl;
    tmpl.itemCode = codes.at(quality);
    tmpl.quality = ItemQuality::Normal;     // Gems are always normal quality
    tmpl.itemLevel = 1;
    tmpl.location = ItemLocation::Stored;
    tmpl.isIdentified = true;
    tmpl.isExtended = true;
    
    // Gems are usually stackable
    tmpl.quantity = 1;
    
    return tmpl;
}
```

### 3. Skulls (Special Socketables)
```cpp
// Skull types (similar to gems but with different properties)
QStringList skullCodes = {
    "skc",  // Chipped Skull
    "skf",  // Flawed Skull  
    "sks",  // Skull
    "skl",  // Flawless Skull
    "skz"   // Perfect Skull
};

ItemCreationTemplate createSkullTemplate(GemQuality quality) {
    if (quality >= skullCodes.size()) {
        qWarning() << "Invalid skull quality";
        return ItemCreationTemplate();
    }
    
    ItemCreationTemplate tmpl;
    tmpl.itemCode = skullCodes.at(quality);
    tmpl.quality = ItemQuality::Normal;
    tmpl.itemLevel = 1;
    tmpl.location = ItemLocation::Stored;
    tmpl.isIdentified = true;
    tmpl.isExtended = true;
    tmpl.quantity = 1;
    
    return tmpl;
}
```

## 🔧 Property System

### Context-Sensitive Properties
```cpp
PropertiesMultiMap getSocketableProperties(ItemInfo *socketable, ItemInfo *hostItem) {
    if (!socketable || !hostItem) return PropertiesMultiMap();
    
    SocketableItemInfo *sockInfo = ItemDataBase::Socketables()->value(socketable->itemType);
    if (!sockInfo) return PropertiesMultiMap();
    
    // Determine host item type for property selection
    SocketableItemInfo::PropertyType hostType;
    ItemBase *hostBase = ItemDataBase::Items()->value(hostItem->itemType);
    
    if (hostBase->socketableType == 0) {
        hostType = SocketableItemInfo::Armor;
    } else if (hostBase->socketableType == 1) {
        hostType = SocketableItemInfo::Shield;  
    } else if (hostBase->socketableType == 2) {
        hostType = SocketableItemInfo::Weapon;
    } else {
        return PropertiesMultiMap(); // Invalid socketable type
    }
    
    // Get properties for specific host type
    QList<SocketableItemInfo::Properties> propsList = sockInfo->properties.value(hostType);
    
    PropertiesMultiMap result;
    foreach (const SocketableItemInfo::Properties &prop, propsList) {
        if (prop.code >= 0) { // Valid property
            ItemProperty *itemProp = new ItemProperty(prop.value, prop.param);
            result.insert(prop.code, itemProp);
        }
    }
    
    return result;
}
```

### Property Examples
```cpp
// Example: Perfect Ruby properties
// In Weapon: +15-20 Fire Damage
// In Armor/Shield: +38 Life

// Example: Perfect Diamond properties  
// In Weapon: +44 Attack Rating
// In Armor: +19 All Resistances
// In Shield: +28 All Resistances

// Example: El Rune properties
// In Weapon: +50 Attack Rating, +1 Light Radius
// In Armor: +15 Defense, +1 Light Radius
// In Shield: +15 Defense, +1 Light Radius
```

## 🎯 Item Creation Implementation

### Socketable Creation Engine
```cpp
class SocketableCreationEngine {
public:
    ItemInfo* createRune(const QString &runeCode) {
        ItemCreationTemplate tmpl = createRuneTemplate(runeCode);
        return ItemCreationEngine::createItem(tmpl);
    }
    
    ItemInfo* createGem(const QString &gemType, GemQuality quality) {
        ItemCreationTemplate tmpl = createGemTemplate(gemType, quality);
        return ItemCreationEngine::createItem(tmpl);
    }
    
    ItemInfo* createSkull(GemQuality quality) {
        ItemCreationTemplate tmpl = createSkullTemplate(quality);
        return ItemCreationEngine::createItem(tmpl);
    }
    
    // Create complete rune set
    QList<ItemInfo*> createRuneSet(int startRune = 1, int endRune = 33) {
        QList<ItemInfo*> runes;
        for (int i = startRune; i <= endRune; ++i) {
            QString runeCode = QString("r%1").arg(i, 2, 10, QChar('0'));
            ItemInfo *rune = createRune(runeCode);
            if (rune) {
                runes.append(rune);
            }
        }
        return runes;
    }
    
    // Create gem set of specific quality
    QList<ItemInfo*> createGemSet(GemQuality quality) {
        QList<ItemInfo*> gems;
        QStringList gemTypes = {"Ruby", "Sapphire", "Topaz", "Emerald", "Diamond", "Amethyst"};
        
        foreach (const QString &gemType, gemTypes) {
            ItemInfo *gem = createGem(gemType, quality);
            if (gem) {
                gems.append(gem);
            }
        }
        return gems;
    }
    
    // Create perfect socketable set (highest quality of each type)
    QList<ItemInfo*> createPerfectSocketableSet() {
        QList<ItemInfo*> items;
        
        // Perfect gems
        items.append(createGemSet(Perfect));
        
        // Perfect skull
        ItemInfo *perfectSkull = createSkull(Perfect);
        if (perfectSkull) {
            items.append(perfectSkull);
        }
        
        // High-level runes (Pul and above)
        for (int i = 21; i <= 33; ++i) {
            QString runeCode = QString("r%1").arg(i, 2, 10, QChar('0'));
            ItemInfo *rune = createRune(runeCode);
            if (rune) {
                items.append(rune);
            }
        }
        
        return items;
    }
};
```

### Runeword Integration
```cpp
class RunewordManager {
public:
    // Check if runes can form a runeword
    bool canFormRuneword(const QList<ItemInfo*> &runes, QString *runewordName = nullptr) {
        QByteArray runeKey;
        foreach (ItemInfo *rune, runes) {
            if (isRune(rune)) {
                runeKey += rune->itemType;
            }
        }
        
        RunewordHash *rwHash = ItemDataBase::RW();
        if (rwHash->contains(runeKey)) {
            RunewordInfo *rwInfo = rwHash->value(runeKey);
            if (runewordName) {
                *runewordName = rwInfo->name;
            }
            return true;
        }
        return false;
    }
    
    // Get rune letters for runeword display
    QString getRuneLetters(const QList<ItemInfo*> &runes) {
        QString letters;
        foreach (ItemInfo *rune, runes) {
            if (isRune(rune)) {
                SocketableItemInfo *sockInfo = ItemDataBase::Socketables()->value(rune->itemType);
                if (sockInfo && !sockInfo->letter.isEmpty()) {
                    letters += sockInfo->letter;
                }
            }
        }
        return letters;
    }
    
private:
    bool isRune(ItemInfo *item) {
        ItemBase *base = ItemDataBase::Items()->value(item->itemType);
        return base && ItemParser::itemTypesInheritFromType(base->types, "rune");
    }
};
```

## 🎮 Socket Operations

### Socketing Items
```cpp
class SocketingEngine {
public:
    bool socketItemIntoHost(ItemInfo *socketable, ItemInfo *hostItem) {
        if (!validateSocketing(socketable, hostItem)) {
            return false;
        }
        
        // Add socketable to host's socketables list
        hostItem->socketablesInfo.append(socketable);
        
        // Update host item properties
        updateHostItemProperties(hostItem);
        
        // Mark as changed
        hostItem->hasChanged = true;
        
        return true;
    }
    
    bool unsocketItem(ItemInfo *socketable, ItemInfo *hostItem) {
        int index = hostItem->socketablesInfo.indexOf(socketable);
        if (index < 0) return false;
        
        // Remove from host
        hostItem->socketablesInfo.removeAt(index);
        
        // Update host properties
        updateHostItemProperties(hostItem);
        
        hostItem->hasChanged = true;
        return true;
    }
    
private:
    bool validateSocketing(ItemInfo *socketable, ItemInfo *hostItem) {
        if (!socketable || !hostItem) return false;
        
        // Check if socketable is actually socketable
        if (!ItemDataBase::isGenericSocketable(socketable)) {
            return false;
        }
        
        // Check if host has available sockets
        if (hostItem->socketablesInfo.size() >= hostItem->socketablesNumber) {
            return false;
        }
        
        // Check if host can accept this socketable type
        ItemBase *hostBase = ItemDataBase::Items()->value(hostItem->itemType);
        if (hostBase->socketableType < 0) {
            return false; // Cannot be socketed
        }
        
        return true;
    }
    
    void updateHostItemProperties(ItemInfo *hostItem) {
        // Recalculate properties based on current socketables
        // This would integrate with existing property calculation system
    }
};
```

## 💡 Usage Examples

### Creating Socketable Items
```cpp
void createSocketableExamples() {
    SocketableCreationEngine engine;
    
    // Create individual items
    ItemInfo *elRune = engine.createRune("r01");           // El rune
    ItemInfo *perfectRuby = engine.createGem("Ruby", Perfect);
    ItemInfo *perfectSkull = engine.createSkull(Perfect);
    
    // Create complete sets
    QList<ItemInfo*> allRunes = engine.createRuneSet();    // r01 to r33
    QList<ItemInfo*> perfectGems = engine.createGemSet(Perfect);
    QList<ItemInfo*> chippedGems = engine.createGemSet(Chipped);
    
    // Create high-end socketables
    QList<ItemInfo*> endgameSocketables = engine.createPerfectSocketableSet();
    
    qDebug() << "Created" << allRunes.size() << "runes";
    qDebug() << "Created" << perfectGems.size() << "perfect gems";
    qDebug() << "Created" << endgameSocketables.size() << "endgame socketables";
}
```

### Socket Operations Example
```cpp
void socketingExample() {
    SocketableCreationEngine sockEngine;
    ItemCreationEngine itemEngine;
    SocketingEngine socketEngine;
    
    // Create a weapon with sockets
    ItemCreationTemplate weaponTmpl = ItemTemplateManager::createWeaponTemplate("lsd");
    weaponTmpl.isSocketed = true;
    weaponTmpl.socketCount = 3;
    ItemInfo *weapon = itemEngine.createItem(weaponTmpl);
    
    // Create socketables
    ItemInfo *rune1 = sockEngine.createRune("r26");  // Vex
    ItemInfo *rune2 = sockEngine.createRune("r27");  // Ohm  
    ItemInfo *gem = sockEngine.createGem("Ruby", Perfect);
    
    // Socket them
    socketEngine.socketItemIntoHost(rune1, weapon);
    socketEngine.socketItemIntoHost(rune2, weapon);
    socketEngine.socketItemIntoHost(gem, weapon);
    
    qDebug() << "Weapon now has" << weapon->socketablesInfo.size() << "socketables";
    
    // Check if runes form a runeword (would need 3+ runes typically)
    RunewordManager rwManager;
    QString runewordName;
    if (rwManager.canFormRuneword(weapon->socketablesInfo, &runewordName)) {
        qDebug() << "Forms runeword:" << runewordName;
    }
}
```

## 🛡️ Validation & Quality Control

### Socketable Validation
```cpp
class SocketableValidator {
public:
    enum ValidationError {
        NoError,
        InvalidItemCode,
        InvalidGemQuality,
        InvalidRuneCode,
        InvalidSkullQuality,
        SocketableNotFound
    };
    
    ValidationError validateSocketableCreation(const ItemCreationTemplate &tmpl, QString *error) {
        // Check if item code exists in database
        if (!ItemDataBase::Items()->contains(tmpl.itemCode)) {
            *error = QString("Item code '%1' not found").arg(tmpl.itemCode);
            return InvalidItemCode;
        }
        
        // Check if it's registered as socketable
        if (!ItemDataBase::Socketables()->contains(tmpl.itemCode)) {
            *error = QString("Item '%1' is not a socketable").arg(tmpl.itemCode);
            return SocketableNotFound;
        }
        
        // Validate socketable-specific constraints
        if (isRuneCode(tmpl.itemCode)) {
            return validateRune(tmpl.itemCode, error);
        } else if (isGemCode(tmpl.itemCode)) {
            return validateGem(tmpl.itemCode, error);
        } else if (isSkullCode(tmpl.itemCode)) {
            return validateSkull(tmpl.itemCode, error);
        }
        
        return NoError;
    }
    
private:
    bool isRuneCode(const QString &code) {
        return code.startsWith("r") || code.startsWith("rx") || 
               code == "erun" || code == "xrun";
    }
    
    bool isGemCode(const QString &code) {
        return code.startsWith("gc") || code.startsWith("gf") || 
               code.startsWith("gs") || code.startsWith("gl") || 
               code.startsWith("gp") || code.startsWith("gz") || code.startsWith("gy");
    }
    
    bool isSkullCode(const QString &code) {
        return code.startsWith("sk");
    }
    
    ValidationError validateRune(const QString &code, QString *error) {
        // Validate rune codes r01-r33, rx1-rx5, erun, xrun
        if (code.startsWith("r") && code.length() == 3) {
            bool ok;
            int runeNum = code.mid(1).toInt(&ok);
            if (!ok || runeNum < 1 || runeNum > 33) {
                *error = QString("Invalid rune number: %1").arg(runeNum);
                return InvalidRuneCode;
            }
        }
        return NoError;
    }
    
    ValidationError validateGem(const QString &code, QString *error) {
        // Validate gem quality progression
        QStringList validPrefixes = {"gc", "gf", "gs", "gl", "gp", "gz", "gy"};
        bool validPrefix = false;
        foreach (const QString &prefix, validPrefixes) {
            if (code.startsWith(prefix)) {
                validPrefix = true;
                break;
            }
        }
        
        if (!validPrefix) {
            *error = QString("Invalid gem code: %1").arg(code);
            return InvalidGemQuality;
        }
        
        return NoError;
    }
    
    ValidationError validateSkull(const QString &code, QString *error) {
        QStringList validSkullCodes = {"skc", "skf", "sks", "skl", "skz"};
        if (!validSkullCodes.contains(code)) {
            *error = QString("Invalid skull code: %1").arg(code);
            return InvalidSkullQuality;
        }
        return NoError;
    }
};
```

## 🎯 Integration với Property System

### Property Calculation for Socketed Items
```cpp
PropertiesMultiMap calculateSocketedItemProperties(ItemInfo *item) {
    PropertiesMultiMap allProperties;
    
    // Add base item properties
    addProperties(&allProperties, item->props);
    
    // Add runeword properties if applicable
    if (item->isRW) {
        addProperties(&allProperties, item->rwProps);
    }
    
    // Add socketable properties
    ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
    foreach (ItemInfo *socketable, item->socketablesInfo) {
        PropertiesMultiMap socketableProps = PropertiesDisplayManager::socketableProperties(
            socketable, itemBase->socketableType
        );
        addProperties(&allProperties, socketableProps);
        
        // Cleanup if temporary properties were created
        if (socketableProps != socketable->props) {
            qDeleteAll(socketableProps);
        }
    }
    
    return allProperties;
}
```

## 🎯 Kết luận

### Key Points về Socketable Items:

1. **Three Main Types**: Runes (runewords), Gems (enhancement), Skulls (special effects)
2. **Context-Sensitive Properties**: Same socketable gives different bonuses in weapons vs armor vs shields
3. **Database Driven**: Properties loaded from `socketables.dat` with gear-type specific columns
4. **Quality Progression**: Gems/skulls have 5 quality levels (chipped→perfect)
5. **Runeword System**: Runes combine to form powerful runewords with unique properties
6. **Stack Support**: Many socketables are stackable for inventory management

### Implementation Files:
- **`src/structs.h`** - SocketableItemInfo structure definitions
- **`src/itemdatabase.cpp`** - Database loading and socketable detection
- **`src/propertiesdisplaymanager.cpp`** - Property calculation logic
- **Item Creation Templates** - For generating socketables programmatically

### Usage Benefits:
- **Complete Socketable Creation** - Generate any rune, gem, or skull
- **Runeword Support** - Create items that can form runewords
- **Property System Integration** - Correctly calculate socketable bonuses
- **Validation System** - Ensure created socketables are valid
- **Batch Operations** - Create complete sets of socketables

Với implementation này, có thể tạo và quản lý đầy đủ hệ thống socketable items của MedianXL! 🔮⚡💎