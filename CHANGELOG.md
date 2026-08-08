# Changelog

> [!NOTE]
> Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/).

## v0.4.1

### Скриптинг (Lua)

- События: `on_entity_spawned(entityid)` / `on_entity_despawned(entityid)`.
- Скрипт-слушатель при загрузке паков (`scripts-listener`), вызов скриптов пака при его загрузке.
- `new_engine_instance` — аргумент файла вывода.
- Фиксы: `lua::toquat`, `process_properties`, валидация типов в `audio.play`, `app.reconfig_packs`.

### Геймплей

- Фиксы: движение игрока (рефакторинг контролов, исправление цикла телепортов, прикрепление к опоре), проверка AABB сущностей при размещении блоков, падение курсора.

### Графика и рендеринг

- Рефакторинг владения конфигурацией скелетов (shared_ptr); фикс масштабирования 3D-текста; исправление сохранения превью мира; фикс отображения скелетов в `preload.json`.

### Ассеты и контент-паки

- Скрипт-слушатель паков (см. «Скриптинг»); фикс загрузки пака `preload.json` (скелеты).

### Прочее

- Обработка ошибок в `gui_util`-функциях; фиксы локале-зависимых функций `stringutil`; фиксы документации.

## v0.4.0

### Движок и инфраструктура

- Режимы запуска:
  - headless-режим, `--server`, `--version`; `TestMainloop` → `ServerMainloop`, `test.quit()` / `quitSignal`;
  - `--sps <n>` — частота тиков (sparks per second) в headless-режиме;
  - `--stdin-cmd` — выполнение команд из stdin;
  - флаг cheat для консольных команд (`console.set_cheat`, правило `cheat-commands` в `librules`).
- Рефакторинги: Engine, EnginePaths, Window и события, логирование (logger), `THROW_ERR`-макросы, `libbuiltin` → `libapp`.
- `new_engine_instance()`, `get_executable_path()`, `logPath`, `app.focus()`.
- Проекты: файл `project.toml`, права (permissions) проекта, стартовый скрипт проекта (`add project start script`).
- Версионирование: переименование «maintenance → patch» в схеме выпуска релизов.

### Мир и чанки

- Рефакторинг хранения: данные чанков мигрированы в `VoxelsVolume`, `ChunksStorage` → `GlobalChunks`, Chunks перенесены в игрока, `level.chunksStorage` → `level.chunks`, `ChunksStorage.getVoxels` → `Chunks.getVoxels`.
- Взаимодействия с блоками перенесены в хранилище чанков; введён «блок-агент» (blocks agent).
- `compressed_chunks` — сжатые данные чанков; сериализация чанков в Lua: `world.get_chunk_data` / `world.set_chunk_data` / `world.save_chunk_data` (запись в регион), `world.count_chunks()`.
- Версии мира + событие `on_replaced`; миры без имени; меню миров; плейсхолдер имени мира.
- Опциональный lightmap чанков (`chunk lightmap optional`), `LightMap` → `Lightmap`.
- События чанков и блоков: `on_chunk_present(x, z, loaded)` / `on_chunk_remove(x, z)`, `on_block_present(x, y, z)`, `on_block_removed(x, y, z)`, `on_block_spark(x, y, z, sps)` / `on_blocks_spark(sps)`, `on_player_spark(playerid, sps)`, `on_inventory_open` / `on_inventory_closed`, `on_replaced`.
- События мира (`scripts/world.lua`): `on_world_open/save/spark/quit`, `on_block_placed/replaced/broken/interact`.

### Геймплей

- Физика:
  - переработана физика движения относительно поверхности (ground-relative);
  - у твёрдых тел (Rigidbody) появился **материал** (`get_material` / `set_material`, свойство сущности `material`);
  - уточнено сопротивление воздуха (air-damping).
