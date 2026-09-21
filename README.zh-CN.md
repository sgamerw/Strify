# Strify

[English](README.md) | 简体中文

![UE 5.6+](https://img.shields.io/badge/UE-5.6%2B-313131)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

*不是 `LexToString`，不是 JSON — 面向 UE 容器和指针的编译期调试 stringify。*

Unreal Engine 插件，通过 `UStrify::ToString` 将值转为 `FString`。重载在编译期选择（SFINAE / `if constexpr`）。类型可提供 `ToString()` 成员以接入。

## 环境要求

- Unreal Engine 5.6+

## 安装

1. 将 `Plugins/Strify` 复制到项目的 `Plugins` 目录。
2. 在 `.uproject` 中启用插件：

```json
"Plugins": [
  { "Name": "Strify", "Enabled": true }
]
```

3. 在需要包含头文件的模块的 `Build.cs` 中声明依赖：

```cs
PublicDependencyModuleNames.Add("Strify");
```

本仓库是一个已包含该插件的 UE 5.6 示例项目（`StrifyTest`）。

## 用法

```cpp
#include "Strify.h"

FString IntStr = UStrify::ToString(42);

TArray<int32> Numbers = {1, 2, 3};
FString ArrayStr = UStrify::ToString(Numbers);        // [1, 2, 3]
FString Multiline = UStrify::ToString(Numbers, true); // [\n1,\n2,\n3\n]

UE_LOG(LogTemp, Display, TEXT("%s"), *ArrayStr);
```

在结构体或 `UObject` 子类上添加 `FString ToString() const`，`THasToStringFunc` 会自动检测：

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

## 支持的类型

| 类型 | 结果 |
|------|------|
| 整数 | 十进制 |
| 浮点 | 三位小数（`%.3f`） |
| `bool` | `true` / `false` |
| `FString` | 原样返回 |
| `UENUM` enum class | 反射名称 |
| `TEnumAsByte<T>` | `UEnum::GetValueAsString` |
| 其他枚举 | `TypeName(underlying)` |
| 带 `ToString()` 的类型 | 调用该成员 |
| 无 `ToString()` 的 `UObject` | `<unsupported {ClassName}: {GetName()}>` |
| 裸指针、共享/独占/弱指针、对象指针 | 有效则递归转换；空或失效返回 `""` |
| `FVariant` | 按存储类型转发；未知类型 → `<unsupported variant: EVariantTypes(N)>` |
| `TArray` | `[a, b, c]` |
| `TSet` | `(a, b, c)` |
| `TMap` | `{key: value, ...}` |
| 字节缓冲 | `UStrify::ToHexString` — 大写十六进制，无分隔符 |

空容器转换为 `""`。容器内的空值或空元素保留占位（如 `[1, , 3]`）。嵌套容器会递归转换。

## 类型特征

`TypeTraits.h`：

- `TIsContainer<T>` — 检测 `begin()`、`end()`、`IsEmpty()`
- `THasToStringFunc<T>` — 检测 `FString ToString()`（由 `DECLARE_HASFUNCTION_TYPETRAITS` 生成）
- `Strify::GetTypeName<T>()` — 从编译器签名（`__FUNCSIG__` / `__PRETTY_FUNCTION__`）解析类型名，不依赖 UE 反射

```cpp
static_assert(TIsContainer<TArray<int32>>::Value);
static_assert(!TIsContainer<int32>::Value);
```

## 测试

编辑器 Automation Spec 位于 `Plugins/Strify/Source/Strify/Private/Tests`（`Strify.UnitTests`）。

## 许可证

[MIT](LICENSE)。与 Unreal Engine 一起使用时，仍受 Epic Games 的 Unreal Engine EULA 约束。
