# Файл региона (версия 2)

Формат файла в BNF (RFC 5234):

```bnf
file    = header (*chunk) offsets   полный файл
header  = magic %x02 %x00           магическое число, версия и
                                    зарезервированный нулевой байт

magic   = %x2E %x43 %x48 %x52 %x4F  '.CHROMAREG\0'
          %x4D %x41 %x52 %x45 %x47 %x00

chunk   = int32 (*byte)             массив байтов с префиксом размера
offsets = (1024*int32)              таблица смещений
int32   = 4byte                     знаковое 32-битное целое (big-endian)
byte    = %x00-FF                   8-биттовое беззнаковое число
```

Визуализация в виде структуры C:

```c
typedef unsigned char byte;

struct file {
    // 10 байт
    struct {
        char magic[11] = ".CHROMAREG";
        byte version = 2;
        byte reserved = 0;
    } header;

    struct {
        int32_t size; // порядок байтов: big-endian
        byte* data;
    } chunks[1024]; // файл не содержит нулевых размеров для отсутствующих чанков

    int32_t offsets[1024]; // порядок байтов: big-endian
};
```

Таблица смещений содержит позиции чанков в файле. 0 означает, что чанк отсутствует в файле. Минимальное допустимое смещение — 10 (размер заголовка).
