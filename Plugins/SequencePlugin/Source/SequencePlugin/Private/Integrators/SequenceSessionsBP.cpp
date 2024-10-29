// Copyright 2024 Horizon Blockchain Games Inc. All rights reserved.

#include "Integrators/SequenceSessionsBP.h"

#include "PlayFabResponseIntent.h"
#include "PlayFabSendIntent.h"
#include "RequestHandler.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Util/Log.h"
#include "Util/SequenceSupport.h"

USequenceSessionsBP::USequenceSessionsBP()
{
	this->RPCManager = USequenceRPCManager::Make(false);
	this->Authenticator = NewObject<USequenceAuthenticator>();
}

void USequenceSessionsBP::StartEmailLoginAsync(const FString& Email)
{
	const TFunction<void()> OnSuccess = [this]
	{
		this->CallEmailLoginRequiresCode();
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Email Auth Error: %s"), *Error.Message);
		this->CallSessionCreationFailure();
	};

	this->RPCManager->InitEmailAuth(Email.ToLower(),OnSuccess,OnFailure);
}

void USequenceSessionsBP::ConfirmEmailLoginWithCodeAsync(const FString& Code)
{
	const TSuccessCallback<FCredentials_BE> OnSuccess = [this](const FCredentials_BE& Credentials)
	{
		this->StartSession(Credentials);
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Email Auth Error: %s"), *Error.Message);
		this->CallSessionCreationFailure();
	};

	const TFunction<void (FFederationSupportData)> OnFederationRequired = [this](const FFederationSupportData& FederationData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Account Force Create Or Federation Required"));
		this->CallFederationRequired(FederationData);
	};

	this->ClearSession();
	this->RPCManager->OpenEmailSession(Code, true, OnSuccess, OnFailure, OnFederationRequired);
}

void USequenceSessionsBP::GetGoogleTokenIdAsync()
{
	
}

void USequenceSessionsBP::GetAppleTokenIdAsync()
{
	
}

void USequenceSessionsBP::StartOidcSessionAsync(const FString& IdToken)
{
	const TSuccessCallback<FCredentials_BE> OnSuccess = [this](const FCredentials_BE& Credentials)
	{
		this->StartSession(Credentials);
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("OIDC Auth Error: %s"), *Error.Message);
		this->CallSessionCreationFailure();
	};

	const TFunction<void (FFederationSupportData)> OnFederationRequired = [this](const FFederationSupportData& FederationData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Account Force Create Or Federation Required"));
		this->CallFederationRequired(FederationData);
	};

	this->ClearSession();
	this->RPCManager->OpenOIDCSession(IdToken, true, OnSuccess, OnFailure, OnFederationRequired);
}

void USequenceSessionsBP::PlayFabRegistrationAsync(const FString& UsernameIn, const FString& EmailIn, const FString& PasswordIn)
{
	const TSuccessCallback<FString> OnSuccess = [this](const FString& SessionTicket)
	{
		const TSuccessCallback<FCredentials_BE> OnOpenSuccess = [this](const FCredentials_BE& Credentials)
		{
			this->StartSession(Credentials);
		};

		const FFailureCallback OnOpenFailure = [this](const FSequenceError& Error)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error: %s"), *Error.Message);
			this->CallSessionCreationFailure();
		};

		const TFunction<void (FFederationSupportData)> OnFederationRequired = [this](const FFederationSupportData& FederationData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Account Force Create Or Federation Required"));
			this->CallFederationRequired(FederationData);
		};
		
		this->RPCManager->OpenPlayFabSession(SessionTicket, true, OnOpenSuccess, OnOpenFailure, OnFederationRequired);
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error Response: %s"), *Error.Message);
		this->CallSessionCreationFailure();
	};

	this->ClearSession();
	this->PlayFabNewAccountLoginRpcAsync(UsernameIn, EmailIn, PasswordIn, OnSuccess, OnFailure);
}

