# Property Editor - Negative Values Support

## Tổng quan

Property Editor hiện đã hỗ trợ đầy đủ các properties có thể có giá trị âm (negative values). Đây là danh sách các loại properties và cách xử lý của chúng.

## Properties có thể có giá trị âm

### 1. Character Stats (Thuộc tính nhân vật)
- **Strength** - Có thể âm (curses, debuffs)
- **Dexterity** - Có thể âm (penalties)
- **Vitality** - Có thể âm (health penalties)  
- **Energy** - Có thể âm (mana penalties)

### 2. Life/Mana/Stamina
- **Life** - Có thể âm (life penalties từ cursed items)
- **Mana** - Có thể âm (mana penalties)
- **Stamina** - Có thể âm (stamina penalties)

### 3. Resistances (Kháng nguyên tố)
- **Fire Resist** - Có thể âm (fire vulnerability)
- **Cold Resist** - Có thể âm (cold vulnerability) 
- **Lightning Resist** - Có thể âm (lightning vulnerability)
- **Poison Resist** - Có thể âm (poison vulnerability)
- **Magic Resist** - Có thể âm (magic vulnerability)

### 4. Enhanced Properties
- **Enhanced Defense** - Có thể âm (defense penalty)
- **Attack Rating** - Có thể âm (AR penalty)

## Properties KHÔNG thể âm

### 1. Damage Values (Sát thương)
- **Enhanced Damage** - Luôn >= 0
- **Minimum/Maximum Damage** - Luôn >= 0
- **Elemental Damage** (Fire, Cold, Lightning, Poison) - Luôn >= 0

### 2. Base Defense
- **Defense** - Luôn >= 0 (base defense value)

### 3. Durability & Quantities
- **Durability** - Luôn >= 0
- **Quantity** - Luôn >= 0

## Implementation Details

### Value Range Calculation
```cpp
// Trong PropertyEditor::getValueRange()
int maxValue = (1 << propTxt->bits) - 1 - propTxt->add;
int minValue = -propTxt->add;

// propTxt->add là offset value từ ItemPropertyTxt
// Nếu add = 32, thì có thể có giá trị từ -32 đến maxValue-32
```

### Validation Logic
```cpp
// Properties được phân loại:

// 1. Damage - chỉ cho phép >= 0
case MinimumDamage:
case MaximumDamage: 
    return QPair<int, int>(qMax(0, minValue), maxValue);

// 2. Stats/Resistances - cho phép giá trị âm
case Strength:
case Dexterity:
case Life:
    return QPair<int, int>(minValue, maxValue);

// 3. Special cases
case EnhancedDamage:
    return QPair<int, int>(0, 32767);  // Fixed range
```

## Ví dụ thực tế

### Curse Items (Vật phẩm bị nguyền)
```
-10 to Strength     // Giảm Strength 10 điểm
-25% Fire Resist    // Tăng sát thương lửa nhận vào 25%
-50 to Life         // Giảm HP tối đa 50 điểm
```

### Penalties (Hình phạt)
```
-5 to All Attributes    // Giảm tất cả stats 5 điểm
-20% to All Resistances // Giảm tất cả resistance 20%
-100 to Mana           // Giảm mana tối đa 100 điểm
```

### Vulnerabilities (Điểm yếu)
```
-50% Cold Resist       // Dễ bị sát thương lạnh
-30% Lightning Resist  // Dễ bị sát thương điện
```

## UI Behavior

### Spinbox Range
- **Positive-only properties**: Range 0 to max
- **Can-be-negative properties**: Range minValue to maxValue
- **Stats**: Thường -32 to +31 (nếu bits=6, add=32)
- **Resistances**: Thường có range tương tự

### Validation Warnings
```
Range: -32 to +31     // Cho stats với propTxt->add = 32
Range: 0-32767        // Cho Enhanced Damage  
Range: -50 to +75     // Cho resistance với bits khác
```

### Color Coding
- **Green**: Giá trị hợp lệ
- **Red warning**: Vượt quá range cho phép
- **Orange info**: Duplicate property

## Testing Guidelines

### Test Cases cần chạy:

1. **Negative Stats**
   ```cpp
   Strength = -10;        // Should work
   Dexterity = -5;        // Should work  
   Vitality = -15;        // Should work
   ```

2. **Negative Resistances**
   ```cpp
   FireResist = -25;      // Should work (vulnerability)
   ColdResist = -30;      // Should work  
   PoisonResist = -20;    // Should work
   ```

3. **Positive-only Properties**
   ```cpp
   EnhancedDamage = -10;  // Should be blocked
   MinimumDamage = -5;    // Should be blocked
   Defense = -20;         // Should be blocked
   ```

4. **Edge Cases**
   ```cpp
   Strength = -32;        // Min value (usually)
   Strength = -33;        // Should be blocked
   FireResist = 0;        // Neutral (should work)
   ```

## Bit Storage Format

### Negative Value Storage
```
Stored Value = Display Value + propTxt->add

Example: Strength -10 with add=32
Display: -10
Stored:  -10 + 32 = 22 (in bits)

When reading back:
Display = Stored - add = 22 - 32 = -10
```

### Range Calculation Logic
```cpp
// Max stored value: (1 << bits) - 1
// Max display value: maxStored - add  
// Min display value: 0 - add = -add

For 6-bit property with add=32:
- Max stored: 63
- Max display: 63 - 32 = 31
- Min display: 0 - 32 = -32
- Range: -32 to +31
```

## Safety Notes

### Before Testing
1. **Backup save files** - Negative values có thể affect gameplay
2. **Test với items không quan trọng** trước
3. **Verify in-game** - Đảm bảo game hiển thị đúng

### Common Mistakes
1. **Confusing display vs storage values** 
2. **Assuming all properties can be negative**
3. **Not checking propTxt->add offset**
4. **Forgetting bit limits still apply**

### Validation Flow
```
User Input → PropertyEditor validation → 
PropertyModificationEngine validation → 
Bit manipulation → Storage → Game display
```

Mỗi bước đều có validation riêng để đảm bảo tính toàn vẹn dữ liệu.

## Conclusion

Property Editor hiện tại đã hỗ trợ đầy đủ:
- ✅ Negative values cho stats và resistances
- ✅ Positive-only validation cho damage và defense  
- ✅ Correct bit range calculation với propTxt->add
- ✅ UI warnings rõ ràng về valid ranges
- ✅ Safe bit manipulation với negative values

Người dùng có thể tạo cursed items, penalty items, và vulnerability effects một cách an toàn!