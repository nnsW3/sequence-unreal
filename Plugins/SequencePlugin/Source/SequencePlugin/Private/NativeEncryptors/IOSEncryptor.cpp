﻿//Copyright 2024 Horizon Blockchain Games Inc. All rights reserved.

#include "NativeEncryptors/IOSEncryptor.h"
#include "Native/IOSBridge.h"

FString UIOSEncryptor::Encrypt(const FString& StringIn)
{
	FString Result = "";
	UE_LOG(LogTemp,Display,TEXT("Preparing to encrypt on IOS"));
	Result = UIOSBridge::IOSEncrypt(StringIn);
	UE_LOG(LogTemp,Display,TEXT("Encrypted Result: %s"),*Result);
	return Result;
}

FString UIOSEncryptor::Decrypt(const FString& StringIn)
{
	FString Result = "";
	UE_LOG(LogTemp,Display,TEXT("Preparing to decrypt on IOS"));
	Result = UIOSBridge::IOSDecrypt(StringIn);
	UE_LOG(LogTemp,Display,TEXT("Decrypted Result: %s"),*Result);
	return Result;
}
