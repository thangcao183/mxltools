#include <QApplication>
#include <QFile>
#include <QDebug>

// Tool to reverse-engineer byte to bit conversion
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    
    if (argc < 2) {
        qDebug() << "Usage: reverse_engineer_bytes <file.d2i>";
        return 1;
    }
    
    QString fileName = argv[1];
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << fileName;
        return 2;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    qDebug() << "=== REVERSE ENGINEERING BYTE TO BIT CONVERSION ===";
    qDebug() << "File size:" << data.size() << "bytes";
    qDebug() << "Header:" << data.left(2).toHex(' ');
    qDebug() << "Item data:" << (data.size() - 2) << "bytes";
    
    // Convert first 8 bytes of item data to bits using different methods
    QByteArray itemData = data.mid(2); // Skip header
    
    qDebug() << "\n=== TESTING DIFFERENT BIT CONVERSION METHODS ===";
    
    for (int method = 1; method <= 3; method++) {
        qDebug() << "\n--- Method" << method << "---";
        QString bitString;
        
        // Convert first 8 bytes to bits
        for (int i = 0; i < qMin(8, itemData.size()); i++) {
            quint8 byte = itemData[i];
            QString byteBits;
            
            if (method == 1) {
                // Method 1: MSB first (standard binary)
                for (int j = 7; j >= 0; j--) {
                    byteBits += ((byte >> j) & 1) ? '1' : '0';
                }
            } else if (method == 2) {
                // Method 2: LSB first (reverse order)
                for (int j = 0; j < 8; j++) {
                    byteBits += ((byte >> j) & 1) ? '1' : '0';
                }
            } else if (method == 3) {
                // Method 3: Byte-reverse then MSB first
                quint8 reversedByte = 0;
                for (int j = 0; j < 8; j++) {
                    if (byte & (1 << j)) {
                        reversedByte |= (1 << (7 - j));
                    }
                }
                for (int j = 7; j >= 0; j--) {
                    byteBits += ((reversedByte >> j) & 1) ? '1' : '0';
                }
            }
            
            bitString += byteBits;
            qDebug() << "Byte" << i << "=" << QString("0x%1").arg(byte, 2, 16, QChar('0')) 
                     << "bits:" << byteBits;
        }
        
        qDebug() << "Method" << method << "result:" << bitString;
    }
    
    // Expected bitString start (from research tool)
    QString expected = "0001111111110001011110110000000001000111101001000111100111010110";
    qDebug() << "\n=== COMPARISON WITH EXPECTED ===";
    qDebug() << "Expected:   " << expected;
    
    return 0;
}