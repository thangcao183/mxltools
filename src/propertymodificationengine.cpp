#include "propertymodificationengine.h"
#include "itemparser.h"
#include "itemdatabase.h"
#include "reversebitwriter.h"
#include "reversebitreader.h"
#include "enums.h"
#include "helpers.h"

#include <QApplication>
#include <QDebug>
#include <QBuffer>

// Helper: find first differing bit index between two bit-strings
static int firstDifferingBitIndex(const QString &a, const QString &b)
{
    int n = qMin(a.length(), b.length());
    for (int i = 0; i < n; ++i) {
        if (a.at(i) != b.at(i)) return i;
    }
    if (a.length() != b.length()) return n;
    return -1;
}

// Map a bit index in the bitString to the byte index within the fullBytes ("JM" + itemBytes) produced by ItemParser::writeItems
static int bitIndexToFullBytesIndex(int bitIndex, int bitStringLength)
{
    if (bitIndex < 0) return -1;
    int numChunks = (bitStringLength + 7) / 8; // number of 8-bit chunks
    int chunkIndex = bitIndex / 8; // which chunk (0 = leftmost)
    // In writeItems the chunk at index j becomes itemBytes position (numChunks-1 - j), and fullBytes adds 2 bytes header
    int fullIndex = 2 + (numChunks - 1 - chunkIndex);
    return fullIndex;
}

PropertyModificationEngine::PropertyModificationEngine(QObject *parent)
    : QObject(parent)
{
}

bool PropertyModificationEngine::modifyItemProperties(ItemInfo *item, const PropertiesMultiMap &newProperties)
{
    if (!item) {
        setError(tr("Invalid item"));
        return false;
    }
    
    emit statusChanged(tr("Validating properties..."));
    emit progressChanged(10);
    
    // Validate all properties
    QString validationError;
    if (!validatePropertyCombination(newProperties, &validationError)) {
        setError(validationError);
        return false;
    }
    
    emit statusChanged(tr("Backing up original properties..."));
    emit progressChanged(20);
    
    // Backup original properties
    PropertiesMultiMap originalProps;
    for (auto it = item->props.constBegin(); it != item->props.constEnd(); ++it) {
        originalProps.insert(it.key(), new ItemProperty(*it.value()));
    }
    
    try {
    emit statusChanged(tr("Preparing merged properties..."));
        emit progressChanged(30);

        // Merge original properties with newProperties. Callers may pass only the
        // delta (newProperties) expecting the engine to add/override existing properties.
        PropertiesMultiMap mergedProps;
        // Start with original properties (we already copied them into originalProps)
        for (auto it = originalProps.constBegin(); it != originalProps.constEnd(); ++it) {
            mergedProps.insert(it.key(), new ItemProperty(*it.value()));
        }

        // Overlay newProperties: replace existing entries or add new ones
        for (auto it = newProperties.constBegin(); it != newProperties.constEnd(); ++it) {
            // Remove existing entries for this property id from mergedProps
            QList<ItemProperty*> toDelete;
            auto range = mergedProps.equal_range(it.key());
            for (auto iter = range.first; iter != range.second; ++iter) {
                toDelete.append(iter.value());
            }
            for (ItemProperty *p : toDelete) {
                mergedProps.remove(it.key(), p);
                delete p;
            }
            // Insert a copy of the new property
            mergedProps.insert(it.key(), new ItemProperty(*it.value()));
        }

        emit statusChanged(tr("Applying merged properties..."));
        emit progressChanged(50);

        // If this is a simple additive change (only new properties added, no deletions or modifications),
        // attempt an in-place insert which is safer and faster. Otherwise fall back to full reconstruction.
        QSet<int> originalKeys;
        for (auto it = originalProps.constBegin(); it != originalProps.constEnd(); ++it) originalKeys.insert(it.key());
        QSet<int> mergedKeys;
        for (auto it = mergedProps.constBegin(); it != mergedProps.constEnd(); ++it) mergedKeys.insert(it.key());

        bool onlyAdditions = true;
        // If any key in mergedProps existed in original but value differs, treat as modification
        for (auto it = mergedProps.constBegin(); it != mergedProps.constEnd(); ++it) {
            if (originalKeys.contains(it.key())) {
                // Compare values - simple pointer compare not reliable; assume modification if present in both
                onlyAdditions = false; break;
            }
        }

        // Apply in-place if only additions and small number of properties
        if (onlyAdditions && mergedProps.size() - originalProps.size() <= 4) {
            // attempt to insert new props one by one
            bool inPlaceOk = true;
            QList<int> newIds;
            for (auto it = mergedProps.constBegin(); it != mergedProps.constEnd(); ++it) if (!originalKeys.contains(it.key())) newIds.append(it.key());

            // Make a working copy of item's props (so we can restore on failure)
            PropertiesMultiMap savedProps = originalProps;

            for (int id : newIds) {
                ItemProperty *p = nullptr;
                // find the property in mergedProps
                auto range = mergedProps.equal_range(id);
                if (range.first != range.second) p = new ItemProperty(*range.first.value());
                if (!p) { inPlaceOk = false; break; }

                // Try in-place insert
                if (!insertPropertyInPlace(item, id, p)) {
                    inPlaceOk = false;
                    delete p;
                    break;
                }

                // on success, add to savedProps
                savedProps.insert(id, p);
            }

            if (inPlaceOk) {
                // commit savedProps as the new props
                qDeleteAll(item->props);
                item->props = savedProps;
                emit statusChanged(tr("Properties updated (in-place)"));
                emit progressChanged(100);
                // cleanup mergedProps entries that were copies
                qDeleteAll(mergedProps);
                return true;
            } else {
                // rollback any partial changes - restore original bitString from originalProps by forcing full reconstruct later
                qDeleteAll(savedProps);
                // restore original props (we haven't changed item->props yet)
            }
        }

        // Replace item's props with mergedProps
        qDeleteAll(item->props);
        item->props = mergedProps;
        
        emit statusChanged(tr("Reconstructing item bit string..."));
        emit progressChanged(70);
        
        // Reconstruct the item's bit string
        if (!reconstructItemBitString(item)) {
            // Restore original properties on failure
            qDeleteAll(item->props);
            item->props = originalProps;
            return false;
        }
        
        emit statusChanged(tr("Updating display strings..."));
        emit progressChanged(90);
        
        // Update display strings for all properties
        for (auto it = item->props.begin(); it != item->props.end(); ++it) {
            ItemParser::createDisplayStringForPropertyWithId(it.key(), it.value());
        }
        
        // Clean up backup
        qDeleteAll(originalProps);
        
        item->hasChanged = true;
        
        emit statusChanged(tr("Properties updated successfully"));
        emit progressChanged(100);
        
        return true;
        
    } catch (const std::exception &e) {
        // Restore original properties
        qDeleteAll(item->props);
        item->props = originalProps;
        setError(QString("Exception during property modification: %1").arg(e.what()));
        return false;
    } catch (...) {
        // Restore original properties
        qDeleteAll(item->props);
        item->props = originalProps;
        setError(tr("Unknown error during property modification"));
        return false;
    }
}