void USequenceSessionsBP::PlayFabLoginAsync(const FString& UsernameIn, const FString& PasswordIn)
{
	const TSuccessCallback<FString> OnSuccess = [this](const FString& SessionTicket)
	{
		const TSuccessCallback<FCredentials_BE> OnOpenSuccess = [this](const FCredentials_BE& Credentials)
		{
			this->StartSession(Credentials);
		};

		const FFailureCallback OnOpenFailure = [this](const FSequenceError& Error)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error: %s"), *Error.Message);
			this->CallSessionCreationFailure();
		};

		const TFunction<void (FFederationSupportData)> OnFederationRequired = [this](const FFederationSupportData& FederationData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Account Force Create Or Federation Required"));
			this->CallFederationRequired(FederationData);
		};

		this->ClearSession();
		this->RPCManager->OpenPlayFabSession(SessionTicket, true, OnOpenSuccess, OnOpenFailure, OnFederationRequired);
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error Response: %s"), *Error.Message);
		this->CallSessionCreationFailure();
	};

	const TFunction<void(FString)> OnSuccessResponse = [OnSuccess, OnFailure](const FString& Response)
	{
		if (const FPlayFabLoginUserResponse ParsedResponse = USequenceSupport::JSONStringToStruct<FPlayFabLoginUserResponse>(Response); ParsedResponse.IsValid())
		{
			OnSuccess(ParsedResponse.Data.SessionTicket);
		}
		else
		{
			OnFailure(FSequenceError(ResponseParseError,Response));
		}
	};

	this->ClearSession();
	this->PlayFabLoginRpcAsync(UsernameIn, PasswordIn, OnSuccess, OnFailure);
}

void USequenceSessionsBP::StartGuestSessionAsync()
{
	const TSuccessCallback<FCredentials_BE> OnSuccess = [this](const FCredentials_BE& Credentials)
	{
		this->StartSession(Credentials);
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("Guest Auth Error: %s"), *Error.Message);
		this->CallSessionCreationFailure();
	};

	this->ClearSession();
	this->RPCManager->OpenGuestSession(true, OnSuccess, OnFailure);
}

void USequenceSessionsBP::FederateEmailAsync(const FString& EmailIn)
{
	const TFunction<void()> OnSuccess = [this]
	{
		this->CallEmailFederationRequiresCode();
	};

	const FFailureCallback OnFailure = [this](const FSequenceError& Error)
	{
		SEQ_LOG(Error, TEXT("Email Auth Error: %s"), *Error.Message);
		this->CallFederationFailure();
	};

	this->RPCManager->InitEmailFederation(EmailIn.ToLower(),OnSuccess,OnFailure);
}

void USequenceSessionsBP::ConfirmEmailFederationWithCodeAsync(const FString& Code)
{
	if (FStoredCredentials_BE StoredCredentials = this->Authenticator->GetStoredCredentials(); StoredCredentials.GetValid())
	{
		const TFunction<void()> OnSuccess = [this]()
		{
			this->CallFederationSucceeded();
		};

		const FFailureCallback OnFailure = [this](const FSequenceError& Error)
		{
			this->CallFederationFailure();
		};
		
		this->RPCManager->FederateEmailSession(StoredCredentials.GetCredentials().GetWalletAddress(), Code, OnSuccess, OnFailure);
	}
	else
	{
		SEQ_LOG(Error, TEXT("StoredCredentials are invalid, please login"));
		this->CallFederationFailure();
	}
}

void USequenceSessionsBP::FederateOidcTokenAsync(const FString& IdTokenIn)
{
	if (FStoredCredentials_BE StoredCredentials = this->Authenticator->GetStoredCredentials(); StoredCredentials.GetValid())
	{
		const TFunction<void()> OnSuccess = [this]()
		{
			this->CallFederationSucceeded();
		};

		const FFailureCallback OnFailure = [this](const FSequenceError& Error)
		{
			this->CallFederationFailure();
		};
		
		this->RPCManager->FederateOIDCSession(StoredCredentials.GetCredentials().GetWalletAddress(),IdTokenIn, OnSuccess, OnFailure);
	}
	else
	{
		SEQ_LOG(Error, TEXT("StoredCredentials are invalid, please login"));
		this->CallFederationFailure();
	}
}

void USequenceSessionsBP::FederatePlayFabRegistrationAsync(const FString& UsernameIn, const FString& EmailIn, const FString& PasswordIn) const
{
	SEQ_LOG(Error, TEXT("PlayFab Registration Federation is not yet supported."));
	this->CallFederationFailure();
}

void USequenceSessionsBP::FederatePlayFabLoginAsync(const FString& UsernameIn, const FString& PasswordIn) const
{
	SEQ_LOG(Error, TEXT("PlayFab Login Federation is not yet supported."));
	this->CallFederationFailure();
}

void USequenceSessionsBP::ClearSession() const
{
	this->Authenticator->ClearStoredCredentials();
}

bool USequenceSessionsBP::CheckExistingSession() const
{
	const FStoredCredentials_BE Credentials = this->Authenticator->GetStoredCredentials();
	return Credentials.GetValid();
}

void USequenceSessionsBP::StartSession(const FCredentials_BE& Credentials) const
{
	this->Authenticator->StoreCredentials(Credentials);
	this->CallSessionEstablished();
}