- Игрок (`libplayer`):
  - `player.create` / `player.delete`, `set_suspended` / `is_suspended` (заморозка игрока удаляет его сущность), `set_name` / `get_name`;
  - `set_interaction_distance` / `get_interaction_distance`;
  - режимы: flight, noclip, infinite items, instant destruction, loading chunks; spawnpoint; выбранный слот и блок (`get_selected_block`), `get_selected_entity`;
  - камеры: `get_camera` / `set_camera`;
  - поиск: `get_all()`, `get_all_in_radius()`, `get_nearest()`; интерполяция движения игроков, `localPlayer`; тики игрока в headless-режиме.
- Сущности (ECS):
  - рефакторинг сущностей; спавн с аргументами компонентов `entities.spawn(name, pos, args)`;
  - встроенные компоненты `builtin:player`, `builtin:mob`, `builtin:pathfinding`;
  - компоненты: Transform, Rigidbody (gravity scale, linear damping, body type dynamic/kinematic, elasticity, mass), Skeleton (модели, динамические текстуры `$имя`, видимость костей), сенсоры (`on_sensor_enter/exit`), `on_grounded`, `on_fall`, `on_attacked`, `on_used`, `on_aim_on/off`;
  - скелет руки от первого лица (`gfx.skeletons` `'hand'`, `hud.hand_controller`);
  - скелеты стали ассетами, а не контент-юнитами (папка `skeletons/`, свойство `skeleton-name`).
- Инвентарь:
  - исправление открытия инвентаря; `hud.open_block(x, y, z)`, `hud.get_block_inventory()`, `hud.get_second_inventory()`;
  - `inventory.move` / `move_range` (эффективнее get/set), `decrement`, `find_by_item`, `use` (уменьшает счётчик `uses`), `clone`, `remove`;
  - локальные данные предметов: `has_data` / `get_data` / `set_data` / `get_all_data`;
  - заголовки и описания: `get_caption` / `set_caption`, `get_description` / `set_description`;
  - `infiniteItems` (флаг `set_infinite_items`).
- Предметы: использование (`uses`, `uses-display`: none/number/relation/vbar), описание предметов (description, поддерживает md), `model-name`, `placing-block`, `emission`, `icon`, `stack-size`.
- Блоки (Lua API):
  - варианты блоков: `block.get_variant` / `block.set_variant`, свойство `state-based` (`offset`, `bits` до 4 — до 16 вариантов), вариантная геометрия/текстуры/модели; `block.model_name(id, variant)`, `block.get_model`, `block.get_textures`;
  - рейкаст `block.raycast` с фильтром прозрачных блоков и учётом невыбираемых блоков;
  - вращение: `get_rotation` / `set_rotation`, профили вращения `none` / `pipe` / `pane` / `stairs` (в т.ч. профиль ступеней), `get_rotation_profile`;
  - расширенные блоки (>1x1x1): `is_extended`, `get_size`, `is_segment`, `seek_origin`, свойство `size`;
  - пользовательские биты (`get_user_bits` / `set_user_bits`, 8 бит в `voxel.states`);
  - поля блока: `set_field` / `get_field`, типы int8…float64/char, `convert-strategy` (reset/clamp), ≤240 байт на блок;
  - `get_hitbox`, `compose_state` / `decompose_state`, `block.place` / `destruct`;
  - `block.properties` (пользовательские свойства из `config/user-props.toml`), `tags` (`tags.toml`, `has_tag`), `block.materials`.
- Свойства блоков (пак-контент): `translucent`, `emission`, `light-passing`, `sky-light-passing`, `shadeless`, `ambient-occlusion`, `culling` (default/optional/disabled + `*_opaque`), `obstacle`, `hitbox`, `grounded`, `selectable`, `replaceable`, `breakable`, `grounding-behaviour` (partial/complete/origin), `particles`, `draw-group`, `spark-interval`.
- Контент chromaforge: `chromaforge:durability`, `chromaforge:loot` (+ `block_loot()` в модуле `chromaforge:util`), цветные блоки, новые цветы, берёзовые листья и листва, неоновые блоки, обновление текстур ламп; переименование moss → grass_block.
- Чат: внутриигровой чат и хоткей чата.

### Генерация мира