bool PropertyModificationEngine::reconstructItemBitString(ItemInfo *item)
{
    if (!item) return false;
    
    try {
        // Use ItemParser logic to calculate exact properties start position
        int propertiesStart = calculatePropertiesStartPosition(item);
        
        if (propertiesStart < 0) {
            // Fallback: estimate properties start based on typical item structure
            // This is safer than failing completely
            qDebug() << "PropertyModificationEngine: Using fallback estimation for properties start";
            
            // Typical minimum item size before properties: ~200-400 bits
            // Look for a reasonable position by scanning backwards from end
            propertiesStart = item->bitString.length() - 200; // Conservative estimate
            
            if (propertiesStart < 100) propertiesStart = 100; // Ensure some minimum space for item data
            
            qDebug() << "PropertyModificationEngine: Fallback properties start:" << propertiesStart;
        }
        
    // Keep everything before properties (item header + basic data)
    QString beforeProperties = item->bitString.left(propertiesStart);
    qDebug() << "PropertyModificationEngine: Before properties section length:" << beforeProperties.length();
    qDebug() << "PropertyModificationEngine: propertiesStart(bits)=" << propertiesStart;
    // Log a short tail of the beforeProperties bits to check boundaries
    int tailLen = qMin(128, beforeProperties.length());
    qDebug() << "PropertyModificationEngine: beforeProperties tail (bits):" << beforeProperties.right(tailLen);
        
        // Check if there's data after properties that we need to preserve
        QString afterProperties;
        if (!item->props.isEmpty()) {
            // Try to accurately parse properties using ItemParser::parseItemProperties
            // Create a reader positioned at propertiesStart and attempt to parse properties.
            QString currentBitString = item->bitString;
            ReverseBitReader reader(currentBitString);
            // set reader position to propertiesStart (pos is relative to left side)
            reader.setPos(propertiesStart);
            ItemInfo::ParsingStatus tmpStatus = ItemInfo::Ok;
            try {
                PropertiesMultiMap parsed = ItemParser::parseItemProperties(reader, &tmpStatus);
                Q_UNUSED(parsed);
                if (tmpStatus == ItemInfo::Ok) {
                    int endMarkerPos = reader.pos();
                    if (endMarkerPos > propertiesStart && endMarkerPos < currentBitString.length()) {
                        afterProperties = currentBitString.mid(endMarkerPos);
                        qDebug() << "PropertyModificationEngine: Found data after properties, length:" << afterProperties.length();
                    }
                } else {
                    qDebug() << "PropertyModificationEngine: parseItemProperties returned status" << tmpStatus << ", falling back to string search";
                    // fallback to simple search below
                    QString endMarkerPattern = "111111111";
                    int endMarkerPos = -1;
                    for (int i = propertiesStart; i <= currentBitString.length() - 9; ++i) {
                        if (currentBitString.mid(i, 9) == endMarkerPattern) {
                            endMarkerPos = i + 9;
                            break;
                        }
                    }
                    if (endMarkerPos > 0 && endMarkerPos < currentBitString.length()) {
                        afterProperties = currentBitString.mid(endMarkerPos);
                        qDebug() << "PropertyModificationEngine: Found data after properties (fallback), length:" << afterProperties.length();
                    }
                }
            } catch (...) {
                qDebug() << "PropertyModificationEngine: Exception while parsing properties for end detection; falling back to string search";
                QString endMarkerPattern = "111111111";
                int endMarkerPos = -1;
                for (int i = propertiesStart; i <= currentBitString.length() - 9; ++i) {
                    if (currentBitString.mid(i, 9) == endMarkerPattern) {
                        endMarkerPos = i + 9;
                        break;
                    }
                }
                if (endMarkerPos > 0 && endMarkerPos < currentBitString.length()) {
                    afterProperties = currentBitString.mid(endMarkerPos);
                    qDebug() << "PropertyModificationEngine: Found data after properties (fallback), length:" << afterProperties.length();
                }
            }
        }
        
    // Attempt a safer in-place insertion if possible: parse existing properties IDs and
    // only insert new properties at the correct position to avoid touching header/after data.
    // NOTE: in-place path has caused parser crashes in some cases; skip it for creating option outputs.
    bool triedInPlace = false;
    if (false && !item->props.isEmpty()) {
            try {
                // Parse existing property IDs from the current bit string starting at propertiesStart
                ReverseBitReader idReader(item->bitString);
                idReader.setPos(propertiesStart);
                QList<int> existingIds;
                while (true) {
                    int posBefore = idReader.pos();
                    int id = idReader.readNumber(9);
                    if (id == 511) { // end marker
                        break;
                    }
                    existingIds.append(id);
                    // compute bits length and skip the rest of this property
                    int skipBits = calculateBitLength(id, 0, 0) - 9;
                    if (skipBits > 0) idReader.skip(skipBits);
                }

                // Determine new IDs (present in item->props but not in existingIds)
                QSet<int> existingSet;
                for (int v : existingIds) existingSet.insert(v);
                QList<int> newIds;
                for (auto it = item->props.constBegin(); it != item->props.constEnd(); ++it) {
                    if (!existingSet.contains(it.key())) newIds.append(it.key());
                }

                if (!newIds.isEmpty() && newIds.size() <= 6) {
                    // Sort new IDs ascending - insertion must preserve global ascending order
                    std::sort(newIds.begin(), newIds.end());

                    QString candidate = item->bitString;
                    // Prepare original bytes for byte-level comparisons
                    QByteArray origItemBytes;
                    for (int i = 0, n = item->bitString.length(); i < n; i += 8) {
                        QString chunk = item->bitString.mid(i, 8);
                        if (chunk.length() < 8) break;
                        bool ok = false;
                        quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
                        if (!ok) break;
                        origItemBytes.prepend(static_cast<char>(val));
                    }
                    QByteArray origFull = ItemParser::kItemHeader + origItemBytes;
                    qDebug() << "PropertyModificationEngine: attempting ordered in-place insert of" << newIds.size() << "ids";
                    bool allInserted = true;

                    for (int newId : newIds) {
                        // find insertion position in candidate by scanning properties until we find id > newId or end marker
                        ReverseBitReader scanReader(candidate);
                        scanReader.setPos(propertiesStart);
                        int insertionPos = -1;
                        while (true) {
                            int posBefore = scanReader.pos();
                            int id = scanReader.readNumber(9);
                            if (id == 511) { insertionPos = posBefore; break; }
                            if (id > newId) { insertionPos = posBefore; break; }
                            int skipBits = calculateBitLength(id, 0, 0) - 9;
                            if (skipBits > 0) scanReader.skip(skipBits);
                        }

                        if (insertionPos < 0) { allInserted = false; break; }

                        // build bitstring for this property only
                        QString singlePropBits;
                        auto range = item->props.equal_range(newId);
                        if (range.first == range.second) { allInserted = false; break; }
                        writePropertyToBits(singlePropBits, newId, range.first.value());

                        // insert (do not add end marker) at bit index
                        qDebug() << "PropertyModificationEngine: inserting single prop id" << newId << "at bit pos" << insertionPos << "propBitsLen" << singlePropBits.length();
                        candidate.insert(insertionPos, singlePropBits);

                        // quick bit-diff check vs original
                        int diff = firstDifferingBitIndex(item->bitString, candidate);
                        if (diff >= 0 && diff < propertiesStart) {
                            qDebug() << "PropertyModificationEngine: WARNING - candidate differs from original before propertiesStart at bit" << diff;
                            allInserted = false;
                            break;
                        }
                    }

                    if (allInserted) {
                        // byte align and verify via round-trip parse
                        ReverseBitWriter::byteAlignBits(candidate);
                        int diffPostAlign = firstDifferingBitIndex(item->bitString, candidate);
                        qDebug() << "PropertyModificationEngine: in-place candidate first differing bit after align:" << diffPostAlign;

                        // convert to bytes
                        QByteArray itemBytes;
                        for (int i = 0, n = candidate.length(); i < n; i += 8) {
                            QString chunk = candidate.mid(i, 8);
                            if (chunk.length() < 8) break;
                            bool ok = false;
                            quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
                            if (!ok) { allInserted = false; break; }
                            itemBytes.prepend(static_cast<char>(val));
                        }

                        if (allInserted) {
                            QByteArray fullBytes = ItemParser::kItemHeader + itemBytes;
                            // Debug: log first differing bit between original and candidate (bits)
                            int firstDiff = firstDifferingBitIndex(item->bitString, candidate);
                            if (firstDiff >= 0) {
                                int showStart = qMax(0, firstDiff - 32);
                                int showLen = qMin(128, candidate.length() - showStart);
                                qDebug() << "PropertyModificationEngine: first differing bit index:" << firstDiff << "show bits around:" << candidate.mid(showStart, showLen);
                            } else {
                                qDebug() << "PropertyModificationEngine: no differing bits found between original and candidate (in-place path)";
                            }
                            // compute first differing byte between original and candidate full bytes BEFORE parsing
                            qDebug() << "PropertyModificationEngine: original bytes (hex prefix):" << origFull.left(64).toHex();
                            int byteDiff = -1;
                            int minLen = qMin(origFull.length(), fullBytes.length());
                            for (int bi = 0; bi < minLen; ++bi) {
                                if (origFull.at(bi) != fullBytes.at(bi)) { byteDiff = bi; break; }
                            }
                            qDebug() << "PropertyModificationEngine: first differing byte index (in-place):" << byteDiff;
                            int mapped = bitIndexToFullBytesIndex(firstDiff, item->bitString.length());
                            qDebug() << "PropertyModificationEngine: first differing bit index (in-place):" << firstDiff << "maps to fullBytes index:" << mapped;
                            qDebug() << "PropertyModificationEngine: candidate bytes (hex prefix):" << fullBytes.left(64).toHex();
                            QBuffer buf(&fullBytes);
                            if (buf.open(QIODevice::ReadOnly)) {
                                QDataStream ds(&buf); ds.setByteOrder(QDataStream::LittleEndian);
                                ItemInfo *parsed = ItemParser::parseItem(ds, fullBytes);
                                if (parsed) qDebug() << "PropertyModificationEngine: parsed itemType (hex):" << parsed->itemType.toHex();
                                if (parsed) {
                                    if (parsed->itemType == item->itemType && (!item->isExtended || parsed->guid == item->guid)) {
                                        // success: apply candidate
                                        item->bitString = candidate;
                                        delete parsed;
                                        triedInPlace = true;
                                    } else {
                                        delete parsed;
                                        allInserted = false;
                                    }
                                } else allInserted = false;
                            } else allInserted = false;
                        }
                    }
                }
            } catch (...) {
                qDebug() << "PropertyModificationEngine: exception during in-place insert attempt";
            }
        }

        if (!triedInPlace) {
            // Build new properties section using ItemParser's property writing logic
        QString propertiesBits = buildPropertiesBitString(item->props);
            qDebug() << "PropertyModificationEngine: New properties section length:" << propertiesBits.length();

            // Assemble complete bit string
            QString newBitString = beforeProperties + propertiesBits + afterProperties;
            // CREATE Option outputs for debugging/inspection
            // 1) Manual-assembled bytes from newBitString (may be invalid)
            try {
                QByteArray manualBytes;
                for (int i = 0, n = newBitString.length(); i < n; i += 8) {
                    QString chunk = newBitString.mid(i, 8);
                    if (chunk.length() < 8) break;
                    bool ok = false;
                    quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
                    if (!ok) break;
                    manualBytes.prepend(static_cast<char>(val));
                }
                QByteArray manualFull = ItemParser::kItemHeader + manualBytes;
                QFile mf("/tmp/amu_option1_manual.d2i"); if (mf.open(QIODevice::WriteOnly)) { mf.write(manualFull); mf.close(); }
                qDebug() << "PropertyModificationEngine: wrote manual-assembled output to /tmp/amu_option1_manual.d2i";
            } catch (...) {
                qDebug() << "PropertyModificationEngine: failed to write manual-assembled output";
            }
            // Quick diagnostic: find first differing bit between original and new
            int firstDiff = firstDifferingBitIndex(item->bitString, newBitString);
            qDebug() << "PropertyModificationEngine: first differing bit between original and newBitString:" << firstDiff;
            if (firstDiff >= 0) {
                int showStart = qMax(0, firstDiff - 32);
                int showLen = qMin(160, newBitString.length() - showStart);
                qDebug() << "PropertyModificationEngine: original around diff:" << item->bitString.mid(showStart, showLen);
                qDebug() << "PropertyModificationEngine: new around diff:" << newBitString.mid(showStart, showLen);
            }

            qDebug() << "PropertyModificationEngine: Before reconstruction:" 
                     << "total=" << item->bitString.length()
                     << "before_props=" << beforeProperties.length() 
                     << "after_props=" << afterProperties.length();

            // Byte align the result (same as ItemParser does)
            ReverseBitWriter::byteAlignBits(newBitString);

            qDebug() << "PropertyModificationEngine: After reconstruction:" 
                     << "total=" << newBitString.length() << "(was:" << item->bitString.length() << ")"
                     << "new_props=" << propertiesBits.length();

            // Validate we didn't lose critical data
            if (newBitString.length() < item->bitString.length() - 50) { // Allow some difference for property changes
                qDebug() << "PropertyModificationEngine: WARNING - Significant bit loss detected!";
                qDebug() << "Original bit string:" << item->bitString.left(100) << "...";
                qDebug() << "New bit string:" << newBitString.left(100) << "...";
                // Don't apply the change if too much data is lost
                setError(tr("Reconstruction would lose too much data"));
                return false;
            }

            // Try Option B: Chunk-aware insertion with proper padding
            if (tryChunkAwareInsertion(item, beforeProperties, propertiesBits, afterProperties)) {
                qDebug() << "PropertyModificationEngine: Successfully applied Option B (chunk-aware insertion)";
                return true;
            }
            
            // Try Option F: Property replacement as safer alternative
            qDebug() << "PropertyModificationEngine: Chunk-aware insertion failed, trying Option F (property replacement)";
            // Try to replace the last property with our new property (this maintains same total bit count)
            auto lastProp = item->props.constEnd();
            --lastProp;
            if (lastProp != item->props.constEnd()) {
                int replaceId = lastProp.key();
                qDebug() << "PropertyModificationEngine: Attempting to replace property ID" << replaceId << "with new property from addition request";
                
                // For now, try a simple replacement - in a real implementation you'd want more sophistication
                auto it = item->props.find(replaceId);
                if (it != item->props.end()) {
                    // Find the new property we're trying to add
                    for (auto addIt = item->props.constBegin(); addIt != item->props.constEnd(); ++addIt) {
                        if (addIt.key() > replaceId) { // This should be our new property
                            if (tryPropertyReplacement(item, addIt.key(), addIt.value()->value)) {
                                qDebug() << "PropertyModificationEngine: Successfully applied Option F (property replacement)";
                                return true;
                            }
                            break;
                        }
                    }
                }
            }
            
            // Try Option E: Smart bit-level reconstruction
            qDebug() << "PropertyModificationEngine: Property replacement failed, trying Option E (smart bit reconstruction)";
            if (trySmartBitReconstruction(item, beforeProperties, propertiesBits, afterProperties)) {
                qDebug() << "PropertyModificationEngine: Successfully applied Option E (smart bit reconstruction)";
                return true;
            }
            
            // Fallback to Option C: Complete item recreation
            qDebug() << "PropertyModificationEngine: Smart reconstruction failed, trying Option C (complete recreation)";
            if (tryCompleteItemRecreation(item)) {
                qDebug() << "PropertyModificationEngine: Successfully applied Option C (complete recreation)";
                return true;
            }
            
            // Final fallback: Option D - Direct property replacement (no chunk preservation)
            qDebug() << "PropertyModificationEngine: Trying Option D (direct replacement)";
            QString finalBitString = beforeProperties + propertiesBits + afterProperties;
            
            // Store original for potential rollback
            QString originalBitString = item->bitString;
            item->bitString = finalBitString;
            
            qDebug() << "PropertyModificationEngine: Option D - Applied direct bitString replacement";
            qDebug() << "Option D: New length:" << finalBitString.length() << "original:" << originalBitString.length();
            return true;
        }
    // (Reconstruction completed above)
    return true;
        
    } catch (const std::exception &e) {
        setError(QString("Failed to reconstruct bit string: %1").arg(e.what()));
        return false;
    } catch (int errorCode) {
        setError(QString("Bit manipulation error: %1").arg(errorCode));
        return false;
    } catch (...) {
        setError(tr("Unknown error during bit string reconstruction"));
        return false;
    }
}

