# Файл фрагмента (.vox)

Текущая версия: 1.

Файл фрагмента — это [файл vcbjson](binary_json_spec.md).

Представление схемы JSON на языке [Orderly](https://orderly-json.org/docs/):

```orderly
object {
    integer version {1,};
    array {integer;} {1,} size;
    array {string;} block-names;
    array {integer;} {0,65535} voxels;
}
```

Где:

- **version** — версия формата файла фрагмента.
- **size** — размер фрагмента (три положительных целых числа).
- **block-names** — массив полных имён блоков: `pack_id:block_name`.
- **voxels**: массив данных вокселей: *index, state, index, state...*
  где:
  - index основан на массиве block-names.
  - state — см. [состояние блока](region_voxels_chunk_spec.md#состояние-блока).

**block-names** должен содержать `builtin:air` первым элементом.
