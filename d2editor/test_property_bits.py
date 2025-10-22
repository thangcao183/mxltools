#!/usr/bin/env python3

# Test property bit generation
propId = 79
value = 50
add = 100
bits = 9

rawValue = value + add  # 150

# MSB-first (correct for ItemParser)
propId_msb = format(propId, '09b')
value_msb = format(rawValue, f'0{bits}b')
end_marker_msb = format(511, '09b')

print("Property bits (MSB-first, should match Python tools):")
print(f"  Property ID 79: {propId_msb}")
print(f"  Value 150 (50+100): {value_msb}")
print(f"  End marker 511: {end_marker_msb}")
print(f"  Combined: {propId_msb + value_msb + end_marker_msb}")
print()

# Compare with output from property_adder_v3
v3_prop = "001001111010010110"
v3_end = "111111111"
v3_total = v3_prop + v3_end

print(f"From property_adder_v3 output:")
print(f"  Property bits: {v3_prop}")
print(f"  End marker: {v3_end}")
print(f"  Total: {v3_total}")
print()

expected = propId_msb + value_msb + end_marker_msb
print(f"Match: {v3_total == expected}")
if v3_total != expected:
    print(f"❌ MISMATCH!")
    print(f"Expected: {expected}")
    print(f"Got:      {v3_total}")
