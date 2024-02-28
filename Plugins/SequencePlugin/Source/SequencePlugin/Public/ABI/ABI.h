#pragma once
#include "Types/BinaryData.h"

static constexpr int GBlockByteLength = 32;
static constexpr int GSignatureLength = 4;

class ABIEncodeable
{
public:
	virtual ~ABIEncodeable() = default;
	virtual void EncodeHead(TArray<uint8> &Data) = 0;
	/*
	 * The Headposition is how far back the head of the entire array of arguments is,
	 *  while the offset is the argument number 
	 */
	virtual void EncodeTail(TArray<uint8> &Data, int HeadPosition, int Offset) = 0;
	static void PushEmptyBlock(TArray<uint8> &Data);
	static void CopyInUInt32(TArray<uint8> &Data, uint32 Value, int BlockPosition);
	static void CopyInInt32(TArray<uint8> &Data, int32 Value, int BlockPosition);
};

class TFixedABIArray : public ABIEncodeable
{
	TArray<ABIEncodeable*> MyData;
public:
	TFixedABIArray();
	TFixedABIArray(TArray<ABIEncodeable*> MyData);
	TArray<uint8> Encode();
	virtual void EncodeHead(TArray<uint8> &Data) override;
	virtual void EncodeTail(TArray<uint8> &Data, int HeadPosition, int Offset) override;
	void Push(ABIEncodeable* Arg);
};

class TDynamicABIArray : public ABIEncodeable
{
	TArray<ABIEncodeable*> MyData;
public:
	TDynamicABIArray();
	TDynamicABIArray(TArray<ABIEncodeable*> MyData);
	virtual void EncodeHead(TArray<uint8> &Data) override;
	virtual void EncodeTail(TArray<uint8> &Data, int HeadPosition, int Offset) override;
	void Push(ABIEncodeable* Arg);
};

class TFixedABIData : public ABIEncodeable
{
	TArray<uint8> MyData;
public:
	TFixedABIData(TArray<uint8> MyData);
	virtual void EncodeHead(TArray<uint8> &Data) override;
	virtual void EncodeTail(TArray<uint8> &Data, int HeadPosition, int Offset) override;
};

class TDynamicABIData : public ABIEncodeable
{
	TArray<uint8> MyData;
public:
	TDynamicABIData(TArray<uint8> MyData);
	virtual void EncodeHead(TArray<uint8> &Data) override;
	virtual void EncodeTail(TArray<uint8> &Data, int HeadPosition, int Offset) override;
};

class ABI : public ABIEncodeable
{
public:
	static TFixedABIData UInt32(uint32 Input);
	static TFixedABIData Int32(int32 Input);
	static TFixedABIData Bool(bool Input);
	static TFixedABIData Address(FAddress Address);
	static TDynamicABIData String(FString Input);
	static FUnsizedData Encode(FString Signature, TArray<ABIEncodeable*> Arr);
	static FString Display(FString Signature, TArray<ABIEncodeable*> Arr);
};