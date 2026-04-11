# C-utils

Header-only C utilities for day-to-day systems and tooling development.

This project is a personal utility collection focused on:

- zero library binaries to link
- simple `#include`-based usage
- CMake-friendly integration
- small, practical tooling

## Current modules

### `ds/dynarr.h`

Macro-based dynamic array utilities:

- `arr_init(type)`
- `arr_push(arr, value)`
- `arr_pop(arr)`
- `arr_remove(arr, index)`
- `arr_len(arr)`
- `arr_free(arr)`

### `ds/hashtable.h`

Header-only string-key hash table:

- open addressing with linear probing
- tombstone handling
- automatic resizing
- generic `void*` values

### `json/*`

Minimal JSON value/object/codec layer built on the project data structures:

- object storage backed by `HashTable`
- array storage backed by `dynarr`
- parse (`json_parse`) and dump (`json_dump`)

## Requirements

- CMake >= 3.20
- C compiler (project core)
- C++ compiler only when building tests

> Note: `ds/dynarr.h` uses GNU C extensions (`({ ... })`, `__typeof__`).
> Use GCC/Clang in GNU mode (for example `-std=gnu11`) for C builds.

## Use in another CMake project

### Option A: add as a subdirectory

```cmake
add_subdirectory(path/to/data-structures)

target_link_libraries(your_target
		PRIVATE
				c-utils::c-utils
)
```

### Option B: install and find package

```cmake
find_package(c-utils CONFIG REQUIRED)

target_link_libraries(your_target
		PRIVATE
				c-utils::c-utils
)
```

## Build and install

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build
```

Install headers + CMake export target:

```bash
cmake --install build --prefix /your/prefix
```

By default, install only exports the header-only target and files under `include/`.
Test binaries are never installed.

## Testing

Tests use GoogleTest and are enabled by default only when this is the top-level project.

Manual control:

```bash
cmake -S . -B build -DC_UTILS_BUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Project layout

```text
include/
	ds/
		dynarr.h
		hashtable.h
	json/
		value.h
		object.h
		codec.h
tests/
```

## License

Add your preferred license file and update this section.