- Новые параметры генератора: `caption`, `biome-parameters` (0–4), `sea-level`, `biomes-bpd`, `heights-bpd`, `wide-structs-chunks-radius`, `heightmap-inputs`, `player-spawn-radius`, `player-min-spawn-height`, `player-max-spawn-height`.
- Фрагменты: `generation.create_fragment`, `load_fragment`, `save_fragment`, `fragment:place` (с поворотом), `fragment:crop`, команда `fragment.save`.
- Размещения структур: `{имя, позиция, поворот, приоритет}`, `":line"` (линии с радиусом), `":block"` (отдельные блоки с поворотом и приоритетом) — добавлен тип `:block`;
  `builtin:struct_air` (структурный воздух).
- Структуры: свойство `lowering` (погружение под поверхность).
- Heightmap: класс `Heightmap(w, h)`, методы `abs/add/sub/mul/pow/min/max/mixin`, `noise` (симплекс, октавы), `cellnoise`, `resize` (nearest/linear/cubic), `crop`, `at`, `dump` (отладка).
- Функции: `generate_heightmap`, `generate_biome_parameters`, `place_structures`, `place_structures_wide`.
- Генератор `chromaforge:standart`: руда через `standart.files/ores.json` (`{"struct": ..., "rarity": N}`) + исправление генерации руды.

### Скриптинг (Lua)

- Язык — LuaJIT. Модули: `builtin:bit_converter` (LE/BE, int8…int64, float32/64), `builtin:data_buffer` (сериализация с типизированным `put_number`), `builtin:vector2` / `builtin:vector3` (перегруженные операторы), `builtin:internal/scripts_registry`.
- Расширения стандартных библиотек (stdmin): `string.explode/split/replace/trim/...`, `table.copy/deep_copy/...`, `math.clamp/rand/normalize/round`, `await`, `sleep`, `os.pid`, UTF-8 `lower/upper`.
- Библиотека событий: `events.on/reset/emit/remove_by_prefix`.
- Новые библиотеки:
  - `libsession` — хранение данных между выгрузками контента (`session.get/reset/has`);
  - `libcompression` — сжатие gzip (`compression.encode/decode`);
  - `libbyteutil` — упаковка байт по строке формата (`pack/tpack/unpack`, порядки байт);
  - `libbase64` — `encode/decode` + urlsafe-варианты;
  - `librandom` — `random.random/bytes/uuid`, класс `Random` с изолированным состоянием;
  - `libutf8` — `tobytes/tostring/length/codepoint/escape/...`;
  - `libvecn` (vec2/3/4), `libmat4` (матрицы 4x4, `decompose`, `look_at`, `perspective`), `libquat` (кватернионы, `slerp`, `from_euler`);
  - `libtime` — `uptime/delta/utc_time/local_time/utc_offset`;
  - `librules` — правила-флаги (`create/listen/get/set/reset`), стандартные правила `cheat-commands`, `allow-flight`, `allow-destroy` и др.;
  - `libpack` — управление паками (`is_installed`, `assemble`, `request_writeable`, `data_file`, `shared_file`, `get_info`);
  - `libfile` — см. «Файловая система» ниже;
  - `libcameras`, `libentities`, `libitem`, `libinventory`, `libhud`, `libgui`, `libinput`, `libworld`, `libblock`, `libassets`, `libnetwork`;
  - `libgfx-posteffects`, `libgfx-skeletons`, `libgfx-weather`, `libgfx-blockwraps`, `libtext3d`, `libparticles`, `console`.