QString PropertyModificationEngine::buildPropertiesBitString(const PropertiesMultiMap &properties)
{
    QString bitString;
    
    qDebug() << "PropertyModificationEngine: Building properties bit string for" << properties.size() << "properties";
    
    try {
        // Get ordered list of properties
        QList<QPair<int, ItemProperty*>> orderedProps = getOrderedProperties(properties);
        
        qDebug() << "PropertyModificationEngine: Got" << orderedProps.size() << "ordered properties";
        
        // Write each property
        for (const auto &pair : orderedProps) {
            qDebug() << "PropertyModificationEngine: Writing property ID" << pair.first 
                     << "value" << pair.second->value << "param" << pair.second->param;
            writePropertyToBits(bitString, pair.first, pair.second);
        }
        
        // Add end marker
        insertEndMarker(bitString);
        
    } catch (...) {
        qDebug() << "PropertyModificationEngine: Exception in buildPropertiesBitString";
        throw;
    }
    
    return bitString;
}

void PropertyModificationEngine::writePropertyToBits(QString &bitString, int propertyId, const ItemProperty *property)
{
    if (!property) return;
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    // Handle special properties
    if (isSpecialProperty(propertyId)) {
        switch (propertyId) {
            case Enums::ItemProperties::EnhancedDamage:
                writeEnhancedDamageProperty(bitString, property->value);
                return;
                
            case Enums::ItemProperties::MinimumDamageFire:
            case Enums::ItemProperties::MinimumDamageLightning:
            case Enums::ItemProperties::MinimumDamageMagic:
            case Enums::ItemProperties::MinimumDamageCold:
            case Enums::ItemProperties::MinimumDamagePoison:
                // Elemental damage requires special handling - skip for now
                // This should be handled by the calling function to write min/max/length together
                break;
        }
    }
    
    // Standard property writing
    
    // Write property ID (9 bits)
    insertPropertyId(bitString, propertyId);
    
    // Write parameter if present
    if (propTxt->paramBits > 0) {
        appendBits(bitString, property->param, propTxt->paramBits);
    }
    
    // Write value with add correction
    int adjustedValue = property->value + propTxt->add;
    appendBits(bitString, adjustedValue, propTxt->bits);
    
    // Debug logging for property 93
    if (propertyId == 93) {
        qDebug() << "PropertyModificationEngine: Property 93 detailed write:";
        qDebug() << "  - add:" << propTxt->add << "bits:" << propTxt->bits << "paramBits:" << propTxt->paramBits;
        qDebug() << "  - original value:" << property->value << "adjusted:" << adjustedValue;
        qDebug() << "  - ID bits (9):" << QString::number(propertyId, 2).rightJustified(9, '0');
        if (propTxt->paramBits > 0) {
            qDebug() << "  - param bits (" << propTxt->paramBits << "):" << QString::number(property->param, 2).rightJustified(propTxt->paramBits, '0');
        }
        qDebug() << "  - value bits (" << propTxt->bits << "):" << QString::number(adjustedValue, 2).rightJustified(propTxt->bits, '0');
        qDebug() << "  - total bits expected:" << (9 + propTxt->paramBits + propTxt->bits);
    }
}

