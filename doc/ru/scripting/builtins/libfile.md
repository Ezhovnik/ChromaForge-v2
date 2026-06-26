# Библиотека *file*

Библиотека функций для работы с файлами

```lua
file.resolve(путь: str) -> str
```

Функция приводит запись `точка_входа:путь` (например `user:saves/house1`) к обычному пути. (например `C://Users/user/ChromaForge/saves/house1`)

> [!NOTE]
> Функцию не нужно использовать в сочетании с другими функциями из библиотеки, так как они делают это автоматически

Возвращаемый путь не является каноническим и может быть как абсолютным, так и относительным.

```lua
file.read(путь: str) -> str
```

Читает весь текстовый файл и возвращает в виде строки

```lua
file.read_bytes(путь: str) -> array of integers
```

Читает файл в массив байт.

```lua
file.is_writeable(путь: str) -> bool
```

Проверяет, доступно ли право записи по указанному пути.

```lua
file.write(путь: str, текст: str) -> nil
```

Записывает текст в файл (с перезаписью)

```lua
file.write_bytes(путь: str, data: array of integers)
```

Записывает массив байт в файл (с перезаписью)

```lua
file.length(путь: str) -> int
```

Возвращает размер файла в байтах, либо -1, если файл не найден

```lua
file.exists(путь: str) -> bool
```

Проверяет, существует ли по данному пути файл или директория

```lua
file.isfile(путь: str) -> bool
```

Проверяет, существует ли по данному пути файл

```lua
file.isdir(путь: str) -> bool
```

Проверяет, существует ли по данному пути директория

```lua
file.mkdir(путь: str) -> bool
```

Создает директорию. Возвращает true если была создана новая директория

```lua
file.mkdirs(путь: str) -> bool
```

Создает всю цепочку директорий. Возвращает true если были созданы директории.

```lua
file.list(путь: str) -> массив строк
```

Возвращает список файлов и директорий в указанной.

```lua
file.list_all_res(путь: str) -> массив строк
```

Возвращает список файлов и директорий в указанной без указания конкретной точки входа.

```lua
file.find(путь: str) -> str
```

Ищет файл от последнего пака до res. Путь указывается без префикса. Возвращает путь с нужным префиксом. Если файл не найден, возвращает nil.

```lua
file.remove(путь: str) -> bool
```

Удаляет файл. Возращает **true** если файл существовал. Бросает исключение при нарушении доступа.

```lua
file.remove_tree(путь: str) -> int
```

Рекурсивно удаляет файлы. Возвращает число удаленных файлов.

```lua
file.read_combined_list(путь: str) -> массив
```

Совмещает массивы из JSON файлов разных паков.

```lua
file.read_combined_object(путь: str) -> массив
```

Совмещает объекты из JSON файлов разных паков.

```lua
file.name(путь: str) --> str
```

Извлекает имя файла из пути. Пример: `world:data/chromaforge/config.toml` -> `config.toml`.

```lua
file.stem(путь: str) --> str
```

Извлекает имя файла из пути, удаляя расширение. Пример: `world:data/chromaforge/config.toml` -> `config`.

```lua
file.ext(путь: str) --> str
```

Извлекает расширение из пути. Пример: `world:data/chromaforge/config.toml` -> `toml`.

```lua
file.prefix(путь: str) --> str
```

Извлекает точку входа (префикс) из пути. Пример: `world:data/chromaforge/config.toml` -> `world`.

```lua
file.parent(путь: str) --> str
```

Возвращает путь на уровень выше. Пример: `world:data/chromaforge/config.toml` -> `world:data/chromaforge`

```lua
file.path(путь: str) --> str
```

Убирает точку входа (префикс) из пути. Пример: `world:data/chromaforge/config.toml` -> `data/chromaforge/config.toml`

```lua
file.join(директория: str, путь: str) --> str
```

Соединяет путь. Пример: `file.join("world:data", "chromaforge/config.toml)` -> `world:data/chromaforge/config.toml`.

Следует использовать данную функцию вместо конкатенации с `/`, так как `префикс:/путь` не является валидным.
