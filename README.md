# ChromaForge

[![Version](https://img.shields.io/badge/version-0.3.0-green?style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/releases/latest)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2)

Воксельный движок с открытым исходным кодом, вдохновлённый Minecraft и Hytale.

**Состояния сборок:**

- [![Linux](https://img.shields.io/github/actions/workflow/status/Ezhovnik/ChromaForge-v2/build.yml?label=Linux&style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/actions)
- [![Linux AppImage](https://img.shields.io/github/actions/workflow/status/Ezhovnik/ChromaForge-v2/appimage.yml?label=Linux%20AppImage&style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/actions)
- [![Windows (MSVC)](https://img.shields.io/github/actions/workflow/status/Ezhovnik/ChromaForge-v2/windows.yml?label=Windows%20(MSVC)&style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/actions)
- [![Windows (MinGW+Clang)](https://img.shields.io/github/actions/workflow/status/Ezhovnik/ChromaForge-v2/windows-clang.yml?label=Windows%20(MinGW%2BClang)&style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/actions)
- [![macOS](https://img.shields.io/github/actions/workflow/status/Ezhovnik/ChromaForge-v2/macos.yml?label=macOS&style=flat-square)](https://github.com/Ezhovnik/ChromaForge-v2/actions)

## 📸 Скриншоты

[![Закат](doc/screenshots/screenshot1_thumb.png)](doc/screenshots/screenshot1.png)
[![Ночное время суток](doc/screenshots/screenshot2_thumb.png)](doc/screenshots/screenshot2.png)
[![Цветное освещение](doc/screenshots/screenshot3_thumb.png)](doc/screenshots/screenshot3.png)
[![Творите!](doc/screenshots/screenshot4_thumb.png)](doc/screenshots/screenshot4.png)

## Документация

- [Документация](doc/ru/main-page.md)
- [Documentation](doc/en/main-page.md)

## Сборка проекта

### Windows (Visual Studio + vcpkg)

> [!NOTE]
> Требуется: **vcpkg**, **CMake** (≥ 3.26), **Git**, **Visual Studio** (с C++ инструментами).
<!-- -->
> [!TIP]
> После установки vcpkg укажите его путь в переменной окружения `VCPKG_ROOT`.

```powershell
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2
cmake --preset default-vs-msvc-windows
cmake --build --preset "MSVC Release" --config Release
```

### Windows (MinGW)

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **MSYS2** с MinGW toolchain (clang64), **Ninja**, **vcpkg**.
<!-- -->
> [!WARNING]
> Избегайте установки инструментов в пути, содержащие пробелы (например, `Program Files`).
> Это может сломать сборку MinGW (особенно `windres`).
<!-- -->
> [!TIP]
> Если vcpkg ещё не установлен, склонируйте его и выполните начальную настройку:
> ```shell
> git clone https://github.com/microsoft/vcpkg.git
> cd vcpkg && .\bootstrap-vcpkg.bat && cd ..
> ```
<!-- -->
> [!IMPORTANT]
> Команды ниже должны выполняться в **MSYS2 shell** (clang64), а не в PowerShell или cmd.

```msys2
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2

cmake --preset default-ninja-mingw-windows \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
  -DCMAKE_TOOLCHAIN_FILE="$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build-mingw --config Release
```

### Linux (нативная сборка)

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **g++**, **pkg-config**.

Сборка (после установки зависимостей для вашего дистрибутива):

```sh
chmod +x run.sh
./run.sh --build      # собрать
./run.sh --rebuild    # пересобрать
./run.sh              # собрать и запустить
```

#### Debian / Ubuntu

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **g++** (или `build-essential`), **make**, **pkg-config**.

```sh
sudo apt install cmake build-essential git pkg-config zlib1g-dev libglfw3-dev libglfw3 libglew-dev libopenal-dev \
  libluajit-5.1-dev libvorbis-dev libcurl4-openssl-dev libfmt-dev \
  libspdlog-dev libgtest-dev

# LuaJIT: Debian/Ubuntu не кладут заголовки в стандартный путь
sudo ln -sf /usr/lib/x86_64-linux-gnu/libluajit-5.1.a /usr/lib/x86_64-linux-gnu/liblua5.1.a
sudo ln -sf /usr/include/luajit-2.1 /usr/include/lua

# EnTT (Устанавливаем последнюю версию, поддерживающую C++17)
git clone --branch v3.16.0 https://github.com/skypjack/entt.git
cd entt/build
cmake -DCMAKE_BUILD_TYPE=Release -DENTT_INSTALL=ON ..
sudo make install
```

#### Fedora / RHEL

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **gcc-c++**, **make**, **pkg-config**.

```sh
sudo dnf install cmake gcc-c++ make git glfw-devel glew-devel openal-soft-devel luajit-devel \
  libvorbis-devel libcurl-devel fmt-devel spdlog-devel gtest-devel zlib-devel

# EnTT (Устанавливаем последнюю версию, поддерживающую C++17)
git clone --branch v3.16.0 https://github.com/skypjack/entt.git
cd entt/build
cmake -DCMAKE_BUILD_TYPE=Release -DENTT_INSTALL=ON ..
sudo make install
```

#### Arch Linux

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **base-devel** (gcc, make), **pkg-config**.

```sh
# X11
sudo pacman -S cmake base-devel git glfw-x11 glew openal-soft luajit libvorbis curl fmt spdlog gtest
```

```sh
# Wayland
sudo pacman -S cmake base-devel git glfw-wayland glew openal-soft luajit libvorbis curl fmt spdlog gtest
```

```sh
# EnTT (AUR)
yay -S entt
```

Перед установкой обновите базу пакетов: `sudo pacman -Syu`.

#### ALT Linux

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **gcc**, **make**, **pkg-config**.

```sh
su -
apt-get install cmake gcc make git entt-devel libglfw3-devel libGLEW-devel libopenal-devel \
  libluajit-devel libvorbis-devel libcurl-devel libfmt-devel libspdlog-devel libgtest-devel zlib-devel
```

> [!NOTE]
> Названия пакетов `fmt`, `spdlog`, `gtest` могут различаться в зависимости от версии ALT Linux
> (например `libfmt-devel`, `libspdlog-devel`, `libgtest-devel`).
> Проверьте доступные имена: `apt-cache search fmt spdlog gtest`.
<!-- -->
> [!WARNING]
> На **ALT Linux** НЕ используйте ручную установку EnTT из исходников — только `entt-devel` из репозитория.

### macOS (Homebrew)

> [!NOTE]
> Требуется: **CMake** (≥ 3.26), **Git**, **Homebrew**.

```sh
brew install glfw glew openal-soft luajit libvorbis skypjack/entt/entt googletest fmt spdlog
```

> [!TIP]
> Если Homebrew не может установить `luajit` или `openal-soft`, скачайте и соберите их вручную.

```sh
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DOPENAL_INCLUDE_DIR="$(brew --prefix openal-soft)/include" \
  -DOPENAL_LIBRARY="$(brew --prefix openal-soft)/lib/libopenal.dylib" \
  -DChromaForge_BUILD_TESTS=ON
cmake --build build --parallel
```

### Docker

> [!NOTE]
> Требуется: **Docker Engine**.

```sh
git clone https://github.com/Ezhovnik/ChromaForge-v2.git
cd ChromaForge-v2

# Шаг 1. Собрать образ
docker build -t chromaforge .

# Шаг 2. Собрать проект внутри контейнера
docker run --rm -it --user root -v "$(pwd):/project" chromaforge \
  bash -c "cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && cmake --build build --parallel"
```

Запуск графического приложения из контейнера (требуется X11):

```sh
# Linux
docker run --rm -it \
  -v "$(pwd):/project" \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$XAUTHORITY:/home/user/.Xauthority:ro" \
  -e DISPLAY="$DISPLAY" \
  --network=host \
  chromaforge ./build/ChromaForge
```

> [!TIP]
> На Windows для запуска GUI из Docker понадобится X-сервер (например, **VcXsrv**):
> ```powershell
> .\vcxsrv.exe :0 -multiwindow -ac
> docker run --rm -it -v "${PWD}:/project" -e DISPLAY=host.docker.internal:0 chromaforge ./build/ChromaForge
> ```
<!-- -->
> [!NOTE]
> Для сборки AppImage используется флаг `-DChromaForge_BUILD_APPDIR=ON`.
> Подробнее — в [`appimage.yml`](.github/workflows/appimage.yml).
>
> Запуск под Windows требует X-сервер (например, **VcXsrv**), а также проброс Xauthority отсутствует — используйте Linux-хост для GUI.

## Тестирование

> [!NOTE]
> Все тесты собираются только если включён флаг `ChromaForge_BUILD_TESTS=ON` *(включён по умолчанию в Windows-пресетах; для Linux передайте флаг `-DChromaForge_BUILD_TESTS=ON` вручную)*.

### Модульные тесты (C++ / Google Test)

```sh
cmake --build build --target ChromaForgeTest
ctest --test-dir build --output-on-failure
```

### Интеграционные тесты (Lua)

Запускают Lua-скрипты из `dev/tests/` через `--headless` режим движка:

```sh
# Сборка раннера
cmake --build build --target vctest

# Запуск
chmod +x build/vctest/vctest build/ChromaForge   # только Linux/macOS
build/vctest/vctest -e build/ChromaForge -d dev/tests -u build
```

| Флаг | Описание |
| ---- | -------- |
| `-e <path>` / `--exe <path>` | путь к исполняемому файлу ChromaForge |
| `-d <path>` / `--tests <path>` | директория с Lua-тестами (`dev/tests`) |
| `-r <path>` / `--res <path>` | путь к папке с ресурсами (по умолчанию `res`) |
| `-u <path>` / `--user <path>` | рабочая директория (создаётся `.vctest/` внутри) |
| `--output-always` | всегда показывать вывод тестов |