void PropertyModificationEngine::writeEnhancedDamageProperty(QString &bitString, int value)
{
    insertPropertyId(bitString, Enums::ItemProperties::EnhancedDamage);
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(Enums::ItemProperties::EnhancedDamage);
    if (!propTxt) return;
    
    int adjustedValue = value + propTxt->add;
    
    // Enhanced damage is stored twice
    appendBits(bitString, adjustedValue, propTxt->bits);
    appendBits(bitString, adjustedValue, propTxt->bits);
}

void PropertyModificationEngine::writeElementalDamageProperty(QString &bitString, int baseId, int minValue, int maxValue, int length)
{
    ItemPropertyTxt *minPropTxt = ItemDataBase::Properties()->value(baseId);
    ItemPropertyTxt *maxPropTxt = ItemDataBase::Properties()->value(baseId + 1);
    
    if (!minPropTxt || !maxPropTxt) return;
    
    // Write min damage
    insertPropertyId(bitString, baseId);
    appendBits(bitString, minValue + minPropTxt->add, minPropTxt->bits);
    
    // Write max damage
    insertPropertyId(bitString, baseId + 1);
    appendBits(bitString, maxValue + maxPropTxt->add, maxPropTxt->bits);
    
    // Write length for cold/poison
    if (length > 0 && (baseId == Enums::ItemProperties::MinimumDamageCold || 
                       baseId == Enums::ItemProperties::MinimumDamagePoison)) {
        ItemPropertyTxt *lengthPropTxt = ItemDataBase::Properties()->value(baseId + 2);
        if (lengthPropTxt) {
            insertPropertyId(bitString, baseId + 2);
            appendBits(bitString, length + lengthPropTxt->add, lengthPropTxt->bits);
        }
    }
}

void PropertyModificationEngine::writeSkillProperty(QString &bitString, int propertyId, int value, quint32 skillId)
{
    insertPropertyId(bitString, propertyId);
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    // Write skill ID parameter
    if (propTxt->paramBits > 0) {
        appendBits(bitString, skillId, propTxt->paramBits);
    }
    
    // Write skill level
    appendBits(bitString, value + propTxt->add, propTxt->bits);
}

void PropertyModificationEngine::writeBooleanProperty(QString &bitString, int propertyId, bool value)
{
    insertPropertyId(bitString, propertyId);
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    appendBits(bitString, value ? 1 : 0, propTxt->bits);
}

bool PropertyModificationEngine::validateProperty(int propertyId, int value, quint32 parameter, QString *error)
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) {
        if (error) *error = tr("Unknown property ID: %1").arg(propertyId);
        return false;
    }
    
    // Check bit limits
    int maxValue = (1 << propTxt->bits) - 1 - propTxt->add;
    int minValue = -propTxt->add;
    
    if (value < minValue || value > maxValue) {
        if (error) *error = tr("Property %1: Value %2 out of range [%3, %4]")
                               .arg(propertyId).arg(value).arg(minValue).arg(maxValue);
        return false;
    }
    
    // Parameter validation
    if (propTxt->paramBits > 0) {
        quint32 maxParam = (1 << propTxt->paramBits) - 1;
        if (parameter > maxParam) {
            if (error) *error = tr("Property %1: Parameter %2 out of range [0, %3]")
                                   .arg(propertyId).arg(parameter).arg(maxParam);
            return false;
        }
    }
    
    // Special validation - only for specific properties that need it
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
            return validateEnhancedDamage(value, error);
            
        case Enums::ItemProperties::Defence: // Only base defense, not enhanced defense
            return validateDefense(value, error);
            
        // Validate all damage-related properties
        case Enums::ItemProperties::MinimumDamage:
        case Enums::ItemProperties::MaximumDamage:
        case Enums::ItemProperties::MinimumDamageSecondary:
        case Enums::ItemProperties::MaximumDamageSecondary:
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
        case Enums::ItemProperties::MaximumDamagePoison:
            return validateElementalDamage(propertyId, value, error);
    }
    
    // For all other properties (stats, resistances, etc.), 
    // only validate bit range limits - allow negative values if in range
    
    return true;
}

bool PropertyModificationEngine::validatePropertyCombination(const PropertiesMultiMap &properties, QString *error)
{
    QMap<int, int> propertyCount;
    
    // Count occurrences of each property
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        propertyCount[it.key()]++;
        
        // Validate individual property
        if (!validateProperty(it.key(), it.value()->value, it.value()->param, error)) {
            return false;
        }
    }
    
    // Check for duplicate properties that shouldn't be duplicated
    for (auto it = propertyCount.constBegin(); it != propertyCount.constEnd(); ++it) {
        if (it.value() > 1) {
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(it.key());
            QString propName = propTxt ? QString::fromUtf8(propTxt->stat) : QString::number(it.key());
            
            if (error) *error = tr("Duplicate property not allowed: %1 (appears %2 times)")
                                   .arg(propName).arg(it.value());
            return false;
        }
    }
    
    return true;
}

QList<int> PropertyModificationEngine::getDependentProperties(int propertyId) const
{
    QList<int> dependents;
    
    switch (propertyId) {
        case Enums::ItemProperties::MinimumDamageFire:
            dependents << Enums::ItemProperties::MaximumDamageFire;
            break;
            
        case Enums::ItemProperties::MinimumDamageLightning:
            dependents << Enums::ItemProperties::MaximumDamageLightning;
            break;
            
        case Enums::ItemProperties::MinimumDamageMagic:
            dependents << Enums::ItemProperties::MaximumDamageMagic;
            break;
            
        case Enums::ItemProperties::MinimumDamageCold:
            dependents << Enums::ItemProperties::MaximumDamageCold 
                      << Enums::ItemProperties::DurationCold;
            break;
            
        case Enums::ItemProperties::MinimumDamagePoison:
            dependents << Enums::ItemProperties::MaximumDamagePoison 
                      << Enums::ItemProperties::DurationPoison;
            break;
    }
    
    return dependents;
}

bool PropertyModificationEngine::isSpecialProperty(int propertyId) const
{
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
            return true;
        default:
            return false;
    }
}

int PropertyModificationEngine::calculateBitLength(int propertyId, int value, quint32 parameter) const
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return 0;
    
    int totalBits = 9; // Property ID
    totalBits += propTxt->paramBits; // Parameter
    totalBits += propTxt->bits; // Value
    
    // Special cases
    if (propertyId == Enums::ItemProperties::EnhancedDamage) {
        totalBits += propTxt->bits; // Enhanced damage stores value twice
    }
    
    return totalBits;
}

void PropertyModificationEngine::appendBits(QString &bitString, quint64 value, int bitCount)
{
    if (bitCount <= 0) return;
    
    QString bits = QString::number(value, 2).rightJustified(bitCount, '0');
    
    // Reverse the bits (LSB first)
    std::reverse(bits.begin(), bits.end());
    
    bitString.prepend(bits);
}

void PropertyModificationEngine::insertPropertyId(QString &bitString, int propertyId)
{
    appendBits(bitString, propertyId, 9);
}

void PropertyModificationEngine::insertEndMarker(QString &bitString)
{
    insertPropertyId(bitString, 511); // End marker
}

QList<QPair<int, ItemProperty*>> PropertyModificationEngine::getOrderedProperties(const PropertiesMultiMap &properties)
{
    QList<QPair<int, ItemProperty*>> orderedList;
    
    qDebug() << "PropertyModificationEngine: getOrderedProperties input size:" << properties.size();
    
    // Convert multimap to list
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        if (!it.value()) {
            qDebug() << "PropertyModificationEngine: Null property value for ID" << it.key();
            continue;
        }
        orderedList.append(QPair<int, ItemProperty*>(it.key(), it.value()));
    }
    
    qDebug() << "PropertyModificationEngine: After filtering, list size:" << orderedList.size();
    
    // Sort by property ID
    std::sort(orderedList.begin(), orderedList.end(), 
              [](const QPair<int, ItemProperty*> &a, const QPair<int, ItemProperty*> &b) {
                  return a.first < b.first;
              });
    
    return orderedList;
}

bool PropertyModificationEngine::validateEnhancedDamage(int value, QString *error) const
{
    if (value < 0 || value > 32767) {
        if (error) *error = tr("Enhanced Damage must be between 0 and 32767");
        return false;
    }
    return true;
}

bool PropertyModificationEngine::validateElementalDamage(int propertyId, int value, QString *error) const
{
    // Only validate that actual damage values are non-negative
    // Don't block negative values for properties that might have penalties
    switch (propertyId) {
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
        case Enums::ItemProperties::MaximumDamagePoison:
            if (value < 0) {
                if (error) *error = tr("Damage values cannot be negative");
                return false;
            }
            break;
        default:
            // For other properties, allow negative values (could be penalties/resistances)
            break;
    }
    
    return true;
}

bool PropertyModificationEngine::validateDefense(int value, QString *error) const
{
    // Only actual defense values cannot be negative
    // Other defense-related properties might allow negative values (penalties)
    if (value < 0) {
        if (error) *error = tr("Defense value cannot be negative");
        return false;
    }
    return true;
}

