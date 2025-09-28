# Property Editor Test PowerShell Script
# Usage: .\test_property_editor.ps1

Write-Host "=== MedianXL Offline Tools - Property Editor Test ===" -ForegroundColor Cyan

# Check if build directory exists
if (-Not (Test-Path "build")) {
    Write-Host "Build directory not found. Creating..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
    Set-Location build
    cmake ..
    cmake --build .
    Set-Location ..
}

Write-Host "Testing PropertyEditor integration..." -ForegroundColor Green

# Check if required files exist
$files = @(
    "src\propertyeditor.h",
    "src\propertyeditor.cpp", 
    "src\propertymodificationengine.h",
    "src\propertymodificationengine.cpp",
    "Huong_dan_doc_thuoc_tinh_item.md",
    "Property_Editor_Guide.md"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "✓ $file exists" -ForegroundColor Green
    } else {
        Write-Host "✗ $file missing" -ForegroundColor Red
    }
}

# Check CMakeLists.txt integration
$cmakeContent = Get-Content "src\CMakeLists.txt" -Raw
if ($cmakeContent -match "propertyeditor") {
    Write-Host "✓ PropertyEditor integrated in CMakeLists.txt" -ForegroundColor Green
} else {
    Write-Host "✗ PropertyEditor not found in CMakeLists.txt" -ForegroundColor Red
}

# Check PropertiesViewerWidget integration
$headerContent = Get-Content "src\propertiesviewerwidget.h" -Raw
if ($headerContent -match "PropertyEditor") {
    Write-Host "✓ PropertyEditor integrated in PropertiesViewerWidget.h" -ForegroundColor Green
} else {
    Write-Host "✗ PropertyEditor not integrated in PropertiesViewerWidget.h" -ForegroundColor Red
}

$cppContent = Get-Content "src\propertiesviewerwidget.cpp" -Raw
if ($cppContent -match "openPropertyEditor") {
    Write-Host "✓ openPropertyEditor method found in PropertiesViewerWidget.cpp" -ForegroundColor Green
} else {
    Write-Host "✗ openPropertyEditor method not found in PropertiesViewerWidget.cpp" -ForegroundColor Red
}

Write-Host ""
Write-Host "=== Test Summary ===" -ForegroundColor Cyan
Write-Host "Property Editor implementation should be complete." -ForegroundColor White
Write-Host "To test functionality:" -ForegroundColor Yellow
Write-Host "1. Build the project: cd build; cmake --build ." -ForegroundColor White
Write-Host "2. Run MedianXLOfflineTools.exe" -ForegroundColor White
Write-Host "3. Load a save file with items" -ForegroundColor White
Write-Host "4. Select an item and click 'Edit Properties'" -ForegroundColor White
Write-Host "5. Test property modification" -ForegroundColor White

Write-Host ""
Write-Host "=== Safety Reminders ===" -ForegroundColor Yellow
Write-Host "- Always backup save files before editing" -ForegroundColor Red
Write-Host "- Test with non-critical items first" -ForegroundColor Red
Write-Host "- Verify changes work in-game" -ForegroundColor Red  
Write-Host "- Check for property validation warnings" -ForegroundColor Red