void USequenceSessionsBP::PlayFabNewAccountLoginRpcAsync(const FString& UsernameIn, const FString& EmailIn, const FString& PasswordIn, const TSuccessCallback<FString>& OnSuccess, const FFailureCallback& OnFailure)
{
	const TFunction<void(FString)> OnSuccessResponse = [OnSuccess, OnFailure](const FString& Response)
	{
		if (const FPlayFabRegisterUserResponse ParsedResponse = USequenceSupport::JSONStringToStruct<FPlayFabRegisterUserResponse>(Response); ParsedResponse.IsValid())
		{
			OnSuccess(ParsedResponse.Data.SessionTicket);
		}
		else
		{
			OnFailure(FSequenceError(ResponseParseError,Response));
		}
	};

	const FString TitleId = UConfigFetcher::GetConfigVar(UConfigFetcher::PlayFabTitleID);
	const FPlayFabRegisterUser RegisterUser(TitleId, EmailIn, PasswordIn, UsernameIn);
	const FString RequestBody = USequenceSupport::StructToPartialSimpleString(RegisterUser);
	const FString Url = "https://" + TitleId + ".playfabapi.com/Client/RegisterPlayFabUser";
	
	this->PlayFabRpcAsync(Url, RequestBody, OnSuccessResponse, OnFailure);
}

void USequenceSessionsBP::PlayFabLoginRpcAsync(const FString& UsernameIn, const FString& PasswordIn, const TSuccessCallback<FString>& OnSuccess, const FFailureCallback& OnFailure)
{
	const TFunction<void(FString)> OnSuccessResponse = [OnSuccess, OnFailure](const FString& Response)
	{
		if (const FPlayFabLoginUserResponse ParsedResponse = USequenceSupport::JSONStringToStruct<FPlayFabLoginUserResponse>(Response); ParsedResponse.IsValid())
		{
			OnSuccess(ParsedResponse.Data.SessionTicket);
		}
		else
		{
			OnFailure(FSequenceError(ResponseParseError,Response));
		}
	};

	const FString TitleId = UConfigFetcher::GetConfigVar(UConfigFetcher::PlayFabTitleID);
	const FPlayFabLoginUser LoginUser(PasswordIn,TitleId,UsernameIn);
	const FString RequestBody = USequenceSupport::StructToPartialSimpleString(LoginUser);
	const FString Url = "https://" + TitleId + ".playfabapi.com/Client/LoginWithPlayFab";
	
	this->PlayFabRpcAsync(Url, RequestBody, OnSuccessResponse, OnFailure);
}

void USequenceSessionsBP::PlayFabRpcAsync(const FString& Url, const FString& Content, const TSuccessCallback<FString>& OnSuccess, const FFailureCallback& OnFailure)
{
	NewObject<URequestHandler>()
		->PrepareRequest()
		->WithUrl(Url)
		->WithHeader("Content-type", "application/json")
		->WithVerb("POST")
		->WithContentAsString(Content)
		->ProcessAndThen(OnSuccess, OnFailure);
}

void USequenceSessionsBP::CallEmailLoginRequiresCode() const
{
	if (this->EmailLoginRequiresCode.IsBound())
		this->EmailLoginRequiresCode.Broadcast();
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: EmailLoginRequiresCode"));
}


void USequenceSessionsBP::CallEmailFederationRequiresCode() const
{
	if (this->EmailFederationRequiresCode.IsBound())
		this->EmailFederationRequiresCode.Broadcast();
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: EmailFederationRequiresCode"));
}

void USequenceSessionsBP::CallSessionEstablished() const
{
	if (this->SessionEstablished.IsBound())
		this->SessionEstablished.Broadcast();
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: SessionEstablished"));
}

void USequenceSessionsBP::CallFederationSucceeded() const
{
	if (this->FederationSucceeded.IsBound())
		this->FederationSucceeded.Broadcast();
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: FederationSucceeded"));
}

void USequenceSessionsBP::CallFederationFailure() const
{
	if (this->FederationFailure.IsBound())
		this->FederationFailure.Broadcast();
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: FederationFailure"));
}

void USequenceSessionsBP::CallSessionCreationFailure() const
{
	if (this->SessionCreationFailure.IsBound())
		this->SessionCreationFailure.Broadcast();
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: SessionCreationFailure"));
}

void USequenceSessionsBP::CallFederationRequired(const FFederationSupportData& FederationData) const
{
	if (this->FederationRequired.IsBound())
		this->FederationRequired.Broadcast(FederationData);
	else
		SEQ_LOG(Error, TEXT("Nothing bound to delegate: FederationRequired"));
}