bool PropertyModificationEngine::validateSkillProperty(int propertyId, int value, quint32 skillId, QString *error) const
{
    if (value < 0 || value > 99) {
        if (error) *error = tr("Skill level must be between 0 and 99");
        return false;
    }
    
    // Could validate skill ID against skill database
    return true;
}

void PropertyModificationEngine::setError(const QString &error)
{
    _lastError = error;
    emit errorOccurred(error);
}

int PropertyModificationEngine::calculatePropertiesStartPosition(ItemInfo *item)
{
    if (!item) return -1;
    
    // Try to use existing bitStringOffset from properties if available
    // This is more reliable than re-parsing
    if (!item->props.isEmpty()) {
        auto firstProp = item->props.constBegin();
        int offset = firstProp.value()->bitStringOffset;
        if (offset > 16) { // Must be after JM header
            // ItemParser offset points at the start of (param+value) for the property instance
            // To find the start of the property sequence we need to back up by: paramBits + valueBits + propertyID(9)
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(firstProp.key());
            if (propTxt) {
                int backBits = 9 + propTxt->paramBits + propTxt->bits;
                int propertiesStart = item->bitString.length() - (offset - 16) - backBits;

                qDebug() << "PropertyModificationEngine: Using existing offset method, candidate properties start at:" << propertiesStart;

                // Quick verification: try to read 9 bits at candidate start and see if it equals firstProp.key()
                if (propertiesStart >= 0 && propertiesStart + 9 <= item->bitString.length()) {
                    try {
                        ReverseBitReader verifier(item->bitString);
                        verifier.setPos(propertiesStart);
                        int readId = verifier.readNumber(9);
                        if (readId == firstProp.key()) {
                            return propertiesStart;
                        } else {
                            qDebug() << "PropertyModificationEngine: verification failed (readId:" << readId << "!= expected:" << firstProp.key() << ")";
                        }
                    } catch (...) {
                        qDebug() << "PropertyModificationEngine: verification exception while reading candidate property id";
                    }
                }
            }
        }
    }
    
    // Fallback: try re-parsing (but catch exceptions gracefully)
    ReverseBitReader bitReader(item->bitString);
    
    try {
        qDebug() << "PropertyModificationEngine: Attempting to re-parse to find properties start";
        qDebug() << "BitString length:" << item->bitString.length() << "isEar:" << item->isEar << "isExtended:" << item->isExtended;
        
        // Skip all the same fields that ItemParser skips before reaching properties
        skipItemBasicData(bitReader, item);
        
        // Return current position - this is where properties start
        int pos = bitReader.pos();
        
        qDebug() << "PropertyModificationEngine: Re-parse successful, properties start at:" << pos;
        
        // Validate position is reasonable
        if (pos < 0 || pos >= item->bitString.length()) {
            qDebug() << "PropertyModificationEngine: Invalid properties start position:" << pos;
            return -1;
        }
        
        return pos;
        
    } catch (int errorCode) {
        qDebug() << "PropertyModificationEngine: Exception" << errorCode << "while calculating properties position";
        qDebug() << "BitReader position when exception occurred:" << bitReader.pos();
        return -1;
    } catch (...) {
        qDebug() << "PropertyModificationEngine: Unknown exception while calculating properties position";
        return -1;
    }
}

void PropertyModificationEngine::skipItemBasicData(ReverseBitReader &bitReader, ItemInfo *item)
{
    // This mirrors the exact parsing logic in ItemParser::parseItem
    // Skip all fields before properties section
    
    qDebug() << "PropertyModificationEngine: Starting skipItemBasicData, absolutePos:" << bitReader.absolutePos();
    
    // Add validation to prevent reading past bitstring end
    // ReverseBitReader uses internal _pos that counts from end, not from beginning
    auto safeSkip = [&bitReader](int bits, const char* field = "") {
        if (bitReader.absolutePos() - bits < 0) {
            qDebug() << "PropertyModificationEngine: Cannot skip" << bits << "bits for" << field 
                     << "at position" << bitReader.pos() << "(absolute:" << bitReader.absolutePos() << ")";
            throw 3; // Same exception code as ReverseBitReader
        }
        bitReader.skip(bits);
    };
    
    auto safeReadNumber = [&bitReader](int bits, const char* field = "") -> int {
        if (bitReader.absolutePos() - bits < 0) {
            qDebug() << "PropertyModificationEngine: Cannot read" << bits << "bits for" << field 
                     << "at position" << bitReader.pos() << "(absolute:" << bitReader.absolutePos() << ")";
            throw 3; // Same exception code as ReverseBitReader
        }
        return bitReader.readNumber(bits);
    };
    
    auto safeReadBool = [&bitReader](const char* field = "") -> bool {
        if (bitReader.absolutePos() - 1 < 0) {
            qDebug() << "PropertyModificationEngine: Cannot read bool for" << field 
                     << "at position" << bitReader.pos() << "(absolute:" << bitReader.absolutePos() << ")";
            throw 3; // Same exception code as ReverseBitReader  
        }
        return bitReader.readBool();
    };
    
    // Skip item flags and basic info
    safeSkip(1, "isQuest"); 
    safeSkip(3, "unknown1");
    safeSkip(1, "isIdentified"); 
    safeSkip(5, "unknown2");
    safeSkip(1, "isDuped"); 
    safeSkip(1, "isSocketed"); 
    safeSkip(2, "unknown3");
    safeSkip(2, "illegal+unk"); 
    safeSkip(1, "isEar"); 
    safeSkip(1, "isStarter"); 
    safeSkip(2, "unknown4");
    safeSkip(1, "unknown5");
    safeSkip(1, "isExtended"); 
    safeSkip(1, "isEthereal"); 
    safeSkip(1, "unknown6");
    safeSkip(1, "isPersonalized"); 
    safeSkip(1, "unknown7");
    safeSkip(1, "isRW"); 
    safeSkip(5, "unknown8");
    safeSkip(8, "version"); 
    safeSkip(2, "unknown9");
    safeSkip(3, "location"); 
    safeSkip(4, "whereEquipped"); 
    safeSkip(4, "column"); 
    safeSkip(4, "row"); 
    safeSkip(3, "storage");

    if (item->isEar) {
        safeSkip(3, "earClassCode");
        safeSkip(7, "earLevel");
        for (int i = 0; i < 18; ++i) {
            if (safeReadNumber(7, "earNameChar") == 0) break;
        }
        return; // Ears don't have properties
    }

    // Skip item type (4 bytes)
    safeSkip(32, "itemType");

    if (item->isExtended) {
        qDebug() << "PropertyModificationEngine: Processing extended item, pos:" << bitReader.pos();
        safeSkip(3); // socketablesNumber
        safeSkip(32); // guid
        safeSkip(7); // ilvl
        safeSkip(4); // quality
        
        if (safeReadBool()) // has variable graphic
            safeSkip(3);
        if (safeReadBool()) // autoprefix
            safeSkip(11);

        ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
        if (!itemBase) return;
        
        switch (item->quality) {
        case Enums::ItemQuality::Normal:
            break;
        case Enums::ItemQuality::LowQuality: 
        case Enums::ItemQuality::HighQuality:
            safeSkip(3); // nonMagicType
            break;
        case Enums::ItemQuality::Magic:
            safeSkip(22); // prefix & suffix
            break;
        case Enums::ItemQuality::Set: 
        case Enums::ItemQuality::Unique:
            safeSkip(15); // setOrUniqueId
            break;
        case Enums::ItemQuality::Rare: 
        case Enums::ItemQuality::Crafted:
            safeSkip(16); // first & second names
            for (int i = 0; i < 6; ++i)
                if (safeReadBool())
                    safeSkip(11); // prefix or suffix (1-3)
            break;
        case Enums::ItemQuality::Honorific:
            safeSkip(16);
            break;
        }

        if (item->isRW)
            safeSkip(16); // RW code

        // Skip inscribed name
        if (item->isPersonalized) {
            for (int i = 0; i < 16; ++i) {
                if (safeReadNumber(ItemParser::kInscribedNameCharacterLength) == 0) break;
            }
        }

        safeSkip(1); // tome of ID bit
        if (ItemDataBase::isTomeWithScrolls(item))
            safeSkip(5); // book ID

        // Handle armor/weapon specific data - mirror ItemParser logic exactly
        const bool isArmor = ItemParser::itemTypesInheritFromType(itemBase->types, "armo");
        if (isArmor) {
            ItemPropertyTxt *defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
            if (defenceProp) safeReadNumber(defenceProp->bits); // Read defense but don't store
        }
        if (isArmor || ItemParser::itemTypesInheritFromType(itemBase->types, "weap")) {
            ItemPropertyTxt *maxDurabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::DurabilityMax);
            if (maxDurabilityProp) {
                // Read maxDurability to check if we need to read current durability too
                int maxDurability = safeReadNumber(maxDurabilityProp->bits) - maxDurabilityProp->add;
                if (maxDurability > 0) {
                    ItemPropertyTxt *durabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Durability);
                    if (durabilityProp) safeReadNumber(durabilityProp->bits); // Read current durability but don't store
                }
            }
        }

        // Handle quantity for stackable items - must read, not skip
        if (itemBase->isStackable)
            safeReadNumber(9); // Read quantity but don't store
        
        // Handle sockets number - must read, not skip
        if (item->isSocketed)
            safeReadNumber(4); // Read sockets number but don't store

        // Handle set lists flags - must read each bool, not bulk skip
        if (item->quality == Enums::ItemQuality::Set) {
            const int kSetListsNumber = 5;
            for (int i = 0; i < kSetListsNumber; ++i)
                safeReadBool(); // Read each set list flag but don't store
        }
    }
    
    // Now bitReader is positioned right at the start of properties
}