- Игрок: функции см. «Геймплей»; `libplayer` пополнена `get_dir`, `get_inventory`, `set_selected_slot`, `get_camera/set_camera`, `set_loading_chunks`.
- Мир (`libworld`): `get_day_time` / `set_day_time` / `set_day_time_speed`, `is_day` / `is_night`, `get_seed`, `get_generator`, `get_list`, `exists`, `is_open`.
- HUD: `hud.is_open()`, `hud.set_allow_pause(flag)`, `hud.open_permanent(layoutid)`, `hud.show_overlay(layoutid, playerinv, args)`, `hud.open(layoutid, disablePlayerInventory, invid)`, `hud.is_inventory_open()`, `hud.pause/resume/is_paused`, `hud.hand_controller`.
- GUI: `gui.show_message` / `gui.ask` (диалоги; `gui.alert` / `gui.confirm` помечены устаревшими), `gui.create_frame` / `get_active_frame` / `set_active_frame` (рендер UI в текстуру), `gui.close_menu`, `gui.load_document`, `gui.template`, `gui.root`, `gui.str`, `gui.get_viewport`, `gui.get_locales_info`, `gui.clear_markup` / `gui.escape_markup`.
- Ввод: `input.add_callback(bindname, callback, owner, istoplevel)` (в т.ч. `"key:space"`), `is_pressed`, `get_bindings`, `get_binding_text`, `is_active`, `set_enabled`, `get_mouse_pos`, `get_mouse_delta`, `keycode`, `mousecode`.
- Bytearray: апгрейд, FFI-интерфейс, побитовые операции, `Bytearray_as_string`.
- Canvas: FFI, `create_texture` / `unbind_texture`, `encode` / статический `Canvas.decode` (PNG), `blit`, `mul/add/sub` (цвет или Canvas), `line/rect/at/set/clear/update`, канвас из атласа текстур, `image.region`.
- Сериализация: `json.tostring/parse`, `toml.tostring/parse`, `yaml.tostring/parse`, `bjson.tobytes/frombytes` (vcbjson, сжатие); YAML-парсер и обновление XML.
- Парсер hex-цветов (`#RRGGBB` / `#RRGGBBAA`).
- Скрипты: классификация (hud/world/modules/компоненты), перезагрузка скриптов hud/мира/модулей/компонентов, `scripts_registry`, меню скриптов + кнопка обновления, сохранение редактируемых файлов, фильтр файлов, стартовый скрипт проекта.

### Файловая система и I/O

- `libfile`: `resolve/read/write/length/exists/isfile/isdir/is_writeable/mkdir/mkdirs/list/list_all_res/find/remove/remove_tree`, `read_combined_list`, `read_combined_object`, `name/stem/ext/prefix/parent/path/join`.
- Точки входа и архивы: `file.mount` (ZIP), `file.unmount`, `file.create_zip`, `file.create_memory_device()` + `app.create_memory_device(name)` (ФС в памяти, удаляется при выгрузке контента).
- Стриминговый I/O: `file.open(path, mode)` → `io_stream` (режимы r/w/b/+, binary/text, `"yield"` / `"buffered"`, `read_fully`, `seek`, `flush`), именованные каналы `file.open_named_pipe`.
- `file.write_bytes` — исправление; `world/files` папка; замена `std::filesystem::path` на `io::path` (WIP).
- `Device::read` / `Device::write` — обновлены.

### Сеть

- `libnetwork` на libcurl:
  - HTTP: `network.get`, `network.get_binary`, `network.post` (JSON), заголовки, `onfailure`-колбэк, неблокирующий `httpGet`;
  - TCP: `tcp_connect` / `tcp_open`, `recv` / `recv_async` (корутины), `errorCallback`, `available`, `is_alive`, `get_address`, `set_nodelay`, лимит одновременных клиентов в сервере;
  - UDP: `udp_connect` / `udp_open`, дейтаграммы;
  - `get_total_upload` / `get_total_download`, `find_free_port`, `network.__connect` / `__close`, тесты сети (localhost).
- Сервер отладки (debugging server), который больше не требует сети; брейкпоинты и другие возможности отладки; спецификация `doc/ru/specs/debugging_protocol.md`.
- `open_url` — открытие ссылки в браузере.

### Графика и рендеринг

- Освещение: мягкое освещение (soft-lighting), тени в быстром графическом конвейере, `deffered_lights`, улучшение теней, отладка источников света.
- Рендер мира:
  - dense-рендер (`chunkMaxVerticesDense`, настройка `graphics.dense-render`, culling `optional` + текстуры `*_opaque`) и оптимизация по дистанции;
  - несколько индексных буферов на меш, `ChunkMesh`, рефакторинг глобального меша и render pipeline;
  - рефакторинг и оптимизация WorldRenderer / BlocksRenderer; перенос данных чанков в `VoxelsVolume`;
  - удалён `GLTexture`; невидимый скайбокс за полупрозрачными блоками — исправлено.
