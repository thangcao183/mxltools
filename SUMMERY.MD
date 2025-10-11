Đây chính là lý do tại sao khi bitString length thay đổi, nó corrupt file! Chunk-prepend order có nghĩa là:

bitString [0-7][8-15][16-23][24-31] becomes bytes [byte3][byte2][byte1][byte0]
BitString length: 448 bits (56 bytes × 8)
Chunk-prepend logic hoạt động đúng: Bytes được prepend theo thứ tự chunk [0][1][2]...[55] → file bytes [55][54][53]...[0]
ItemParser logic:

Lấy từng 8-bit chunk từ bitString (i = 0, 8, 16, 24, ...)
Convert chunk thành short (binary string → number): item->bitString.mid(i, 8).toShort(0, 2)
PREPEND chunk vào itemBytes: itemBytes.prepend(...)
Điều này có nghĩa là:

Chunk 0 (bits 0-7) → prepend → becomes last byte
Chunk 1 (bits 8-15) → prepend → becomes second-to-last byte
Chunk N (bits N8 to N8+7) → prepend → becomes first byte
Header: "JM" (0x4A, 0x4D)
## D2I File Structure - Đã Phân Tích Hoàn Chỉnh

### File Format
```
[JM Header: 2 bytes][Item Data: Variable bytes]
```

### BitString Structure (Quan trọng!)
```
[Content Bits][End Marker: 9 bits][Padding Bits]

thêm a*0 bit vào với a là phần còn thiếu để cho số bit cộng với a chia hết cho 8


Perfect! Vậy logic đúng là:

numberToBinary: MSB-first
reverse: Chuyển thành LSB-first cho property bits
byteToBinary/binaryToByte: LSB-first