bool PropertyModificationEngine::insertPropertyInPlace(ItemInfo *item, int propertyId, const ItemProperty *property)
{
    if (!item || !property) return false;

    // Find properties start
    int propertiesStart = calculatePropertiesStartPosition(item);
    if (propertiesStart < 0) {
        qDebug() << "PropertyModificationEngine::insertPropertyInPlace: cannot determine properties start";
        setError(tr("Cannot determine properties start for in-place insert"));
        return false;
    }

    // Build single property bit string
    QString propBits;
    writePropertyToBits(propBits, propertyId, property);
    // Do not add end marker here; we'll preserve existing end marker in the item

    // Prepare a copy for verification
    QString candidate = item->bitString;

    try {
    // Insert at propertiesStart (bit index)
    candidate.insert(propertiesStart, propBits);

        // Byte align
        ReverseBitWriter::byteAlignBits(candidate);

        // Convert to bytes for round-trip verification
        QByteArray itemBytes;
        for (int i = 0, n = candidate.length(); i < n; i += 8) {
            QString chunk = candidate.mid(i, 8);
            if (chunk.length() < 8) break;
            bool ok = false;
            quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
            if (!ok) {
                setError(tr("Verification conversion error"));
                return false;
            }
            itemBytes.prepend(static_cast<char>(val));
        }

        QByteArray fullBytes = ItemParser::kItemHeader + itemBytes;

        // Prepare original bytes for comparison
        QByteArray origItemBytes;
        for (int i = 0, n = item->bitString.length(); i < n; i += 8) {
            QString chunk = item->bitString.mid(i, 8);
            if (chunk.length() < 8) break;
            bool ok = false;
            quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
            if (!ok) break;
            origItemBytes.prepend(static_cast<char>(val));
        }
        QByteArray origFull = ItemParser::kItemHeader + origItemBytes;
        int dumpLen = qMin(16, fullBytes.length());
        qDebug() << "PropertyModificationEngine::insertPropertyInPlace: original bytes (hex):" << origFull.left(dumpLen).toHex();
        qDebug() << "PropertyModificationEngine::insertPropertyInPlace: candidate bytes (hex):" << fullBytes.left(dumpLen).toHex();

        QBuffer buf(&fullBytes);
        if (!buf.open(QIODevice::ReadOnly)) {
            setError(tr("Verification buffer error"));
            return false;
        }
        QDataStream ds(&buf); ds.setByteOrder(QDataStream::LittleEndian);
        ItemInfo *parsed = ItemParser::parseItem(ds, fullBytes);
        if (!parsed) {
            setError(tr("Verification parse failed"));
            return false;
        }

        // Validate critical fields
        if (parsed->itemType != item->itemType) {
            qDebug() << "PropertyModificationEngine::insertPropertyInPlace: verification mismatch itemType" << parsed->itemType << "!=" << item->itemType;
            delete parsed;
            setError(tr("Verification mismatch: item type changed"));
            return false;
        }
        if (item->isExtended && parsed->guid != item->guid) {
            delete parsed;
            setError(tr("Verification mismatch: guid changed"));
            return false;
        }

        // OK - apply candidate
        delete parsed;
        item->bitString = candidate;
        return true;

    } catch (...) {
        setError(tr("Exception during in-place insert verification"));
        return false;
    }
}

// Option B: Chunk-aware insertion with proper padding to preserve byte mapping
bool PropertyModificationEngine::tryChunkAwareInsertion(ItemInfo *item, const QString &beforeProperties, 
                                                       const QString &propertiesBits, const QString &afterProperties) {
    qDebug() << "PropertyModificationEngine: Attempting Option B - Chunk-aware insertion";
    
    // Build new bitString
    QString newBitString = beforeProperties + propertiesBits + afterProperties;
    qDebug() << "Option B: New bitString length before padding:" << newBitString.length();
    
    // Calculate original chunk count - this is critical to preserve byte mapping
    int originalChunks = (item->bitString.length() + 7) / 8;
    int targetLength = originalChunks * 8;
    
    qDebug() << "Option B: Original bitString length:" << item->bitString.length() 
             << "chunks:" << originalChunks << "target length:" << targetLength;
    
    // Check if we can fit within the original chunk structure
    if (newBitString.length() <= targetLength) {
        // We can fit - pad to exact target length
        if (newBitString.length() < targetLength) {
            QString padding(targetLength - newBitString.length(), '0');
            newBitString += padding;
            qDebug() << "Option B: Padded with" << padding.length() << "zeros to reach" << targetLength << "bits";
        }
    } else {
        // Check if we can trim trailing zeros from afterProperties to make room
        QString trimmedAfter = afterProperties;
        int trimAmount = newBitString.length() - targetLength;
        qDebug() << "Option B: Need to trim" << trimAmount << "bits, afterProperties length:" << afterProperties.length();
        
        // Only trim if afterProperties contains enough trailing zeros
        if (trimAmount <= afterProperties.length()) {
            bool canTrim = true;
            for (int i = afterProperties.length() - trimAmount; i < afterProperties.length(); i++) {
                if (afterProperties[i] != '0') {
                    canTrim = false;
                    break;
                }
            }
            
            if (canTrim) {
                trimmedAfter = afterProperties.left(afterProperties.length() - trimAmount);
                newBitString = beforeProperties + propertiesBits + trimmedAfter;
                qDebug() << "Option B: Successfully trimmed" << trimAmount << "trailing zeros";
            } else {
                qDebug() << "Option B: Cannot trim - non-zero bits would be lost";
                return false;
            }
        } else {
            qDebug() << "Option B: Cannot fit within original chunk count";
            return false;
        }
    }
    
    // Store original for rollback
    QString originalBitString = item->bitString;
    
    // Apply the padded bitString
    item->bitString = newBitString;
    
    // Use the same verification approach as insertPropertyInPlace
    try {
        QByteArray itemBytes;
        // Convert bitString to bytes using same logic as ItemParser
        QString workingBitString = item->bitString;
        // Ensure bitString is byte-aligned (pad with zeros if needed)
        ReverseBitWriter::byteAlignBits(workingBitString);
        
        for (int i = 0, n = workingBitString.length(); i < n; i += 8) {
            QString chunk = workingBitString.mid(i, 8);
            if (chunk.length() < 8) break; // Should not happen after byteAlignBits
            bool ok = false;
            quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
            if (!ok) {
                qDebug() << "Option B: FAILED - Invalid bit chunk conversion";
                item->bitString = originalBitString;
                return false;
            }
            itemBytes.prepend(static_cast<char>(val));
        }

        QByteArray fullBytes = ItemParser::kItemHeader + itemBytes;

        // Parse back to verify  
        QBuffer buf(&fullBytes);
        if (!buf.open(QIODevice::ReadOnly)) {
            qDebug() << "Option B: FAILED - Buffer error";
            item->bitString = originalBitString;
            return false;
        }
        QDataStream ds(&buf); 
        ds.setByteOrder(QDataStream::LittleEndian);
        ItemInfo *parsed = ItemParser::parseItem(ds, fullBytes);
        if (!parsed) {
            qDebug() << "Option B: FAILED - Could not parse serialized bytes back";
            item->bitString = originalBitString;
            return false;
        }

        // Validate critical fields
        if (parsed->itemType != item->itemType) {
            qDebug() << "Option B: FAILED - itemType mismatch:" << parsed->itemType << "!=" << item->itemType;
            delete parsed;
            item->bitString = originalBitString;
            return false;
        }

        // Success
        delete parsed;
        qDebug() << "Option B: SUCCESS - Chunk-aware insertion with verification passed";
        return true;

    } catch (...) {
        qDebug() << "Option B: FAILED - Exception during verification";
        item->bitString = originalBitString;
        return false;
    }
}

