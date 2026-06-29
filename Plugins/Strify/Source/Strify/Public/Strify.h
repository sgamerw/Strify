#pragma once
#include "TypeTraits.h"
#include "Templates/IsUEnumClass.h"

#include "Strify.generated.h"

UCLASS()
class UStrify : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


#pragma region Base
public:
	template<typename T>
	static typename TEnableIf<TIsIntegral<T>::Value, FString>::Type ToString(const T& Value)
	{
		return FString::FromInt(static_cast<int32>(Value));
	}

	static FString ToString(const uint32& Value)
	{
		return FString::Printf(TEXT("%u"), Value);
	}

	static FString ToString(const int64& Value)
	{
		return FString::Printf(TEXT("%lld"), Value);
	}

	static FString ToString(const uint64& Value)
	{
		return FString::Printf(TEXT("%llu"), Value);
	}

	template<typename T>
	static typename TEnableIf<TIsFloatingPoint<T>::Value, FString>::Type ToString(const T& Value)
	{
		return FString::Printf(TEXT("%.3f"), static_cast<float>(Value));
	}

	static FString ToString(const bool& Value)
	{
		return FString::Printf(TEXT("%s"), Value ? TEXT("true") : TEXT("false"));
	}

	static FString ToString(const FString& Value)
	{
		return Value;
	}


#pragma endregion Base



#pragma region Enum

	template<typename T>
	static typename TEnableIf<TIsEnum<T>::Value, FString>::Type ToString(const T& Value)
	{
		if constexpr (TIsUEnumClass<T>::Value)
		{
			return StaticEnum<T>()->GetNameStringByValue(static_cast<int64>(Value));
		}
		else
		{
			using UnderlyingType = __underlying_type(T);
			return FString::Printf(TEXT("%s(%s)"),
				*Strify::GetTypeName<T>(),
				*UStrify::ToString(static_cast<UnderlyingType>(Value)));
		}
	}

	template<typename T>
	static typename TEnableIf<TIsEnum<T>::Value, FString>::Type ToString(const TEnumAsByte<T>& Value)
	{
		return UEnum::GetValueAsString(Value);
	}

#pragma endregion Enum



#pragma region ToString
public:
	template<typename T>
	static typename TEnableIf<THasToStringFunc<T>::Value, FString>::Type ToString(const T& Value)
	{
		return Value.ToString();
	}

	template<typename T>
	static typename TEnableIf<TIsDerivedFrom<T, UObject>::Value && !THasToStringFunc<T>::Value, FString>::Type ToString(const T& Value)
	{
			return FString::Printf(TEXT("<unsupported %s: %s>"), *Value.GetClass()->GetName(), *Value.GetName());

	}


#pragma endregion ToString



#pragma region Pointer
public:
	template<typename T>
	static typename TEnableIf<TIsPointer<T>::Value, FString>::Type ToString(const T& Value)
	{
		return Value ? UStrify::ToString(*Value) : TEXT("");
	}

	template<typename T>
	static FString ToString(TStrongObjectPtr<T> Value)
	{
		return Value.IsValid() ? UStrify::ToString(Value.Get()) : TEXT("");
	}

	template<typename T>
	static FString ToString(TWeakObjectPtr<T> Value)
	{
		return Value.IsValid() ? UStrify::ToString(Value.Get()) : TEXT("");
	}

	template<typename T>
	static FString ToString(TWeakPtr<T> Value)
	{
		return Value.IsValid() ? UStrify::ToString(Value.Pin().Get()) : TEXT("");
	}

	template<typename T>
	static FString ToString(TSharedPtr<T> Value)
	{
		return Value.IsValid() ? UStrify::ToString(Value.Get()) : TEXT("");
	}

	template<typename T>
	static FString ToString(const TUniquePtr<T>& Value)
	{
		return Value.IsValid() ? UStrify::ToString(Value.Get()) : TEXT("");
	}
#pragma endregion Pointer



#pragma region FVariant
public:
	template<typename T>
	static typename TEnableIf<std::is_same_v<T, FVariant>, FString>::Type ToString(const T& Value)
	{
		switch (auto VariantType = Value.GetType())
		{
		case EVariantTypes::Bool:     return UStrify::ToString(Value.template GetValue<bool>());
		case EVariantTypes::Float:    return UStrify::ToString(Value.template GetValue<float>());
		case EVariantTypes::String:   return UStrify::ToString(Value.template GetValue<FString>());
		case EVariantTypes::Int8:     return UStrify::ToString(Value.template GetValue<int8>());
		case EVariantTypes::Int16:    return UStrify::ToString(Value.template GetValue<int16>());
		case EVariantTypes::Int32:    return UStrify::ToString(Value.template GetValue<int32>());
		case EVariantTypes::Int64:    return UStrify::ToString(Value.template GetValue<int64>());
		case EVariantTypes::UInt8:    return UStrify::ToString(Value.template GetValue<uint8>());
		case EVariantTypes::UInt16:   return UStrify::ToString(Value.template GetValue<uint16>());
		case EVariantTypes::UInt32:   return UStrify::ToString(Value.template GetValue<uint32>());
		case EVariantTypes::UInt64:   return UStrify::ToString(Value.template GetValue<uint64>());
		default: return FString::Printf(TEXT("<unsupported variant: EVariantTypes(%d)>"), static_cast<int32>(VariantType));
		}
	}
#pragma endregion FVariant



#pragma region Container
public:
	template<typename TContainer, typename TFunctor, typename = typename TEnableIf<TIsContainer<TContainer>::Value>::Type>
	static FString ContainerToStringHelper(const TContainer& Container, const FString& StartToken, const FString& EndToken, bool Multilines, TFunctor&& ElementToStringFunctor)
	{
		if (Container.IsEmpty()) return TEXT("");

		TArray<FString> ElementStrings{};

		for (const auto& Element : Container)
		{
			ElementStrings.Emplace(ElementToStringFunctor(Element));
		}

		auto Res = FString::Join(ElementStrings, Multilines ? TEXT(",\n") : TEXT(", "));

		if (Multilines)
		{
			return FString::Printf(TEXT("%s\n%s\n%s"), *StartToken, *Res, *EndToken);
		}
		else
		{
			return FString::Printf(TEXT("%s%s%s"), *StartToken, *Res, *EndToken);
		}
	}

	template<typename T>
	static FString ToString(const TArray<T>& Array, bool Multilines = false)
	{
		return UStrify::ContainerToStringHelper(Array, TEXT("["), TEXT("]"), Multilines, [](const auto& Element) -> FString {
			return UStrify::ToString(Element);
		});
	}

	template<typename T>
	static FString ToString(const TSet<T>& Set, bool Multilines = false)
	{
		return UStrify::ContainerToStringHelper(Set, TEXT("("), TEXT(")"), Multilines, [](const auto& Element) -> FString {
			return UStrify::ToString(Element);
		});
	}

	template<typename TKey, typename TValue>
	static FString ToString(const TMap<TKey, TValue>& Map, bool Multilines = false)
	{
		return UStrify::ContainerToStringHelper(Map, TEXT("{"), TEXT("}"), Multilines, [](const auto& Element) -> FString {
			return FString::Printf(TEXT("%s: %s"), *UStrify::ToString(Element.Key), *UStrify::ToString(Element.Value));
		});
	}
#pragma endregion Container


#pragma region Hex
public:
	static FString ToHexString(const TArray<uint8>& Data);
	static FString ToHexString(const uint8* Data, int32 Count);
#pragma endregion Hex
};
