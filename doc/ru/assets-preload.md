# Предзагрузка ассетов (файл *preload.json*)

Для загрузки ассетов, не загружаемых автоматически, такие как звуки, дополнительные текстуры, используется файл `preload.json`, создающийся в папке контент-пака.

Ассеты в файле разделяются на категории:

- atlases - атласы
- fonts - шрифты
- shaders - шейдеры
- textures - текстуры
- sounds - звуки
- models - модели

> [!NOTE]
> При загрузке звука подгружаются все его вариации, по шаблону:
> (звук: sound_name) -> *sound_name.ogg, sound_name_1.ogg, sound_name_2.ogg, ...*
> или *sound_name_0.ogg, sound_name_1.ogg, sound_name_2.ogg, ...*

Добавление звука `пак:sounds/events/explosion.ogg` со всеми его вариантами:

```json
{
    "sounds": [
        "events/explosion"
    ]
}
```

Будет доступен под именем: "events/explosion"

В случае, если нужно будет работать с PCM данными звука (сейчас не доступно из скриптинга), требуется указать параметр `keep-pcm`:

```json
{
    "sounds": [
        {
            "name": "events/explosion",
            "keep-pcm": true
        }
    ]
}
```

Пример файла из пакета `builtin:` (`res/preload.json`):

```json
{
    "shaders": [
        "ui",
        "default",
        "lines"
    ],
    "textures": [
        "gui/menubg",
        "gui/delete_icon",
        "gui/no_icon",
        "gui/no_world_icon",
        "gui/warning",
        "gui/error",
        "gui/cross",
        "gui/refresh"
    ],
    "fonts": [
        {
            "name": "normal",
            "path": "fonts/font"
        }
    ],
    "atlases": [
        "blocks",
        "items"
    ]
}
```
