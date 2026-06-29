#pragma once

#include "CoreMinimal.h"

struct FTrueType { static constexpr bool Value = true; };
struct FFalseType { static constexpr bool Value = false; };

// TMakeVoid
template<typename... T>
struct TMakeVoid { using Type = void; };

// TIsContainer
template<typename T, typename = void>
struct TIsContainer : FFalseType {};

template<typename T>
struct TIsContainer<T, typename TMakeVoid<
	decltype(DeclVal<T>().begin()),
	decltype(DeclVal<T>().end()),
	typename TEnableIf<std::is_same_v<decltype(DeclVal<T>().IsEmpty()), bool>>::Type
>::Type> : FTrueType {};

// THas[FunctionName]Func
// Declare type traits to detect if type T has a function named FunctionName with params.
#define DECLARE_HASFUNCTION_TYPETRAITS(FunctionName, ReturnType, ...)                                                                                                             \
template<typename T>                                                                                                                                                              \
struct THas##FunctionName##Func                                                                                                                                                   \
{                                                                                                                                                                                 \
private:                                                                                                                                                                          \
	template<typename, typename... TArgs>                                                                                                                                         \
    struct Internal {                                                                                                                                                             \
		template<typename U, typename = void>                                                                                                                                     \
		struct THas##FunctionName##FuncInternal : FFalseType {};                                                                                                                   \
    	template<typename U>                                                                                                                                                      \
    	struct THas##FunctionName##FuncInternal<U, typename TEnableIf<std::is_same_v<ReturnType, decltype(DeclVal<U>().FunctionName(DeclVal<TArgs>()...))>>::Type> : FTrueType {}; \
    };                                                                                                                                                                            \
public:                                                                                                                                                                           \
	static constexpr bool Value = Internal<void, ##__VA_ARGS__>::template THas##FunctionName##FuncInternal<T>::Value;                                                             \
};

DECLARE_HASFUNCTION_TYPETRAITS(ToString, FString)


// GetTypeName<T>()
//
// Source-level type name recovered from the compiler's function signature
// (`__FUNCSIG__` on MSVC, `__PRETTY_FUNCTION__` elsewhere). No UE reflection
// required; works for raw enums, plain structs, primitives, etc.
namespace Strify::Private
{
	template<typename T>
	const char* TypeNameSig()
	{
#if defined(_MSC_VER)
		return __FUNCSIG__;
#else
		return __PRETTY_FUNCTION__;
#endif
	}

	inline FString ExtractTypeName(const char* Sig)
	{
		FString S = UTF8_TO_TCHAR(Sig);

#if defined(_MSC_VER)
		// "...TypeNameSig<TYPE>(void)"
		int32 LtIdx = INDEX_NONE;
		if (!S.FindChar(TEXT('<'), LtIdx)) return S;

		const int32 Start = LtIdx + 1;
		int32 Depth = 1;
		int32 End = Start;
		for (; End < S.Len(); ++End)
		{
			const TCHAR C = S[End];
			if (C == TEXT('<')) ++Depth;
			else if (C == TEXT('>')) { if (--Depth == 0) break; }
		}
		FString Inner = S.Mid(Start, End - Start);
#else
		// "... TypeNameSig() [T = TYPE]" or "... [with T = TYPE]"
		int32 EqIdx = INDEX_NONE;
		if (!S.FindChar(TEXT('='), EqIdx)) return S;

		int32 Start = EqIdx + 1;
		while (Start < S.Len() && S[Start] == TEXT(' ')) ++Start;

		int32 End = S.Len();
		if (End > 0 && S[End - 1] == TEXT(']')) --End;
		FString Inner = S.Mid(Start, End - Start);
#endif

		Inner.TrimStartAndEndInline();

		// MSVC elaborates with `class` / `struct` / `enum` / `union`. Strip them.
		const TCHAR* Keywords[] = { TEXT("class "), TEXT("struct "), TEXT("enum "), TEXT("union ") };
		for (const TCHAR* Kw : Keywords)
		{
			const int32 KwLen = FCString::Strlen(Kw);
			if (Inner.StartsWith(Kw))
			{
				Inner.RightChopInline(KwLen, EAllowShrinking::No);
				break;
			}
		}

		return Inner;
	}
}

namespace Strify
{
	template<typename T>
	const FString& GetTypeName()
	{
		static const FString Cached = Private::ExtractTypeName(Private::TypeNameSig<T>());
		return Cached;
	}
}