- Облака: рендерер облаков (CloudsRenderer), настройка качества `clouds-quality`, шейдер облаков.
- Пост-эффекты: слот эффектов (`post-effect-slot` в `resources.json`), `gfx.posteffects.set/get/...`, массивные параметры (`set_array`), интенсивность, `set_params`.
- Шейдеры: рефакторинг, `ShaderProgram.recompile`, автоперекомпиляция при изменении.
- Атласы: сборщик палитровых атласов (palette atlas builder), атлас из текстуры, канвас из атласа, типы атласов (`blocks`, `items`).
- Модели `.cfmodel`: примитив `tri` (треугольник), `region` / `region-scale` (UV-регионы), `rotate` (градусы или кватернион), `part` (стороны box), кости-скелет (`@bone`, `name/move/scale/rotate`, корневая кость `root`, `"squash": true` в preload), поворот моделей (cfmodel rotation), документация.
- Шрифты: векторные шрифты (FreeType, `coders/vector_fonts`), 3D-текст (`libtext3d`, `gfx.text3d`: билборды, `xray_opacity`, `set_rotation`), обновление шрифтов, FontStyles, стили текста (md: жирный/курсив/подчёркнутый/зачёркнутый, цвета `[#RRGGBB]`).
- UI-рендер в текстуру.
- Скриншоты — исправление; новая иконка движка и windows-icon; текстура солнца — обновлена.
- Окно: borderless-режим, рефакторинг окна и событий, `glfwWaitEvents` в меню, `display.adaptive-menu-fps`, информация о content scale в лог, правило `allow-zoom`, обновление камер.
- Отладка графики: GL debug output, исправление спама GL.
- Свойства блоков: `translucent` (полупрозрачные блоки), опциональный lightmap чанков.

### Аудио

- Акустика: реверберация через трассировку лучей (Decorator + EFX), настройка `acoustic-effects`, параметры reverb (decay, gain, задержки, rolloff, absorption).
- Параметр материала блока `sound-absorption` (влияет на акустику).
- Бекенды: `NoAudio` (заглушка) и `ALAudio` (OpenAL); каналы master/ui/regular/ambient/music.
- PCMStream: `audio.PCMStream(sample_rate, channels, bits_per_sample)` с `feed`, `share` (публикация для `play_stream`) и `create_sound`; фикс остановки потока при underflow.
- Запись звука: `audio.input.request_open(callback)` / `audio.input.fetch(token, [max_read_size])`, выбор микрофона в настройках аудио, вариант «нет микрофона», поддержка нескольких аудио-фетчеров.
- Звуки ударов по блокам (hits sounds); `NotePreset`.

### Погода и частицы

- `libgfx-weather`: пресеты погоды (`presets/weather/`), `gfx.weather.change(data, time, name)` с плавным переходом, `get_current`, `get_current_data`, `get_fall_intensity`, `is_transition`.
- Настройки погоды: `fall` (осадки: `texture`, `noise`, `vspeed`, `hspeed`, `scale`, `min/max_opacity`, `max_intensity`, `opaque`, `splash`), `clouds`, `fog_opacity`, `fog_dencity`, `fog_curve`, `thunder_rate`, `sky_tint`, `clouds_tint`, `min_sky_light`.
- Дождь (шум и брызги), снег, гром; `WeatherPreset`; новые пресеты погоды.
- Частицы (`gfx.particles`): `emit(origin, count, preset, extension)` (origin может быть uid сущности, count = -1 — бесконечно), `stop/is_alive/get_origin/set_origin`, формы спавна ball/sphere/box, обновление системы частиц.

### UI (XML-разметка)

