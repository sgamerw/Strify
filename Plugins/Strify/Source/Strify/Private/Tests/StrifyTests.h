#pragma once

#include "Strify.h"

#include "StrifyTests.generated.h"

UENUM()
enum ETestEnum
{
	TestEnum_Alpha,
	TestEnum_Beta,
	TestEnum_Gamma,
};

UENUM()
enum class ETestEnumClass : uint8
{
	Red,
	Green,
	Blue,
};

UENUM()
namespace ETestNamespacedEnum
{
	enum Type
	{
		One,
		Two,
		Three,
	};
}

// Raw enums — not tagged with UENUM, no UE reflection.
enum class ERawEnumClass : uint8
{
	Foo = 0,
	Bar = 1,
	Baz = 2,
};

enum ERawEnum
{
	RawEnum_Zero = 0,
	RawEnum_One = 1,
	RawEnum_Two = 2,
};

UCLASS()
class UTestObject : public UObject
{
	GENERATED_BODY()

public:

	int32 IntValue = 0;
	float FloatValue = 0.0f;
	FString StringValue;
	bool BoolValue = false;

	UTestObject(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

	FString ToString() const
	{
		return FString::Printf(
			TEXT("IntValue:%s, FloatValue:%s, StringValue:%s, BoolValue:%s"),
			*UStrify::ToString(IntValue),
			*UStrify::ToString(FloatValue),
			*UStrify::ToString(StringValue),
			*UStrify::ToString(BoolValue));
	}
};

UCLASS()
class UTestObjectNoToString : public UObject
{
	GENERATED_BODY()
};

// Plain (non-UObject) struct exercising the THasToStringFunc convention branch.
struct FTestStruct
{
	int32 Id = 0;
	FString Label;

	FString ToString() const
	{
		return FString::Printf(TEXT("FTestStruct(Id=%s, Label=%s)"),
			*UStrify::ToString(Id),
			*UStrify::ToString(Label));
	}
};
