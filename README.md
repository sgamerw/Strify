# Strify

English | [简体中文](README.zh-CN.md)

![UE 5.6+](https://img.shields.io/badge/UE-5.6%2B-313131)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

*Not `LexToString`, not JSON — compile-time debug stringify for UE containers and pointers.*

Unreal Engine plugin that converts values to `FString` via `UStrify::ToString`. Overload selection is compile-time (SFINAE / `if constexpr`). Types can opt in with a `ToString()` member.

## Requirements

- Unreal Engine 5.6+

## Install

1. Copy `Plugins/Strify` into your project's `Plugins` directory.
2. Enable the plugin in the `.uproject`:

```json
"Plugins": [
  { "Name": "Strify", "Enabled": true }
]
```

3. Add the module to any `Build.cs` that includes the headers:

```cs
PublicDependencyModuleNames.Add("Strify");
```

This repository is a UE 5.6 sample project (`StrifyTest`) that already contains the plugin.

## Usage

```cpp
#include "Strify.h"

FString IntStr = UStrify::ToString(42);

TArray<int32> Numbers = {1, 2, 3};
FString ArrayStr = UStrify::ToString(Numbers);        // [1, 2, 3]
FString Multiline = UStrify::ToString(Numbers, true); // [\n1,\n2,\n3\n]

UE_LOG(LogTemp, Display, TEXT("%s"), *ArrayStr);
```

Add `FString ToString() const` on a struct or `UObject` subclass; `THasToStringFunc` detects it:

```cpp
struct FMyStruct
{
    int32 Value1;
    float Value2;

    FString ToString() const
    {
        return FString::Printf(TEXT("MyStruct(%d,%f)"), Value1, Value2);
    }
};
```

## Supported types

| Kind | Result |
|------|--------|
| Integers | Decimal |
| Floating point | Three decimal places (`%.3f`) |
| `bool` | `true` / `false` |
| `FString` | Unchanged |
| `UENUM` enum class | Reflection name |
| `TEnumAsByte<T>` | `UEnum::GetValueAsString` |
| Other enums | `TypeName(underlying)` |
| Types with `ToString()` | That member |
| `UObject` without `ToString()` | `<unsupported {ClassName}: {GetName()}>` |
| Raw, shared, unique, weak, and object pointers | Recurse if valid; `""` if null or stale |
| `FVariant` | Routed by stored type; unknown → `<unsupported variant: EVariantTypes(N)>` |
| `TArray` | `[a, b, c]` |
| `TSet` | `(a, b, c)` |
| `TMap` | `{key: value, ...}` |
| Byte buffers | `UStrify::ToHexString` — uppercase hex, no separators |

Empty containers stringify to `""`. Null or empty elements inside a container keep a slot (`[1, , 3]`). Nested containers recurse.

## Type traits

`TypeTraits.h`:

- `TIsContainer<T>` — `begin()`, `end()`, and `IsEmpty()`
- `THasToStringFunc<T>` — `FString ToString()` (via `DECLARE_HASFUNCTION_TYPETRAITS`)
- `Strify::GetTypeName<T>()` — compiler signature (`__FUNCSIG__` / `__PRETTY_FUNCTION__`), no UE reflection

```cpp
static_assert(TIsContainer<TArray<int32>>::Value);
static_assert(!TIsContainer<int32>::Value);
```

## Tests

Editor automation specs are in `Plugins/Strify/Source/Strify/Private/Tests` (`Strify.UnitTests`).

## License

[MIT](LICENSE). When used with Unreal Engine, the Epic Games Unreal Engine EULA still applies.
