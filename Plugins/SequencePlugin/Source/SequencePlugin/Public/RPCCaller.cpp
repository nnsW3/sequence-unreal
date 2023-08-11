#include "RPCCaller.h"

#include "HexUtility.h"
#include "RequestHandler.h"

TSharedPtr<FJsonObject> RPCCaller::Parse(FString JsonRaw)
{
	TSharedPtr<FJsonObject> JsonParsed;

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonRaw);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		return JsonParsed;
	}

	return nullptr;
}


TResult<TSharedPtr<FJsonObject>> RPCCaller::ExtractJsonObjectResult(FString JsonRaw)
{
	UE_LOG(LogTemp, Display, TEXT("I got this raw json: %s"), *JsonRaw);
	TSharedPtr<FJsonObject> Json = Parse(JsonRaw);
	
	if(!Json)
	{
		return MakeError(SequenceError(EmptyResponse, "Could not extract response"));
	}

	return MakeValue(Json->GetObjectField("result"));
}

TResult<FString> RPCCaller::ExtractStringResult(FString JsonRaw)
{
	TSharedPtr<FJsonObject> Json = Parse(JsonRaw);
	
	if(!Json)
	{
		return MakeError(SequenceError(EmptyResponse, "Could not extract response"));
	}

	return MakeValue(Json->GetStringField("result"));
}

TResult<uint64> RPCCaller::ExtractUIntResult(FString JsonRaw)
{
	TResult<FString> Result = ExtractStringResult(JsonRaw);
	if(!Result.HasValue())
	{
		return MakeError(Result.GetError());
	}

	TOptional<uint64> Convert = HexStringToUint64(Result.GetValue());
	if(!Convert.IsSet())
	{
		return MakeError(SequenceError(ResponseParseError, "Couldn't convert \"" + Result.GetValue() + "\" to a number."));
	}
	return MakeValue(Convert.GetValue());
}

void RPCCaller::SendRPC(FString Url, FString Content, TSuccessCallback<FString> OnSuccess, FFailureCallback OnError)
{
	NewObject<URequestHandler>()
		->PrepareRequest()
		->WithUrl(Url)
		->WithHeader("Content-type", "application/json")
		->WithHeader("Accept", "application/json")
		->WithVerb("POST")
		->WithContentAsString(Content)
		->ProcessAndThen(OnSuccess, OnError);
}

FJsonBuilder RPCCaller::RPCBuilder(const FString MethodName)
{
	return *FJsonBuilder().ToPtr()
		->AddString("jsonrpc", "2.0")
		->AddInt("id", 1)
		->AddString("method", MethodName);
}