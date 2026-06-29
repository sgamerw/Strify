#include "StrifyTests.h"

#include "Misc/AutomationTest.h"
#include "Misc/Variant.h"
#include "Strify.h"
#include "UObject/StrongObjectPtr.h"

DEFINE_SPEC(FStrifyUnitTestsSpec, "Strify.UnitTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FStrifyUnitTestsSpec::Define()
{
	Describe(TEXT("BaseTypes"), [this]()
	{
		It(TEXT("converts int8"), [this]()
		{
			const int8 Value = 42;
			TestEqual(TEXT("int8 -> string"), UStrify::ToString(Value), TEXT("42"));
		});

		It(TEXT("converts int16"), [this]()
		{
			const int16 Value = 1234;
			TestEqual(TEXT("int16 -> string"), UStrify::ToString(Value), TEXT("1234"));
		});

		It(TEXT("converts int32"), [this]()
		{
			const int32 Value = 123;
			TestEqual(TEXT("int32 -> string"), UStrify::ToString(Value), TEXT("123"));
		});

		It(TEXT("converts int64"), [this]()
		{
			const int64 Value = 123456789;
			TestEqual(TEXT("int64 -> string"), UStrify::ToString(Value), TEXT("123456789"));
		});

		It(TEXT("converts uint8"), [this]()
		{
			const uint8 Value = 255;
			TestEqual(TEXT("uint8 -> string"), UStrify::ToString(Value), TEXT("255"));
		});

		It(TEXT("converts uint16"), [this]()
		{
			const uint16 Value = 65535;
			TestEqual(TEXT("uint16 -> string"), UStrify::ToString(Value), TEXT("65535"));
		});

		It(TEXT("converts uint32"), [this]()
		{
			const uint32 Value = 4294967295u;
			TestEqual(TEXT("uint32 -> string"), UStrify::ToString(Value), TEXT("4294967295"));
		});

		It(TEXT("converts uint64"), [this]()
		{
			const uint64 Value = 18446744073709551615ull;
			TestEqual(TEXT("uint64 -> string"), UStrify::ToString(Value), TEXT("18446744073709551615"));
		});

		It(TEXT("converts float to 3-decimal string"), [this]()
		{
			const float Value = 3.14159f;
			TestEqual(TEXT("float -> string"), UStrify::ToString(Value), TEXT("3.142"));
		});

		It(TEXT("converts double to 3-decimal string"), [this]()
		{
			const double Value = 3.14159;
			TestEqual(TEXT("double -> string"), UStrify::ToString(Value), TEXT("3.142"));
		});

		It(TEXT("converts bool true"), [this]()
		{
			TestEqual(TEXT("bool true -> string"), UStrify::ToString(true), TEXT("true"));
		});

		It(TEXT("converts bool false"), [this]()
		{
			TestEqual(TEXT("bool false -> string"), UStrify::ToString(false), TEXT("false"));
		});

		It(TEXT("passes FString through unchanged"), [this]()
		{
			const FString Value = TEXT("Hello, Strify!");
			TestEqual(TEXT("FString -> string"), UStrify::ToString(Value), Value);
		});
	});

	Describe(TEXT("Container.Array"), [this]()
	{
		It(TEXT("converts TArray<int32>"), [this]()
		{
			const TArray<int32> Values = { 2, 1, 0, -1, -2 };
			TestEqual(TEXT("TArray<int32> -> string"),
				UStrify::ToString(Values), TEXT("[2, 1, 0, -1, -2]"));
		});

		It(TEXT("converts TArray<float>"), [this]()
		{
			const TArray<float> Values = { 1.3f, 2.5f, 3.1f, -1.8f, -2.88f };
			TestEqual(TEXT("TArray<float> -> string"),
				UStrify::ToString(Values), TEXT("[1.300, 2.500, 3.100, -1.800, -2.880]"));
		});

		It(TEXT("converts TArray<bool>"), [this]()
		{
			const TArray<bool> Values = { true, false, true, false, true };
			TestEqual(TEXT("TArray<bool> -> string"),
				UStrify::ToString(Values), TEXT("[true, false, true, false, true]"));
		});
	});

	Describe(TEXT("ToStringConvention"), [this]()
	{
		It(TEXT("dispatches to T::ToString() for types with that member"), [this]()
		{
			UTestObject* Obj = NewObject<UTestObject>(GetTransientPackage());
			Obj->IntValue = 7;
			Obj->FloatValue = 1.5f;
			Obj->StringValue = TEXT("hello");
			Obj->BoolValue = true;

			TestEqual(TEXT("UTestObject -> ToString()"),
				UStrify::ToString(*Obj), Obj->ToString());
		});

		It(TEXT("dispatches to T::ToString() for plain non-UObject structs"), [this]()
		{
			const FTestStruct Value{ 42, TEXT("hi") };
			TestEqual(TEXT("FTestStruct -> ToString()"),
				UStrify::ToString(Value), Value.ToString());
		});
	});

	Describe(TEXT("UObjectFallback"), [this]()
	{
		It(TEXT("renders <unsupported ClassName: InstanceName> for UObjects without ToString()"), [this]()
		{
			UTestObjectNoToString* Obj = NewObject<UTestObjectNoToString>(GetTransientPackage());
			const FString Expected = FString::Printf(TEXT("<unsupported %s: %s>"),
				*Obj->GetClass()->GetName(), *Obj->GetName());
			TestEqual(TEXT("UObject -> <unsupported ClassName: InstanceName>"),
				UStrify::ToString(*Obj), Expected);
		});
	});

	Describe(TEXT("Enum"), [this]()
	{
		It(TEXT("converts TEnumAsByte to UE's readable form (Alpha)"), [this]()
		{
			const TEnumAsByte<ETestEnum> Value(TestEnum_Alpha);
			TestEqual(TEXT("TEnumAsByte<ETestEnum>(Alpha) -> string"),
				UStrify::ToString(Value), UEnum::GetValueAsString(Value));
		});

		It(TEXT("converts TEnumAsByte to UE's readable form (Beta)"), [this]()
		{
			const TEnumAsByte<ETestEnum> Value(TestEnum_Beta);
			TestEqual(TEXT("TEnumAsByte<ETestEnum>(Beta) -> string"),
				UStrify::ToString(Value), UEnum::GetValueAsString(Value));
		});

		It(TEXT("converts TEnumAsByte to UE's readable form (Gamma)"), [this]()
		{
			const TEnumAsByte<ETestEnum> Value(TestEnum_Gamma);
			TestEqual(TEXT("TEnumAsByte<ETestEnum>(Gamma) -> string"),
				UStrify::ToString(Value), UEnum::GetValueAsString(Value));
		});

		It(TEXT("converts UENUM enum class via reflection"), [this]()
		{
			const ETestEnumClass Value = ETestEnumClass::Green;
			const FString Expected = StaticEnum<ETestEnumClass>()->GetNameStringByValue(static_cast<int64>(Value));
			TestEqual(TEXT("ETestEnumClass::Green -> reflection name"),
				UStrify::ToString(Value), Expected);
		});

		// UENUM namespaced enums are NOT detected by TIsUEnumClass (UHT only specialises
		// it for `enum class`). They fall through to the raw branch, which is acceptable —
		// namespaced UENUMs are rare in UE5 codebases. Note GetTypeName reports the fully
		// qualified inner name (ETestNamespacedEnum::Type), which is helpful for clarity.
		It(TEXT("renders bare UENUM namespaced enum as TypeName(underlying)"), [this]()
		{
			const ETestNamespacedEnum::Type Value = ETestNamespacedEnum::Two;
			TestEqual(TEXT("ETestNamespacedEnum::Two -> ETestNamespacedEnum::Type(1) raw fallback"),
				UStrify::ToString(Value), TEXT("ETestNamespacedEnum::Type(1)"));
		});

		It(TEXT("renders raw enum class as TypeName(underlying)"), [this]()
		{
			const ERawEnumClass Value = ERawEnumClass::Bar;
			TestEqual(TEXT("ERawEnumClass::Bar -> ERawEnumClass(1)"),
				UStrify::ToString(Value), TEXT("ERawEnumClass(1)"));
		});

		It(TEXT("renders raw C-style enum as TypeName(underlying)"), [this]()
		{
			const ERawEnum Value = RawEnum_Two;
			TestEqual(TEXT("ERawEnum::RawEnum_Two -> ERawEnum(2)"),
				UStrify::ToString(Value), TEXT("ERawEnum(2)"));
		});
	});

	Describe(TEXT("RawPointer"), [this]()
	{
		It(TEXT("dereferences a valid int32* and recurses"), [this]()
		{
			int32 Value = 42;
			int32* const Ptr = &Value;
			TestEqual(TEXT("int32* -> dereferenced ToString"),
				UStrify::ToString(Ptr), UStrify::ToString(Value));
		});

		It(TEXT("returns \"\" for null int32*"), [this]()
		{
			int32* const Ptr = nullptr;
			TestEqual(TEXT("null int32* -> \"\""),
				UStrify::ToString(Ptr), TEXT(""));
		});

		It(TEXT("dereferences a UTestObject* and recurses through ToString()"), [this]()
		{
			UTestObject* Obj = NewObject<UTestObject>(GetTransientPackage());
			Obj->IntValue = 9;
			Obj->FloatValue = 0.5f;
			Obj->StringValue = TEXT("ptr");
			Obj->BoolValue = false;

			TestEqual(TEXT("UTestObject* -> ToString()"),
				UStrify::ToString(Obj), Obj->ToString());
		});

		It(TEXT("dereferences a UTestObjectNoToString* and renders <unsupported …>"), [this]()
		{
			UTestObjectNoToString* Obj = NewObject<UTestObjectNoToString>(GetTransientPackage());
			const FString Expected = FString::Printf(TEXT("<unsupported %s: %s>"),
				*Obj->GetClass()->GetName(), *Obj->GetName());
			TestEqual(TEXT("UTestObjectNoToString* -> <unsupported ClassName: InstanceName>"),
				UStrify::ToString(Obj), Expected);
		});
	});

	Describe(TEXT("SharedPointer"), [this]()
	{
		It(TEXT("dereferences a valid TSharedPtr<int32> and recurses"), [this]()
		{
			const TSharedPtr<int32> Ptr = MakeShared<int32>(123);
			TestEqual(TEXT("TSharedPtr<int32>(123) -> string"),
				UStrify::ToString(Ptr), TEXT("123"));
		});

		It(TEXT("returns \"\" for null TSharedPtr<int32>"), [this]()
		{
			const TSharedPtr<int32> Ptr;
			TestEqual(TEXT("null TSharedPtr<int32> -> \"\""),
				UStrify::ToString(Ptr), TEXT(""));
		});
	});

	Describe(TEXT("WeakPointer"), [this]()
	{
		It(TEXT("dereferences a valid TWeakPtr<int32> while owner is alive"), [this]()
		{
			const TSharedPtr<int32> Owner = MakeShared<int32>(456);
			const TWeakPtr<int32> Weak = Owner;
			TestEqual(TEXT("TWeakPtr<int32>(456) -> string"),
				UStrify::ToString(Weak), TEXT("456"));
		});

		It(TEXT("returns \"\" for an expired TWeakPtr<int32>"), [this]()
		{
			TWeakPtr<int32> Weak;
			{
				const TSharedPtr<int32> Owner = MakeShared<int32>(789);
				Weak = Owner;
			}
			TestEqual(TEXT("expired TWeakPtr<int32> -> \"\""),
				UStrify::ToString(Weak), TEXT(""));
		});
	});

	Describe(TEXT("UniquePointer"), [this]()
	{
		It(TEXT("dereferences a valid TUniquePtr<int32> and recurses"), [this]()
		{
			const TUniquePtr<int32> Ptr = MakeUnique<int32>(42);
			TestEqual(TEXT("TUniquePtr<int32>(42) -> string"),
				UStrify::ToString(Ptr), TEXT("42"));
		});

		It(TEXT("returns \"\" for null TUniquePtr<int32>"), [this]()
		{
			const TUniquePtr<int32> Ptr;
			TestEqual(TEXT("null TUniquePtr<int32> -> \"\""),
				UStrify::ToString(Ptr), TEXT(""));
		});
	});

	Describe(TEXT("WeakObjectPointer"), [this]()
	{
		It(TEXT("dereferences a valid TWeakObjectPtr<UTestObject> and recurses through ToString()"), [this]()
		{
			UTestObject* Obj = NewObject<UTestObject>(GetTransientPackage());
			Obj->IntValue = 7;
			Obj->FloatValue = 1.5f;
			Obj->StringValue = TEXT("hello");
			Obj->BoolValue = true;

			const TWeakObjectPtr<UTestObject> Weak(Obj);
			TestEqual(TEXT("TWeakObjectPtr<UTestObject> -> ToString()"),
				UStrify::ToString(Weak), Obj->ToString());
		});

		It(TEXT("returns \"\" for an invalid TWeakObjectPtr<UTestObject>"), [this]()
		{
			const TWeakObjectPtr<UTestObject> Weak;
			TestEqual(TEXT("invalid TWeakObjectPtr<UTestObject> -> \"\""),
				UStrify::ToString(Weak), TEXT(""));
		});
	});

	Describe(TEXT("StrongObjectPointer"), [this]()
	{
		It(TEXT("dereferences a valid TStrongObjectPtr<UTestObject> and recurses through ToString()"), [this]()
		{
			UTestObject* Obj = NewObject<UTestObject>(GetTransientPackage());
			Obj->IntValue = 11;
			Obj->FloatValue = 2.25f;
			Obj->StringValue = TEXT("strong");
			Obj->BoolValue = false;

			const TStrongObjectPtr<UTestObject> Strong(Obj);
			TestEqual(TEXT("TStrongObjectPtr<UTestObject> -> ToString()"),
				UStrify::ToString(Strong), Obj->ToString());
		});

		It(TEXT("returns \"\" for a null TStrongObjectPtr<UTestObject>"), [this]()
		{
			const TStrongObjectPtr<UTestObject> Strong;
			TestEqual(TEXT("null TStrongObjectPtr<UTestObject> -> \"\""),
				UStrify::ToString(Strong), TEXT(""));
		});
	});

	Describe(TEXT("Variant"), [this]()
	{
		It(TEXT("routes Bool variant through ToString(bool)"), [this]()
		{
			const bool KnownValue = true;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(bool) -> ToString(bool)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes Float variant through ToString(float)"), [this]()
		{
			const float KnownValue = 3.14159f;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(float) -> ToString(float)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes String variant through ToString(FString)"), [this]()
		{
			const FString KnownValue = TEXT("hello, variant");
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(FString) -> ToString(FString)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes Int8 variant through ToString(int8)"), [this]()
		{
			const int8 KnownValue = -42;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(int8) -> ToString(int8)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes Int16 variant through ToString(int16)"), [this]()
		{
			const int16 KnownValue = -1234;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(int16) -> ToString(int16)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes Int32 variant through ToString(int32)"), [this]()
		{
			const int32 KnownValue = -123456;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(int32) -> ToString(int32)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes Int64 variant through ToString(int64)"), [this]()
		{
			const int64 KnownValue = -123456789012ll;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(int64) -> ToString(int64)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes UInt8 variant through ToString(uint8)"), [this]()
		{
			const uint8 KnownValue = 200;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(uint8) -> ToString(uint8)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes UInt16 variant through ToString(uint16)"), [this]()
		{
			const uint16 KnownValue = 50000;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(uint16) -> ToString(uint16)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes UInt32 variant through ToString(uint32)"), [this]()
		{
			const uint32 KnownValue = 4000000000u;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(uint32) -> ToString(uint32)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("routes UInt64 variant through ToString(uint64)"), [this]()
		{
			const uint64 KnownValue = 18000000000000000000ull;
			FVariant Variant;
			Variant = KnownValue;
			TestEqual(TEXT("FVariant(uint64) -> ToString(uint64)"),
				UStrify::ToString(Variant), UStrify::ToString(KnownValue));
		});

		It(TEXT("renders <unsupported variant: EVariantTypes(0)> for a default-constructed FVariant"), [this]()
		{
			const FVariant Variant;
			TestEqual(TEXT("default FVariant -> <unsupported variant: EVariantTypes(0)>"),
				UStrify::ToString(Variant), TEXT("<unsupported variant: EVariantTypes(0)>"));
		});

		It(TEXT("renders <unsupported variant: EVariantTypes(5)> for a populated ByteArray FVariant"), [this]()
		{
			const TArray<uint8> Bytes = { 1, 2, 3 };
			const FVariant Variant(Bytes);
			TestEqual(TEXT("ByteArray FVariant -> <unsupported variant: EVariantTypes(5)>"),
				UStrify::ToString(Variant), TEXT("<unsupported variant: EVariantTypes(5)>"));
		});
	});

	Describe(TEXT("Container.Set"), [this]()
	{
		It(TEXT("renders TSet<int32> as (...) joined by ', '"), [this]()
		{
			const TSet<int32> Values = { 1, 2, 3, 4, 5 };
			const FString Result = UStrify::ToString(Values);

			TestTrue(TEXT("starts with '('"), Result.StartsWith(TEXT("(")));
			TestTrue(TEXT("ends with ')'"), Result.EndsWith(TEXT(")")));

			const FString Inner = Result.Mid(1, Result.Len() - 2);
			TArray<FString> Parts;
			Inner.ParseIntoArray(Parts, TEXT(", "), false);
			Parts.Sort();

			TArray<FString> Expected = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5") };
			Expected.Sort();

			TestEqual(TEXT("normalised TSet element list"),
				FString::Join(Parts, TEXT(",")), FString::Join(Expected, TEXT(",")));
		});
	});

	Describe(TEXT("Container.Map"), [this]()
	{
		It(TEXT("renders TMap<FString, int32> as {Key: Value, ...}"), [this]()
		{
			const TMap<FString, int32> Map = {
				{ TEXT("alpha"), 1 },
				{ TEXT("beta"),  2 },
				{ TEXT("gamma"), 3 },
			};
			const FString Result = UStrify::ToString(Map);

			TestTrue(TEXT("starts with '{'"), Result.StartsWith(TEXT("{")));
			TestTrue(TEXT("ends with '}'"), Result.EndsWith(TEXT("}")));

			const FString Inner = Result.Mid(1, Result.Len() - 2);
			TArray<FString> Parts;
			Inner.ParseIntoArray(Parts, TEXT(", "), false);
			Parts.Sort();

			TArray<FString> Expected = {
				TEXT("alpha: 1"),
				TEXT("beta: 2"),
				TEXT("gamma: 3"),
			};
			Expected.Sort();

			TestEqual(TEXT("normalised TMap entry list"),
				FString::Join(Parts, TEXT(",")), FString::Join(Expected, TEXT(",")));
		});
	});

	Describe(TEXT("Container.Nested"), [this]()
	{
		It(TEXT("renders TArray<TArray<int32>> with inner brackets"), [this]()
		{
			const TArray<TArray<int32>> Nested = {
				{ 1, 2 },
				{ 3, 4 },
			};
			TestEqual(TEXT("TArray<TArray<int32>> -> string"),
				UStrify::ToString(Nested), TEXT("[[1, 2], [3, 4]]"));
		});

		It(TEXT("renders TMap<FString, TArray<int32>> with inner brackets"), [this]()
		{
			const TMap<FString, TArray<int32>> Map = {
				{ TEXT("nums"), { 10, 20, 30 } },
			};
			TestEqual(TEXT("TMap<FString, TArray<int32>> -> string"),
				UStrify::ToString(Map), TEXT("{nums: [10, 20, 30]}"));
		});
	});

	Describe(TEXT("Container.Empty"), [this]()
	{
		It(TEXT("returns \"\" for an empty TArray<int32>"), [this]()
		{
			const TArray<int32> Values;
			TestEqual(TEXT("empty TArray<int32> -> \"\""),
				UStrify::ToString(Values), TEXT(""));
		});

		It(TEXT("returns \"\" for an empty TSet<int32>"), [this]()
		{
			const TSet<int32> Values;
			TestEqual(TEXT("empty TSet<int32> -> \"\""),
				UStrify::ToString(Values), TEXT(""));
		});

		It(TEXT("returns \"\" for an empty TMap<FString, int32>"), [this]()
		{
			const TMap<FString, int32> Map;
			TestEqual(TEXT("empty TMap<FString, int32> -> \"\""),
				UStrify::ToString(Map), TEXT(""));
		});
	});

	Describe(TEXT("Container.EmptySlots"), [this]()
	{
		It(TEXT("renders empty FString elements as a true gap"), [this]()
		{
			const TArray<FString> Values = { TEXT("a"), TEXT(""), TEXT("b") };
			TestEqual(TEXT("TArray<FString> with empty middle element"),
				UStrify::ToString(Values), TEXT("[a, , b]"));
		});

		It(TEXT("renders nested empty TArray<int32> as a true gap"), [this]()
		{
			const TArray<TArray<int32>> Nested = { { 1, 2 }, {}, { 3 } };
			TestEqual(TEXT("TArray<TArray<int32>> with empty middle"),
				UStrify::ToString(Nested), TEXT("[[1, 2], , [3]]"));
		});

		It(TEXT("renders null int32* elements as a true gap"), [this]()
		{
			int32 A = 1, C = 3;
			const TArray<int32*> Values = { &A, nullptr, &C };
			TestEqual(TEXT("TArray<int32*> with null middle element"),
				UStrify::ToString(Values), TEXT("[1, , 3]"));
		});

		It(TEXT("preserves a real FString \"Empty\" element distinct from a true empty slot"), [this]()
		{
			const TArray<FString> Values = { TEXT("Empty"), TEXT(""), TEXT("Empty") };
			TestEqual(TEXT("TArray<FString> distinguishing literal Empty from gap"),
				UStrify::ToString(Values), TEXT("[Empty, , Empty]"));
		});
	});

	Describe(TEXT("Container.Multilines"), [this]()
	{
		It(TEXT("multilines TArray<int32> with [\\n, ,\\n joins, \\n]"), [this]()
		{
			const TArray<int32> Values = { 1, 2, 3 };
			TestEqual(TEXT("multilines TArray<int32> -> string"),
				UStrify::ToString(Values, true), TEXT("[\n1,\n2,\n3\n]"));
		});

		It(TEXT("multilines TSet<int32> with (\\n, ,\\n joins, \\n)"), [this]()
		{
			const TSet<int32> Values = { 1, 2, 3 };
			const FString Result = UStrify::ToString(Values, true);

			TestTrue(TEXT("starts with '(\\n'"), Result.StartsWith(TEXT("(\n")));
			TestTrue(TEXT("ends with '\\n)'"), Result.EndsWith(TEXT("\n)")));

			const FString Inner = Result.Mid(2, Result.Len() - 4);
			TArray<FString> Parts;
			Inner.ParseIntoArray(Parts, TEXT(",\n"), false);
			Parts.Sort();

			TArray<FString> Expected = { TEXT("1"), TEXT("2"), TEXT("3") };
			Expected.Sort();

			TestEqual(TEXT("normalised multiline TSet element list"),
				FString::Join(Parts, TEXT(",")), FString::Join(Expected, TEXT(",")));
		});

		It(TEXT("multilines TMap<FString, int32> with {\\n, ,\\n joins, \\n}"), [this]()
		{
			const TMap<FString, int32> Map = {
				{ TEXT("alpha"), 1 },
				{ TEXT("beta"),  2 },
			};
			const FString Result = UStrify::ToString(Map, true);

			TestTrue(TEXT("starts with '{\\n'"), Result.StartsWith(TEXT("{\n")));
			TestTrue(TEXT("ends with '\\n}'"), Result.EndsWith(TEXT("\n}")));

			const FString Inner = Result.Mid(2, Result.Len() - 4);
			TArray<FString> Parts;
			Inner.ParseIntoArray(Parts, TEXT(",\n"), false);
			Parts.Sort();

			TArray<FString> Expected = { TEXT("alpha: 1"), TEXT("beta: 2") };
			Expected.Sort();

			TestEqual(TEXT("normalised multiline TMap entry list"),
				FString::Join(Parts, TEXT(",")), FString::Join(Expected, TEXT(",")));
		});
	});

	Describe(TEXT("HexString"), [this]()
	{
		It(TEXT("returns \"\" for an empty TArray<uint8>"), [this]()
		{
			const TArray<uint8> Bytes;
			TestEqual(TEXT("empty TArray<uint8> -> \"\""),
				UStrify::ToHexString(Bytes), TEXT(""));
		});

		It(TEXT("renders a single byte as zero-padded uppercase hex"), [this]()
		{
			const TArray<uint8> Bytes = { 0xAB };
			TestEqual(TEXT("{0xAB} -> \"AB\""),
				UStrify::ToHexString(Bytes), TEXT("AB"));
		});

		It(TEXT("renders multiple bytes as concatenated zero-padded uppercase hex"), [this]()
		{
			const TArray<uint8> Bytes = { 0x00, 0x0F, 0xFF };
			TestEqual(TEXT("{0x00, 0x0F, 0xFF} -> \"000FFF\""),
				UStrify::ToHexString(Bytes), TEXT("000FFF"));
		});

		It(TEXT("uint8*+count overload matches the TArray<uint8> overload"), [this]()
		{
			const TArray<uint8> Bytes = { 0x00, 0x0F, 0xFF };
			TestEqual(TEXT("ToHexString(ptr, count) == ToHexString(TArray)"),
				UStrify::ToHexString(Bytes.GetData(), Bytes.Num()),
				UStrify::ToHexString(Bytes));
		});
	});
}
