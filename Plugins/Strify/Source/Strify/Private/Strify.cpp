#include "Strify.h"

FString UStrify::ToHexString(const TArray<uint8>& Data)
{
	return ToHexString(Data.GetData(), Data.Num());
}

FString UStrify::ToHexString(const uint8* Data, int32 Count)
{
	FString Res;
	for (auto i = 0; i < Count; i++)
	{
		Res += FString::Printf(TEXT("%02X"), *(Data + i));
	}
	return Res;
}
