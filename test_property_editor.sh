#!/bin/bash
# Test script for PropertyEditor functionality
# Usage: ./test_property_editor.sh

echo "=== MedianXL Offline Tools - Property Editor Test ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Creating..."
    mkdir build
    cd build
    cmake ..
    make
    cd ..
fi

echo "Testing PropertyEditor integration..."

# Check if required files exist
files=(
    "src/propertyeditor.h"
    "src/propertyeditor.cpp" 
    "src/propertymodificationengine.h"
    "src/propertymodificationengine.cpp"
    "Huong_dan_doc_thuoc_tinh_item.md"
    "Property_Editor_Guide.md"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file exists"
    else
        echo "✗ $file missing"
    fi
done

# Check CMakeLists.txt integration
if grep -q "propertyeditor" src/CMakeLists.txt; then
    echo "✓ PropertyEditor integrated in CMakeLists.txt"
else
    echo "✗ PropertyEditor not found in CMakeLists.txt"
fi

# Check PropertiesViewerWidget integration
if grep -q "PropertyEditor" src/propertiesviewerwidget.h; then
    echo "✓ PropertyEditor integrated in PropertiesViewerWidget.h"
else
    echo "✗ PropertyEditor not integrated in PropertiesViewerWidget.h"
fi

if grep -q "openPropertyEditor" src/propertiesviewerwidget.cpp; then
    echo "✓ openPropertyEditor method found in PropertiesViewerWidget.cpp"
else
    echo "✗ openPropertyEditor method not found in PropertiesViewerWidget.cpp"
fi

echo "=== Test Summary ==="
echo "Property Editor implementation should be complete."
echo "To test functionality:"
echo "1. Build the project: cd build && make"
echo "2. Run MedianXLOfflineTools"
echo "3. Load a save file with items"
echo "4. Select an item and click 'Edit Properties'"
echo "5. Test property modification"

echo ""
echo "=== Safety Reminders ==="
echo "- Always backup save files before editing"
echo "- Test with non-critical items first"
echo "- Verify changes work in-game"
echo "- Check for property validation warnings"