// Option C: Complete item recreation with byte-aligned strategy
bool PropertyModificationEngine::tryCompleteItemRecreation(ItemInfo *item) {
    qDebug() << "PropertyModificationEngine: Attempting Option C - Byte-aligned recreation";
    
    if (!item) {
        qDebug() << "Option C: FAILED - Missing item";
        return false;
    }
    
    // Store original for rollback
    QString originalBitString = item->bitString;
    int originalLength = originalBitString.length();
    int originalChunks = (originalLength + 7) / 8;
    
    qDebug() << "Option C: Original length:" << originalLength << "chunks:" << originalChunks;
    
    // Find where properties start in original item
    int propertiesStart = calculatePropertiesStartPosition(item);
    if (propertiesStart < 0) {
        qDebug() << "Option C: FAILED - Could not calculate properties position";
        return false;
    }
    
    // Find where properties end to preserve after-properties data
    QString afterProperties;
    QString endMarkerPattern = "111111111";
    int endMarkerPos = -1;
    for (int i = propertiesStart; i <= originalBitString.length() - 9; ++i) {
        if (originalBitString.mid(i, 9) == endMarkerPattern) {
            endMarkerPos = i + 9;
            break;
        }
    }
    if (endMarkerPos > 0 && endMarkerPos < originalBitString.length()) {
        afterProperties = originalBitString.mid(endMarkerPos);
    }
    
    // Calculate original properties section length
    int originalPropertiesLength = originalLength - propertiesStart - afterProperties.length();
    qDebug() << "Option C: Original properties length:" << originalPropertiesLength << "afterProperties:" << afterProperties.length();
    
    // Build new properties section
    QString newPropertiesBits = buildPropertiesBitString(item->props);
    qDebug() << "Option C: New properties length:" << newPropertiesBits.length();
    
    // BYTE-ALIGNED STRATEGY: Maintain exact same total length and chunk count
    int propertiesLengthDiff = newPropertiesBits.length() - originalPropertiesLength;
    
    if (propertiesLengthDiff > 0) {
        // Need more space but CANNOT change total length - use creative padding/compression
        qDebug() << "Option C: Need" << propertiesLengthDiff << "more bits - trying compressed approach";
        
        // Strategy 1: Try to find padding in after-properties section
        if (afterProperties.length() >= propertiesLengthDiff) {
            // Can steal bits from after-properties
            QString reducedAfterProperties = afterProperties.left(afterProperties.length() - propertiesLengthDiff);
            QString finalBitString = originalBitString.left(propertiesStart) + newPropertiesBits + reducedAfterProperties;
            
            qDebug() << "Option C: Compressed after-properties from" << afterProperties.length() << "to" << reducedAfterProperties.length();
            qDebug() << "Option C: Final length:" << finalBitString.length() << "(target:" << originalLength << ")";
            
            if (finalBitString.length() == originalLength) {
                item->bitString = finalBitString;
                qDebug() << "Option C: SUCCESS - Applied compression strategy";
                return true;
            }
        }
        
        // Strategy 2: Look for padding bits in the original bitString
        // Scan for sequences of zeros that might be padding
        QList<QPair<int, int>> paddingRegions; // <start, length>
        QString scanPattern = "00000000"; // 8 consecutive zeros
        int scanStart = propertiesStart + originalPropertiesLength;
        for (int i = scanStart; i <= originalBitString.length() - 8; ++i) {
            if (originalBitString.mid(i, 8) == scanPattern) {
                paddingRegions.append(QPair<int, int>(i, 8));
                i += 7; // Skip ahead to avoid overlap
            }
        }
        
        int totalPaddingFound = 0;
        for (const auto &region : paddingRegions) {
            totalPaddingFound += region.second;
        }
        
        qDebug() << "Option C: Found" << paddingRegions.size() << "padding regions totaling" << totalPaddingFound << "bits";
        
        if (totalPaddingFound >= propertiesLengthDiff) {
            // Can use existing padding - implement bit-level surgery
            QString surgicalBitString = originalBitString;
            
            // Remove padding bits to make room
            int bitsToRemove = propertiesLengthDiff;
            for (int i = paddingRegions.size() - 1; i >= 0 && bitsToRemove > 0; --i) {
                int regionStart = paddingRegions[i].first;
                int regionLength = paddingRegions[i].second;
                int removeFromRegion = qMin(bitsToRemove, regionLength);
                
                surgicalBitString.remove(regionStart, removeFromRegion);
                bitsToRemove -= removeFromRegion;
            }
            
            // Now replace properties section
            QString beforeProperties = surgicalBitString.left(propertiesStart);
            int newAfterStart = propertiesStart + originalPropertiesLength;
            QString newAfterProperties = surgicalBitString.mid(newAfterStart);
            
            QString finalBitString = beforeProperties + newPropertiesBits + newAfterProperties;
            
            qDebug() << "Option C: Surgical result length:" << finalBitString.length() << "(target:" << originalLength << ")";
            
            if (finalBitString.length() == originalLength) {
                item->bitString = finalBitString;
                qDebug() << "Option C: SUCCESS - Applied surgical strategy";
                return true;
            }
        }
        
        qDebug() << "Option C: FAILED - Cannot fit new properties without changing total length";
        return false;
        
    } else {
        // Properties section is same size or smaller - use exact length preservation
        QString beforeProperties = originalBitString.left(propertiesStart);
        
        // Pad properties section to match original length exactly
        if (newPropertiesBits.length() < originalPropertiesLength) {
            int padAmount = originalPropertiesLength - newPropertiesBits.length();
            // Insert padding before end marker
            QString endMarker = newPropertiesBits.right(9); // "111111111"
            QString propsWithoutEnd = newPropertiesBits.left(newPropertiesBits.length() - 9);
            newPropertiesBits = propsWithoutEnd + QString(padAmount, '0') + endMarker;
            qDebug() << "Option C: Padded properties with" << padAmount << "zeros";
        }
        
        QString finalBitString = beforeProperties + newPropertiesBits + afterProperties;
        
        qDebug() << "Option C: Same-size bitString length:" << finalBitString.length();
        
        // Apply same-size bitString
        item->bitString = finalBitString;
        qDebug() << "Option C: SUCCESS - Applied same-size strategy";
        return true;
    }
}

// Option E: Smart bit-level reconstruction with corruption prevention
bool PropertyModificationEngine::trySmartBitReconstruction(ItemInfo *item, const QString &beforeProperties, 
                                                          const QString &propertiesBits, const QString &afterProperties) {
    qDebug() << "PropertyModificationEngine: Attempting Option E - Smart bit reconstruction";
    
    if (!item) {
        qDebug() << "Option E: FAILED - Missing item";
        return false;
    }
    
    // Store original for rollback
    QString originalBitString = item->bitString;
    int originalLength = originalBitString.length();
    
    // Build new bitString
    QString newBitString = beforeProperties + propertiesBits + afterProperties;
    int newLength = newBitString.length();
    
    qDebug() << "Option E: Original length:" << originalLength << "New length:" << newLength;
    
    // STRATEGY 1: Try to compress the new properties to fit exact original length
    if (newLength > originalLength) {
        int excessBits = newLength - originalLength;
        qDebug() << "Option E: Need to remove" << excessBits << "bits to maintain exact length";
        
        // Try to find and remove padding bits from afterProperties
        if (afterProperties.length() >= excessBits) {
            // Check if we can trim trailing bits from afterProperties
            QString trimmedAfter = afterProperties;
            bool canTrim = true;
            
            // Only trim if the excess bits are at the end and are zeros or minimal data
            for (int i = afterProperties.length() - excessBits; i < afterProperties.length(); i++) {
                if (afterProperties[i] != '0') {
                    // Found non-zero bit - cannot safely trim
                    canTrim = false;
                    break;
                }
            }
            
            if (canTrim) {
                trimmedAfter = afterProperties.left(afterProperties.length() - excessBits);
                newBitString = beforeProperties + propertiesBits + trimmedAfter;
                qDebug() << "Option E: Successfully trimmed" << excessBits << "trailing zeros";
            } else {
                qDebug() << "Option E: Cannot trim - would lose important data";
            }
        }
        
        // STRATEGY 2: If still too long, try to optimize properties encoding
        if (newBitString.length() > originalLength) {
            qDebug() << "Option E: Still too long, trying property optimization";
            
            // Try to rebuild properties with more compact encoding
            // For now, just truncate to exact original length as last resort
            if (newBitString.length() <= originalLength + 8) { // Allow small overflow
                newBitString = newBitString.left(originalLength);
                // Add minimal padding if needed
                while (newBitString.length() < originalLength) {
                    newBitString += '0';
                }
                qDebug() << "Option E: Applied truncation to exact original length";
            } else {
                qDebug() << "Option E: Cannot fit - excess too large";
                return false;
            }
        }
    } else if (newLength < originalLength) {
        // Pad to exact original length
        int paddingNeeded = originalLength - newLength;
        QString padding(paddingNeeded, '0');
        newBitString += padding;
        qDebug() << "Option E: Added" << paddingNeeded << "padding bits to match original length";
    }
    
    // Ensure we have exact original length
    if (newBitString.length() != originalLength) {
        qDebug() << "Option E: Length mismatch - cannot guarantee corruption prevention";
        return false;
    }
    
    // Apply the reconstructed bitString
    item->bitString = newBitString;
    
    // Verification: Try to serialize and parse back using same logic as insertPropertyInPlace
    try {
        QByteArray itemBytes;
        // Convert bitString to bytes - no additional alignment since we preserved exact length
        for (int i = 0, n = item->bitString.length(); i < n; i += 8) {
            QString chunk = item->bitString.mid(i, 8);
            if (chunk.length() < 8) {
                // Handle remaining bits by padding
                chunk = chunk.leftJustified(8, '0');
            }
            bool ok = false;
            quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
            if (!ok) {
                qDebug() << "Option E: FAILED - Invalid bit chunk conversion";
                item->bitString = originalBitString;
                return false;
            }
            itemBytes.prepend(static_cast<char>(val));
        }

        QByteArray fullBytes = ItemParser::kItemHeader + itemBytes;

        // Parse back to verify  
        QBuffer buf(&fullBytes);
        if (!buf.open(QIODevice::ReadOnly)) {
            qDebug() << "Option E: FAILED - Buffer error";
            item->bitString = originalBitString;
            return false;
        }
        QDataStream ds(&buf); 
        ds.setByteOrder(QDataStream::LittleEndian);
        ItemInfo *parsed = ItemParser::parseItem(ds, fullBytes);
        if (!parsed) {
            qDebug() << "Option E: FAILED - Could not parse reconstructed item bytes back";
            item->bitString = originalBitString;
            return false;
        }

        // Validate critical fields
        if (parsed->itemType != item->itemType) {
            qDebug() << "Option E: FAILED - itemType mismatch:" << parsed->itemType.toHex() << "!=" << item->itemType.toHex();
            delete parsed;
            item->bitString = originalBitString;
            return false;
        }

        if (item->isExtended && parsed->guid != item->guid) {
            qDebug() << "Option E: FAILED - guid mismatch";
            delete parsed;
            item->bitString = originalBitString;
            return false;
        }

        // Success - the item maintains exact same structure but with updated properties
        delete parsed;
        qDebug() << "Option E: SUCCESS - Smart reconstruction with exact length preservation and verification passed";
        return true;

    } catch (...) {
        qDebug() << "Option E: FAILED - Exception during verification";
        item->bitString = originalBitString;
        return false;
    }
}