- Новые элементы: `splitbox` (`split-pos`, двойной клик по разделителю), `iframe` (встраивание документа), `canvas`, `select` (+ `option`), `trackbar` (в т.ч. `change-on-release`), `bindbox`, `slots-grid`, `scrollbar`, `pagebox`, `files_panel.xml`.
- TextBox: подсветка синтаксиса (`syntax="lua"`), разметка (`markup="md"`), история, `hint`, `keep-line-selection`, `error-color`, `text-color`, `validator`, `onup`/`ondown`, `line-numbers`, новые методы (`paste`, `lineAt`, `linePos`), фикс смещения строк.
- GUI debug mode; доступ к под-узлам документа; свойство `exists`; исправлен порядок загрузки InlineFrame; улучшен polling окна.
- События мыши: `onmouseover` / `onmouseout` (+ `onmouseenter` / `onmouseleave`), боковые кнопки мыши, обновление mouse events.
- Меню и консоль: меню миров, меню скриптов (с кнопкой обновления), поиск в контент-меню, рефакторинг меню и консоли, раскладка консоли, сохранение редактируемых файлов, фильтр файлов, курсор.
- `hud.show_overlay` — параметр `args`; `keep prev values in debug panel`.

### Ассеты и контент-паки

- `AssetsManagement`: менеджер ассетов, фоновый загрузчик (`assets.request_texture`), `assets.load_texture` (из байтов, PNG), `assets.parse_model` (xml/cfmodel), `assets.to_canvas`.
- Скелеты как ассеты (не контент-юниты); `preload.json` с `"keep-pcm"` и `"squash"`.
- Управление источниками контента: `app.get_content_sources` / `set_content_sources` / `reset_content_sources` (порядок: world → user → project → res), `app.reconfig_packs` / `config_packs` (с проверкой зависимостей), `app.is_content_loaded()`, `non_reset_packs`.
- Рефакторинги: `ContentLoader`, `ContentUnitIndices`, `ContentGfxCache`; `getContentPacks` → `getAllContentPacks`; ObjectsPool; threadpool upd; weak ptrs.
- Библиотека `pack`: `get_info` (id/title/creator/version/icon/dependencies), `assemble`, `request_writeable`, `data_file`, `shared_file`.
- `skeletons` в контент-паке, `chromaforge pack upd`, `update non-reset-packs`.

### Сборка, тесты и CI

- Рефакторинг системы сборки, добавлены CI/CD (GitHub Actions), тесты и Docker.
- CMake-пресеты, интеграция vcpkg, обновление windows-clang, `.vscode` конфиг.
- Фиксы сборки: MSVC / Linux / macOS (недостающие `<sstream>`, case-sensitive Lightmap.h, gai_strerror, `gai_strerrorA`), MinGW+Clang (LLD и флаг `-Wl,-E` → `--export-all-symbols`, обход clang-драйвера при линковке).
- Фиксы платформ: ARM (unaligned access в SmallHeap → SIGBUS, `read_int_le` ptrdiff_t), `power()` с отрицательной степенью, UB в инициализации `memory_istream` (+ seekoff/seekpos).
- AppImage-сборка (Linux) и её фиксы (пакеты spdlog/fmt, AppDir.path, mkdir для applications/icons).
- Тесты: `libtest`, `vctest` (work in progress), тесты чанков/StructLayout/сети, фикс имён в тестах; `test.quit()` / `quitSignal`.
- README — обновления (платформы сборки, бейджи CI).

### Документация

- `doc/ru/specs`: спецификации `binary_json` (cfbjson), `debugging_protocol`, `fragment_file`, `region_file`, `region_voxels_chunk`, `vec3_format`; устаревшие версии перенесены в `specs/outdated`.
- Новая документация: cfmodel, модели блоков, риггинг, свойства блоков/предметов/сущностей, контент-паки, генератор мира, консоль, аудио, 3D-текст, частицы, стили текста, XML UI layouts, предзагрузка ассетов, ресурсы (resources.json).
- Скриптинг: главная страница, события, сущности и компоненты, встроенные компоненты, расширения stdlib, файловая система, io_stream, UI, пользовательский ввод; документация всех `lib*` (в т.ч. libpathfinding, libassets, libsession).
