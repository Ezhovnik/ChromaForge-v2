<div align="center">

# ChromaForge

## *Воксельный движок, стирающий границы между мечтой и реальностью*

[![Version](https://img.shields.io/badge/version-0.3.0-green?style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/releases/latest)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey?style=flat-square)]

**ChromaForge** — это воксельный движок с открытым исходным кодом, вдохновлённый Minecraft и Hytale.  
Творите, создавайте, вдохновляйте и вдохновляйтесь!
</div>

---

## 📸 Скриншоты

[![Закат](doc/screenshots/screenshot1_thumb.png)](doc/screenshots/screenshot1.png)
[![Ночное время суток](doc/screenshots/screenshot2_thumb.png)](doc/screenshots/screenshot2.png)
[![Цветное освещение](doc/screenshots/screenshot3_thumb.png)](doc/screenshots/screenshot3.png)
[![Творите!](doc/screenshots/screenshot4_thumb.png)](doc/screenshots/screenshot4.png)

---

## 📦 Быстрый старт (Windows)

Скачайте последнюю сборку:

[**ChromaForge-v0.3.0-Windows-x64.zip**](https://github.com/Ezhovnik/ChromaForge-v2/releases/download/v0.3.0/ChromaForge-v.0.3.0-Windows-64-bit.zip)

Распакуйте архив и запустите `ChromaForge.exe`.

---

## Сборка проекта

### Windows (Visual Studio + vcpkg)

> Требуется: **vcpkg**, **CMake**, **Git**, **Visual Studio** (с C++ инструментами).

1. Установите **vcpkg**:
```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

2. Установите переменную окружения `VCPKG_ROOT`:
```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"
```

3. Соберите проект:
```powershell
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2
cmake --preset default-vs-msvc-windows
cmake --build --preset "MSVC Release" --config Release
```

### Windows (MinGW)

> Требуется: **CMake**, **Git**, **MSYS2** с MinGW toolchain.

```powershell
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2
cmake --preset default-ninja-mingw-windows -DCMAKE_BUILD_TYPE=Debug
cmake --build --preset "MinGW Debug"
```

### Linux (нативная сборка)

> Требуется: **CMake**, **Git**, **g++**, **pkg-config**.

Установите зависимости:

```sh
# Debian/Ubuntu
sudo apt install libglfw3-dev libglfw3 libglew-dev libopenal-dev libluajit-5.1-dev libvorbis-dev libcurl4-openssl-dev libfmt-dev libspdlog-dev

# EnTT (последняя версия под C++17)
git clone --branch v3.16.0 https://github.com/skypjack/entt.git
cd entt/build
cmake -DCMAKE_BUILD_TYPE=Release -DENTT_INSTALL=ON ..
sudo make install
```

Сборка через `run.sh`:

```sh
chmod +x run.sh
./run.sh --build      # собрать
./run.sh --rebuild    # пересобрать (с удалением build/)
./run.sh              # собрать и запустить
```

### Linux (Docker)

> Требуется: **Docker Engine**.

```sh
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2
docker build -t chromaforge .
docker run --rm -it -v "$(pwd):/project" chromaforge bash -c "cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && cmake --build build --parallel"
```

---

## Управление по умолчанию

- <kbd>**Esc**</kbd> — пауза
- <kbd>**E**</kbd> — открытие инвенторя
- <kbd>**W**</kbd> <kbd>**A**</kbd> <kbd>**S**</kbd> <kbd>**D**</kbd> — передвижение
- <kbd>**C**</kbd> — приближение
- <kbd>**Space**</kbd> — прыжок
- <kbd>**LMB**</kbd> — разрушить блок
- <kbd>**RMB**</kbd> — поставить блок
- <kbd>**MMB**</kbd> — взять блок
- <kbd>**F**</kbd> — включить/выключить полёт
- <kbd>**N**</kbd> — включить/выключить "режим наблюдателя"
- <kbd>**F1**</kbd> — включить/выключить видимость инвенторя
- <kbd>**F2**</kbd> — сохранить скриншот
- <kbd>**F3**</kbd> — включить/выключить режим отладки
- <kbd>**F4**</kbd> — переключить вид камеры
- <kbd>**F5**</kbd> — перезагрузить чанки