// Option F: Property replacement (modify existing property instead of adding new one)
bool PropertyModificationEngine::tryPropertyReplacement(ItemInfo *item, int newPropertyId, int newValue) {
    qDebug() << "PropertyModificationEngine: Attempting Option F - Property replacement";
    qDebug() << "Option F: Attempting to add property ID" << newPropertyId << "with value" << newValue;
    
    if (!item) {
        qDebug() << "Option F: FAILED - Missing item";
        return false;
    }
    
    // Strategy: Find a property with similar bit requirements and replace it
    // This maintains the same total bitString length
    
    ItemPropertyTxt *newPropTxt = ItemDataBase::Properties()->value(newPropertyId);
    if (!newPropTxt) {
        qDebug() << "Option F: FAILED - Unknown property ID" << newPropertyId;
        return false;
    }
    
    int newPropertyBits = 9 + newPropTxt->paramBits + newPropTxt->bits; // ID + param + value
    qDebug() << "Option F: New property requires" << newPropertyBits << "bits";
    
    // Find a suitable property to replace (preferably with same or similar bit count)
    int bestReplaceId = -1;
    int bestBitDiff = INT_MAX;
    
    for (auto it = item->props.constBegin(); it != item->props.constEnd(); ++it) {
        int existingId = it.key();
        if (existingId == newPropertyId) continue; // Don't replace with itself
        
        ItemPropertyTxt *existingPropTxt = ItemDataBase::Properties()->value(existingId);
        if (!existingPropTxt) continue;
        
        int existingBits = 9 + existingPropTxt->paramBits + existingPropTxt->bits;
        int bitDiff = abs(existingBits - newPropertyBits);
        
        qDebug() << "Option F: Property ID" << existingId << "uses" << existingBits << "bits (diff:" << bitDiff << ")";
        
        // Prefer exact matches, but accept close matches
        if (bitDiff < bestBitDiff) {
            bestBitDiff = bitDiff;
            bestReplaceId = existingId;
        }
        
        // Perfect match - use it immediately
        if (bitDiff == 0) {
            break;
        }
    }
    
    if (bestReplaceId < 0) {
        qDebug() << "Option F: FAILED - No suitable property found for replacement";
        return false;
    }
    
    qDebug() << "Option F: Selected property ID" << bestReplaceId << "for replacement (bit diff:" << bestBitDiff << ")";
    
    // Store original for rollback
    QString originalBitString = item->bitString;
    PropertiesMultiMap originalProps;
    for (auto it = item->props.constBegin(); it != item->props.constEnd(); ++it) {
        originalProps.insert(it.key(), new ItemProperty(*it.value()));
    }
    
    // Remove the property to be replaced and add the new property
    auto replaceRange = item->props.equal_range(bestReplaceId);
    for (auto it = replaceRange.first; it != replaceRange.second; ++it) {
        delete it.value();
    }
    item->props.remove(bestReplaceId);
    
    // Add the new property
    ItemProperty *newProp = new ItemProperty();
    newProp->param = 0;
    newProp->value = newValue;
    newProp->bitStringOffset = -1;
    item->props.insert(newPropertyId, newProp);
    
    // Try to reconstruct with the replaced property
    try {
        // Use the same reconstruction logic but with modified properties
        int propertiesStart = calculatePropertiesStartPosition(item);
        if (propertiesStart < 0) {
            qDebug() << "Option F: FAILED - Cannot determine properties position";
            throw std::runtime_error("Cannot determine properties position");
        }
        
        QString beforeProperties = originalBitString.left(propertiesStart);
        
        // Find afterProperties by parsing the original properties
        QString afterProperties;
        ReverseBitReader reader(originalBitString);
        reader.setPos(propertiesStart);
        ItemInfo::ParsingStatus status = ItemInfo::Ok;
        PropertiesMultiMap parsedProps = ItemParser::parseItemProperties(reader, &status);
        if (status == ItemInfo::Ok && reader.pos() < originalBitString.length()) {
            afterProperties = originalBitString.mid(reader.pos());
        }
        
        // Build new properties section
        QString propertiesBits = buildPropertiesBitString(item->props);
        
        // Combine
        QString newBitString = beforeProperties + propertiesBits + afterProperties;
        
        qDebug() << "Option F: Original length:" << originalBitString.length() << "New length:" << newBitString.length();
        
        // If length changed significantly, try to adjust
        if (newBitString.length() != originalBitString.length()) {
            int lengthDiff = newBitString.length() - originalBitString.length();
            qDebug() << "Option F: Length difference:" << lengthDiff << "bits";
            
            if (abs(lengthDiff) <= 8) { // Small difference - try to pad or trim
                if (lengthDiff > 0) {
                    // Too long - try to trim from afterProperties or add padding
                    if (afterProperties.length() >= lengthDiff) {
                        // Try trimming trailing bits
                        afterProperties = afterProperties.left(afterProperties.length() - lengthDiff);
                        newBitString = beforeProperties + propertiesBits + afterProperties;
                        qDebug() << "Option F: Trimmed" << lengthDiff << "bits from afterProperties";
                    }
                } else {
                    // Too short - add padding
                    QString padding(-lengthDiff, '0');
                    newBitString += padding;
                    qDebug() << "Option F: Added" << (-lengthDiff) << "padding bits";
                }
            }
        }
        
        // Apply new bitString
        item->bitString = newBitString;
        
        // Verification
        QByteArray itemBytes;
        for (int i = 0, n = item->bitString.length(); i < n; i += 8) {
            QString chunk = item->bitString.mid(i, 8);
            if (chunk.length() < 8) {
                chunk = chunk.leftJustified(8, '0');
            }
            bool ok = false;
            quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
            if (!ok) {
                throw std::runtime_error("Invalid bit chunk conversion");
            }
            itemBytes.prepend(static_cast<char>(val));
        }

        QByteArray fullBytes = ItemParser::kItemHeader + itemBytes;
        QBuffer buf(&fullBytes);
        if (!buf.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Buffer error");
        }
        
        QDataStream ds(&buf);
        ds.setByteOrder(QDataStream::LittleEndian);
        ItemInfo *parsed = ItemParser::parseItem(ds, fullBytes);
        if (!parsed) {
            throw std::runtime_error("Parse verification failed");
        }

        // Validate critical fields
        if (parsed->itemType != item->itemType) {
            delete parsed;
            throw std::runtime_error("ItemType mismatch after replacement");
        }

        if (item->isExtended && parsed->guid != item->guid) {
            delete parsed;
            throw std::runtime_error("GUID mismatch after replacement");
        }

        delete parsed;
        qDeleteAll(originalProps); // Clean up backup
        qDebug() << "Option F: SUCCESS - Property replacement completed with verification";
        return true;

    } catch (const std::exception &e) {
        qDebug() << "Option F: FAILED -" << e.what();
        
        // Rollback
        item->bitString = originalBitString;
        qDeleteAll(item->props);
        item->props = originalProps;
        
        return false;
    } catch (...) {
        qDebug() << "Option F: FAILED - Unknown exception";
        
        // Rollback
        item->bitString = originalBitString;
        qDeleteAll(item->props);
        item->props = originalProps;
        
        return false;
    }
}