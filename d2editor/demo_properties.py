#!/usr/bin/env python3
"""
Demo script showing property database capabilities
"""

import property_adder

print("=" * 70)
print("D2I PROPERTY ADDER - DATABASE DEMO")
print("=" * 70)

print(f"\n📊 Database Statistics:")
print(f"   Total properties loaded: {len(property_adder.PROPERTIES)}")
print(f"   Property ID range: {min(property_adder.PROPERTIES.keys())} - {max(property_adder.PROPERTIES.keys())}")

print("\n🔍 Search Examples:\n")

# Example 1: Find all attack-related properties
print("1️⃣  Properties with 'attack' in name:")
attack_props = [(pid, p) for pid, p in property_adder.PROPERTIES.items() if 'attack' in p['name'].lower()]
for pid, prop in sorted(attack_props[:5]):
    print(f"   ID {pid:3d}: {prop['name']:<35} (add={prop['add']}, bits={prop['bits']})")
if len(attack_props) > 5:
    print(f"   ... and {len(attack_props) - 5} more")

# Example 2: Find all resist properties
print("\n2️⃣  Resistance properties:")
resist_props = [(pid, p) for pid, p in property_adder.PROPERTIES.items() if 'resist' in p['name'].lower()]
for pid, prop in sorted(resist_props[:8]):
    print(f"   ID {pid:3d}: {prop['name']:<35} (add={prop['add']}, bits={prop['bits']})")

# Example 3: Properties with high add values
print("\n3️⃣  Properties with high base offset (add >= 500):")
high_add = [(pid, p) for pid, p in property_adder.PROPERTIES.items() if p['add'] >= 500]
for pid, prop in sorted(high_add, key=lambda x: x[1]['add'], reverse=True)[:5]:
    print(f"   ID {pid:3d}: {prop['name']:<35} (add={prop['add']}, bits={prop['bits']})")

# Example 4: Properties with most bits (can store largest values)
print("\n4️⃣  Properties with most bits (largest possible values):")
most_bits = sorted(property_adder.PROPERTIES.items(), key=lambda x: x[1]['bits'], reverse=True)[:5]
for pid, prop in most_bits:
    max_val = (2 ** prop['bits']) - 1
    print(f"   ID {pid:3d}: {prop['name']:<35} ({prop['bits']} bits, max value: {max_val})")

# Example 5: Skill-related properties
print("\n5️⃣  Skill-related properties:")
skill_props = [(pid, p) for pid, p in property_adder.PROPERTIES.items() if 'skill' in p['name'].lower()]
for pid, prop in sorted(skill_props[:10]):
    print(f"   ID {pid:3d}: {prop['name']:<35} (add={prop['add']}, bits={prop['bits']})")

# Example 6: Damage properties
print("\n6️⃣  Damage properties:")
damage_props = [(pid, p) for pid, p in property_adder.PROPERTIES.items() if 'damage' in p['name'].lower()]
for pid, prop in sorted(damage_props[:8]):
    print(f"   ID {pid:3d}: {prop['name']:<35} (add={prop['add']}, bits={prop['bits']})")

# Example 7: Item-specific properties (item_*)
print("\n7️⃣  Item properties (item_*):")
item_props = [(pid, p) for pid, p in property_adder.PROPERTIES.items() if p['name'].startswith('item_')]
print(f"   Total item properties: {len(item_props)}")
for pid, prop in sorted(item_props[:10]):
    print(f"   ID {pid:3d}: {prop['name']:<35} (add={prop['add']}, bits={prop['bits']})")
if len(item_props) > 10:
    print(f"   ... and {len(item_props) - 10} more")

print("\n" + "=" * 70)
print("💡 Usage Examples:")
print("=" * 70)

print("\n# Add Gold Find + Magic Find:")
print("python3 property_adder.py file.d2i 79 50 80 30")

print("\n# Add All Resistances +20:")
print("python3 property_adder.py file.d2i 39 20 41 20 43 20 45 20")

print("\n# Add Stats +10:")
print("python3 property_adder.py file.d2i 0 10 1 10 2 10 3 10")

print("\n# Add Life +20 and All Skills +1:")
print("python3 property_adder.py file.d2i 7 20 127 1")

print("\n# Search for specific property:")
print("python3 property_adder.py --list-all | grep -i 'cast'")

print("\n" + "=" * 70)
print(f"✨ {len(property_adder.PROPERTIES)} properties available!")
print("Use: python3 property_adder.py --list-all")
print("=" * 70 + "\n")
