#include "itemsviewerdialog.h"

// Minimal stubs to satisfy linker for CLI tools that use ItemParser
// These return safe defaults and avoid pulling in full GUI dependencies.

const QString &ItemsViewerDialog::tabNameAtIndex(int i)
{
    static QString names[] = { QString("Gear"), QString("Inventory"), QString("Cube"), QString("PersonalStash"), QString("SigmaSharedStash"), QString("SigmaHCStash"), QString("SharedStash"), QString("HCStash") };
    static const QString defaultName("Tab");
    if (i >= 0 && i < 8)
        return names[i];
    return defaultName;
}

int ItemsViewerDialog::tabIndexFromItemStorage(int storage)
{
    // Default mapping used by the GUI; for CLI tools we return PersonalStashIndex for safety
    return ItemsViewerDialog::PersonalStashIndex;
}
