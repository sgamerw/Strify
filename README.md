# Strify

## 项目介绍

Strify 是一个基于 C++ 开发的字符串处理和模板元编程工具库，专注于为开发人员提供高效的编译期类型操作和运行时数据转换功能。项目深度集成 Unreal Engine 框架，旨在简化游戏开发和实时系统中的日志输出、调试流程及类型安全转换工作流。

**核心价值**：
- 通过模板元编程实现编译期类型检查与优化，提升运行时性能
- 提供统一的数据到字符串转换接口，支持包括 UObject、弱指针、容器、枚举等复杂类型的标准化输出
- 模块化架构设计，工具包层与测试层分离，便于功能扩展和集成验证
- 专为 Unreal Engine 项目优化的字符串处理方案，内置生命周期管理与引擎无缝对接

**典型应用场景**：
- 游戏开发中的调试信息格式化输出
- 实时系统的运行时日志生成
- 复杂数据结构的可视化诊断
- 自动化测试用例的验证输出

## 功能简介

### 核心功能模块

| 模块             | 功能描述                                                                  |
|------------------|---------------------------------------------------------------------------|
| UStrify          | 提供静态方法实现运行时数据转换（基本类型/指针/容器→FString）              |
| 模板元编程工具集 | 包含 TIsContainer 类型检查、FTrueType/FFalseType 标记等编译期工具         |
| FStrifyModule    | 模块生命周期管理（初始化/关闭）                                           |
| 测试层           | 通过 Automation Spec 验证核心功能                                         |

### 关键特性
- **智能类型转换**：自动处理弱指针有效性（无效返回 `""`）、容器格式化等边界情况；UObject 子类若没有 `ToString()`，统一渲染为 `<unsupported {ClassName}: {GetName()}>`
- **多格式输出**：支持单行/多行模式控制字符串呈现格式
- **扩展性设计**：通过为类型添加 `ToString()` 成员函数，由 SFINAE 自动检测并接入
- **性能优化**：编译期类型特征判断避免运行时开销

## 快速上手

### 环境要求
- **编译器**：支持 C++17 及模板元编程的现代编译器
- **引擎依赖**：Unreal Engine 5.6+
- **构建系统**：Unreal Build Tool (UBT)

### 基础集成
1. 将 Strify 模块复制到项目 Plugins 目录
2. 在 `.uproject` 中启用插件：

```json
"Plugins": [
    { "Name": "Strify", "Enabled": true }
]
```

3. 在代码中调用转换接口：

```cpp
// 基本类型转换
FString IntStr = UStrify::ToString(42);
// 容器转换
TArray<int32> Numbers = {1, 2, 3};
FString ArrayStr = UStrify::ToString(Numbers, true); // 多行模式
// 日志输出
UE_LOG(LogTemp, Display, TEXT("Converted: %s"), *ArrayStr);
```

### 测试验证
在测试场景的 BeginPlay 中调用示例测试：

```cpp
void ATestActor::BeginPlay()
{
    Super::BeginPlay();

    // 验证整数数组转换
    TArray<int32> TestArray = {10, 20, 30};
    FString Result = UStrify::ToString(TestArray);
    UE_LOG(LogTemp, Log, TEXT("Array Test: %s"), *Result);
}
```

## 常见问题

**Q：如何处理自定义类型的字符串转换？**
A：为类型添加 `FString ToString() const` 成员函数，`THasToStringFunc` 会通过 SFINAE 自动检测并接入：

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

**Q：弱指针转换时输出空字符串？**
A：这是预期行为——当指针为 null 或弱指针失效时返回空串 `""`。在容器中表现为一个真实的空缺位（例如 `[1, , 3]`）。可通过 `IsValid()` 预先检查：

```cpp
if (WeakPtr.IsValid()) {
    FString ObjInfo = UStrify::ToString(WeakPtr);
}
```

**Q：如何验证容器类型判断是否正确？**
A：使用编译时静态断言：

```cpp
static_assert(TIsContainer<TArray<int32>>::Value, "Should be container");
static_assert(!TIsContainer<int32>::Value, "Should not be container");
```
