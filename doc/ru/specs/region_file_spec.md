# Файл региона (версия 3)

Формат файла в BNF (RFC 5234):

```bnf
file    = header (*chunk) offsets   полный файл
header  = magic %x02 byte           магическое число, версия и метод
                                    сжатия

magic   = %x2E %x43 %x48 %x52 %x4F  '.CHROMAREG\0'
          %x4D %x41 %x52 %x45 %x47 %x00

chunk   = uint32 uint32 (*byte)     массив байтов с префиксом размера и
                                    исходного размера, где исходный
                                    размер — размер распакованных данных
                                    чанка

offsets = (1024*uint32)             таблица смещений
int32   = 4byte                     беззнаковое 32-битное целое (big-endian)
byte    = %x00-FF                   8-битное беззнаковое целое
```

Визуализация в виде структуры C:

```c
typedef unsigned char byte;

struct file {
    // 10 байт
    struct {
        char magic[11] = ".CHROMAREG";
        byte version = 3;
        byte compression;
    } header;

    struct {
        uint32_t size; // порядок байтов: little-endian
        uint32_t sourceSize; // порядок байтов: little-endian
        byte* data;
    } chunks[1024]; // файл не содержит нулевых размеров для отсутствующих чанков

    uint32_t offsets[1024]; // порядок байтов: little-endian
};
```

Таблица смещений содержит позиции чанков в файле. 0 означает, что чанк отсутствует в файле. Минимальное допустимое смещение — 10 (размер заголовка).

Доступные методы сжатия:

0. без сжатия
1. extRLE8
2. extRLE16
