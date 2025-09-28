# Hướng dẫn Build Windows EXE - MedianXL Offline Tools

## 🎯 **Tóm tắt các phương pháp**

### 1. **Native Windows Build (Khuyến nghị)**
### 2. **Cross-compile từ Linux (Phức tạp hơn)**  
### 3. **Sử dụng GitHub Actions (Tự động)**

---

## 🏆 **Phương pháp 1: Native Windows Build (Dễ nhất)**

### **Bước 1: Chuẩn bị môi trường Windows**
- **Windows 10/11**
- **Visual Studio 2019+ Community** (free)
- **Qt5** (5.15.x khuyến nghị)
- **CMake 3.18+**
- **Git**

### **Bước 2: Cài đặt dependencies**

#### **Cài Qt5:**
1. Download Qt Online Installer: https://www.qt.io/download-qt-installer
2. Install Qt 5.15.2 với:
   - ✅ **MSVC 2019 64-bit**  
   - ✅ **Qt Creator**
   - ✅ **CMake**

#### **Cài Visual Studio:**
1. Download Visual Studio Community 2022
2. Chọn workload: **"Desktop development with C++"**

### **Bước 3: Build**

```batch
# Clone repository
git clone https://github.com/kambala-decapitator/MedianXLOfflineTools.git
cd MedianXLOfflineTools

# Tạo build directory
mkdir build-win
cd build-win

# Configure với CMake
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DQt5_DIR="C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5"

# Build
cmake --build . --config Release

# Executable sẽ ở: build-win/Release/MedianXLOfflineTools.exe
```

### **Bước 4: Deploy (tạo installer)**

```batch
# Copy exe
mkdir dist
copy Release\\MedianXLOfflineTools.exe dist\\

# Copy Qt DLLs
cd dist
C:\\Qt\\5.15.2\\msvc2019_64\\bin\\windeployqt.exe MedianXLOfflineTools.exe

# Tạo package
"C:\\Program Files\\7-Zip\\7z.exe" a MedianXLOfflineTools_Windows.7z *
```

---

## 🐧 **Phương pháp 2: Cross-compile từ Linux**

### **Bước 1: Cài MXE (M cross environment)**

```bash
# Install dependencies
sudo apt install autoconf automake autopoint bash bison bzip2 \\
    flex g++ g++-multilib gettext git gperf intltool libc6-dev-i386 \\
    libgdk-pixbuf2.0-dev libltdl-dev libssl-dev libtool-bin \\
    libxml-parser-perl lzip make openssl p7zip-full patch perl \\
    pkg-config python ruby sed unzip wget xz-utils

# Clone MXE
cd /opt
sudo git clone https://github.com/mxe/mxe.git
cd mxe

# Build Qt5 for MinGW (mất 1-2 giờ!)
sudo make MXE_TARGETS='x86_64-w64-mingw32.static' qtbase qttools

# Add to PATH
echo 'export PATH=/opt/mxe/usr/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### **Bước 2: Build với MXE**

```bash
cd /path/to/MedianXLOfflineTools
mkdir build-mxe
cd build-mxe

# Configure
x86_64-w64-mingw32.static-cmake .. \\
    -DCMAKE_BUILD_TYPE=Release

# Build  
make -j$(nproc)

# Kết quả: MedianXLOfflineTools.exe (static, không cần DLL)
```

---

## 🤖 **Phương pháp 3: GitHub Actions (Tự động)**

### **Tạo file `.github/workflows/build.yml`:**

```yaml
name: Build Windows
on: [push, pull_request]

jobs:
  windows:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Install Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '5.15.2'
        
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      
    - name: Build
      run: cmake --build build --config Release
      
    - name: Deploy Qt
      run: |
        mkdir dist
        copy build\\Release\\MedianXLOfflineTools.exe dist\\
        windeployqt dist\\MedianXLOfflineTools.exe
        
    - name: Upload artifacts
      uses: actions/upload-artifact@v3
      with:
        name: MedianXLOfflineTools-Windows
        path: dist/
```

---

## 🛠 **Phương pháp 4: Docker Build**

### **Tạo Dockerfile:**

```dockerfile
FROM mxe/mxe:latest as builder

RUN apt-get update && apt-get install -y git cmake

# Build Qt5 for MXE
RUN cd /usr/src/mxe && \\
    make MXE_TARGETS='x86_64-w64-mingw32.static' qtbase qttools

COPY . /src
WORKDIR /src

RUN mkdir build && cd build && \\
    x86_64-w64-mingw32.static-cmake .. -DCMAKE_BUILD_TYPE=Release && \\
    make -j$(nproc)

FROM alpine:latest
COPY --from=builder /src/build/MedianXLOfflineTools.exe /
CMD ["cp", "/MedianXLOfflineTools.exe", "/output/"]
```

### **Build với Docker:**

```bash
# Build image
docker build -t medianxl-builder .

# Extract exe
mkdir output
docker run --rm -v $(pwd)/output:/output medianxl-builder
```

---

## 📦 **Cách tạo Installer (NSIS)**

### **Tạo file `installer.nsi`:**

```nsis
!define APP_NAME "MedianXL Offline Tools"
!define APP_VERSION "0.6.6"

OutFile "MedianXLOfflineTools_Setup.exe"
InstallDir "$PROGRAMFILES64\\${APP_NAME}"

Section "Install"
    SetOutPath $INSTDIR
    File "MedianXLOfflineTools.exe"
    File /r "platforms"
    File /r "styles"
    File "*.dll"
    
    CreateShortCut "$DESKTOP\\${APP_NAME}.lnk" "$INSTDIR\\MedianXLOfflineTools.exe"
    
    WriteUninstaller "$INSTDIR\\uninstall.exe"
SectionEnd
```

### **Build installer:**

```batch
"C:\\Program Files (x86)\\NSIS\\makensis.exe" installer.nsi
```

---

## ⚡ **Quick Build trên Windows (Nhanh nhất)**

```batch
REM Assumes Qt5 installed in C:\\Qt\\5.15.2\\msvc2019_64
set QT_DIR=C:\\Qt\\5.15.2\\msvc2019_64
set PATH=%QT_DIR%\\bin;%PATH%

mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

REM Deploy
mkdir dist
copy Release\\MedianXLOfflineTools.exe dist\\
cd dist
%QT_DIR%\\bin\\windeployqt.exe MedianXLOfflineTools.exe
```

---

## 🎯 **Khuyến nghị:**

1. **Cho phát triển:** Phương pháp 1 (Native Windows)
2. **Cho CI/CD:** Phương pháp 3 (GitHub Actions)  
3. **Cho production:** Phương pháp 1 + NSIS installer
4. **Cho advanced users:** Phương pháp 2 (MXE cross-compile)

**Native Windows build sẽ cho kết quả tốt nhất và dễ debug nhất!**