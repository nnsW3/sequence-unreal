// Fill out your copyright notice in the Description page of Project Settings.



#pragma once

#include "CoreMinimal.h"
#include "Async.h"
#include "Types/BinaryData.h"
#include "Errors.h"
#include "Types/Header.h"
#include "Http.h"
#include "JsonBuilder.h"
#include "Types/TransactionReceipt.h"
#include "EthTransaction.h"
#include "RPCCaller.h"

struct FContractCall;

enum EBlockTag
{
	Latest,
	Earliest,
	Pending,
	Safe,
	Finalized
};

FString TagToString(EBlockTag Tag);

/**
 * 
 */
class Provider : public RPCCaller
{
	FString Url;
	Provider Copy();

//helpers
	void BlockByNumberHelper(FString Number, TSuccessCallback<TSharedPtr<FJsonObject>> OnSuccess, FFailureCallback OnFailure);
	void HeaderByNumberHelper(FString Number, TSuccessCallback<FHeader> OnSuccess, FFailureCallback OnFailure);
	void NonceAtHelper(FString Number, TSuccessCallback<FBlockNonce> OnSuccess, FFailureCallback OnFailure);
	void CallHelper(FContractCall ContractCall, FString Number, TSuccessCallback<FUnsizedData> OnSuccess, FFailureCallback OnFailure);

public:
	Provider(FString Url);
	void BlockByNumber(uint64 Number, TSuccessCallback<TSharedPtr<FJsonObject>> OnSuccess, FFailureCallback OnFailure);
	void BlockByNumber(EBlockTag Tag, TSuccessCallback<TSharedPtr<FJsonObject>> OnSuccess, FFailureCallback OnFailure);
	void BlockByHash(FHash256 Hash, TSuccessCallback<TSharedPtr<FJsonObject>> OnSuccess, FFailureCallback OnFailure);
	void BlockNumber(TSuccessCallback<uint64> OnSuccess, FFailureCallback OnFailure);

	void HeaderByNumber(uint64 Id, TSuccessCallback<FHeader> OnSuccess, FFailureCallback OnFailure);
	void HeaderByNumber(EBlockTag Tag, TSuccessCallback<FHeader> OnSuccess, FFailureCallback OnFailure);
	void HeaderByHash(FHash256 Hash, TSuccessCallback<FHeader> OnSuccess, FFailureCallback OnFailure);
	
	void TransactionByHash(FHash256 Hash, TSuccessCallback<TSharedPtr<FJsonObject>> OnSuccess, FFailureCallback OnFailure);
	void TransactionCount(FAddress Addr, uint64 Number, TSuccessCallback<uint64> OnSuccess, FFailureCallback OnFailure);
	void TransactionCount(FAddress Addr, EBlockTag Tag, TSuccessCallback<uint64> OnSuccess, FFailureCallback OnFailure);
	void TransactionReceipt(FHash256 Hash, TSuccessCallback<FTransactionReceipt> OnSuccess, FFailureCallback OnFailure);

	void GetGasPrice(TSuccessCallback<FUnsizedData>, FFailureCallback OnFailure);
	void EstimateContractCallGas(FContractCall ContractCall, TSuccessCallback<FUnsizedData>, FFailureCallback OnFailure);
	void EstimateDeploymentGas(FAddress From, FString Bytecode, TSuccessCallback<FUnsizedData>, FFailureCallback OnFailure);

	void DeployContract(FString Bytecode, FPrivateKey PrivKey, int64 ChainId, TSuccessCallback<FAddress> OnSuccess, FFailureCallback OnFailure);
	void DeployContractWithHash(FString Bytecode, FPrivateKey PrivKey, int64 ChainId, TSuccessCallbackTuple<FAddress, FUnsizedData> OnSuccess, FFailureCallback OnFailure);
	
	void NonceAt(uint64 Number, TSuccessCallback<FBlockNonce> OnSuccess, FFailureCallback OnFailure);
	void NonceAt(EBlockTag Tag, TSuccessCallback<FBlockNonce> OnSuccess, FFailureCallback OnFailure);
	void SendRawTransaction(FString Data, TSuccessCallback<FUnsizedData> OnSuccess, FFailureCallback OnFailure);
	
	void ChainId(TSuccessCallback<uint64> OnSuccess, FFailureCallback OnFailure);

	void Call(FContractCall ContractCall, uint64 Number, TSuccessCallback<FUnsizedData> OnSuccess, FFailureCallback OnFailure);
	void Call(FContractCall ContractCall, EBlockTag Number, TSuccessCallback<FUnsizedData> OnSuccess, FFailureCallback OnFailure);
	void NonViewCall(FEthTransaction transaction, FPrivateKey PrivateKey, int ChainID, TSuccessCallback<FUnsizedData> OnSuccess, FFailureCallback OnFailure);

};
