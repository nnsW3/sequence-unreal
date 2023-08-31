// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectHandler.h"

TMap<FString, UTexture2D*> UObjectHandler::getProcessedImages()
{
	return this->storedResponses;
}

void UObjectHandler::OnDone()
{
	if (FOnDoneImageProcessingDelegate.IsBound())
	{
		UE_LOG(LogTemp, Display, TEXT("[Delegate bound calling bound UFUNCTION]"));
		FOnDoneImageProcessingDelegate.ExecuteIfBound();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Delegate Not bound in UObjectHandler]"));
	}
}

void UObjectHandler::setup(bool raw_cache_enabled)
{
	this->syncer = NewObject<USyncer>();
	this->syncer->setupForTesting("ImageHandler_"+this->GetName());
	//binding will occur here
	//need to bind to OnDone
	this->syncer->OnDoneDelegate.BindUFunction(this, "OnDone");
	this->use_raw_cache = raw_cache_enabled;
}

/*
* Used for using a hardset custom image format for all image requests
* made through this object, if you want to use the computable format use the other setup call
*/
void UObjectHandler::setupCustomFormat(bool raw_cache_enabled, EImageFormat format)
{
	this->syncer = NewObject<USyncer>();
	this->customFormat = format;
	this->useCustomFormat = true;
	this->syncer->OnDoneDelegate.BindUFunction(this, "OnDone");
	this->use_raw_cache = raw_cache_enabled;
}

void UObjectHandler::storeImageData(UTexture2D* image, FString url)
{
	this->storedResponses.Add(TPair<FString,UTexture2D*>(url,image));
	UE_LOG(LogTemp, Display, TEXT("[Image stored]"));
	this->syncer->dec();
	//this is when we would consider a response satisfied
}

void UObjectHandler::handle_request_raw(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString rep_content = "[error]";
	if (bWasSuccessful)
	{//Parse the json response
		Response = Request.Get()->GetResponse();//allows for more independence!
		//cache our response first!
		TArray<uint8> response_data = Response.Get()->GetContent();
		FString response_url = Request.Get()->GetURL();
		if (this->use_raw_cache)
			this->add_to_cache(response_url, response_data);//cache it if we need it!

		//build the image data and store it! also decrements our active request count!
		this->storeImageData(build_img_data(response_data, response_url), response_url);
	}
	else//error
	{
		switch (Request->GetStatus()) {
		case EHttpRequestStatus::Failed_ConnectionError:
			UE_LOG(LogTemp, Error, TEXT("Connection failed."));
		default:
			UE_LOG(LogTemp, Error, TEXT("Request failed."));
		}
		//in the event we failed or errored out we should decrement
		this->syncer->dec();
	}
}

TArray<FString> UObjectHandler::filterURLs(TArray<FString> urls)
{
	TArray<FString> filteredUrls;

	for (FString url : urls)
	{//note we don't support .svg or .gif, and we should filter out empty / invalid urls
		bool valid = true;
		valid &= !url.Contains(".gif");
		valid &= !url.Contains(".svg");
		if (valid && url.Len() > 7 && (url.Contains("http://") || url.Contains("https://")))
		{
			filteredUrls.Add(url);
		}
	}

	return filteredUrls;
}

bool UObjectHandler::check_raw_cache(FString URL, TArray<uint8>* raw_data)
{
	bool cache_hit = this->cache.Contains(URL);

	if (cache_hit)
	{
		UE_LOG(LogTemp, Display, TEXT("Raw Cache hit"));
		*raw_data = this->cache.Find(URL)->raw_data;//get the data in the cache!
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Raw Cache miss"));
	}
	return cache_hit;
}

void UObjectHandler::add_to_cache(FString URL, TArray<uint8> raw_data)
{
	if (!this->cache.Contains(URL) || this->cache.Num() == 0)
	{
		if (this->can_add_to_cache(raw_data.Num()))//do we have room in the cache?
		{//we have room in the cache for more!
			FRawData new_cache_entry;
			new_cache_entry.raw_data = raw_data;
			this->cache.Add(TTuple<FString, FRawData>(URL, new_cache_entry));//add the new cache entry if we don't already have it!
			this->current_cache_size += raw_data.Num();//add the bytes in to keep track!
		}
		else
		{//we now will clear data out at random until we can fit it
			TArray<FString> cache_keys;
			FString r_key;
			FRawData cached_data;

			//I include a check here incase we download an image that is just massive so we don't spin forever!
			while (this->cache.Num() > 0 && (this->current_cache_size + raw_data.Num()) > this->max_cache_size)
			{
				this->cache.GetKeys(cache_keys);//get the keys!
				r_key = cache_keys[FMath::RandRange(0, cache_keys.Num() - 1)];//get a random key!
				cached_data = *this->cache.Find(r_key);
				this->current_cache_size -= cached_data.raw_data.Num();//remove the count!
				this->cache.Remove(r_key);//remove it from the cache!
			}
			
			//now that we have room for a new entry add it in!
			FRawData new_cache_entry;
			new_cache_entry.raw_data = raw_data;
			this->cache.Add(TTuple<FString, FRawData>(URL, new_cache_entry));//add the new cache entry if we don't already have it!
			this->current_cache_size += raw_data.Num();//add the bytes in to keep track!
		}
	}
}

//used to clear the contents of the raw cache!
void UObjectHandler::clear_raw_cache()
{
	this->current_cache_size = 0;//nothing in here!
	this->cache.Empty();//clear the cache!
	this->cache.Shrink();//remove all slack as well
}

bool UObjectHandler::can_add_to_cache(int32 byte_count_to_add)
{
	return (this->current_cache_size + byte_count_to_add) <= (this->max_cache_size);
}

bool UObjectHandler::request_raw_base(FString URL)
{
	UE_LOG(LogTemp, Display, TEXT("Fetching Raw From: %s"), *URL);

	if (this->use_raw_cache)//are we using the cache?
	{
		TArray<uint8> cached_data, * cached_data_ptr;
		cached_data_ptr = &cached_data;
		bool cache_hit = this->check_raw_cache(URL, cached_data_ptr);
		if (cache_hit)
		{
			this->storeImageData(build_img_data(cached_data, URL), URL);
			return true; //we are done here!
		}
	}
	TSharedRef<IHttpRequest> http_post_req = FHttpModule::Get().CreateRequest();
	http_post_req->SetVerb("GET");
	http_post_req->SetURL(URL);
	http_post_req->SetTimeout(5);
	http_post_req->OnProcessRequestComplete().BindUObject(this, &UObjectHandler::handle_request_raw);
	http_post_req->ProcessRequest();
	return http_post_req.Get().GetStatus() == EHttpRequestStatus::Processing || http_post_req.Get().GetStatus() == EHttpRequestStatus::Succeeded;
}

void UObjectHandler::requestImage(FString URL)
{
	this->syncer->inc();
	this->request_raw_base(URL);
}

void UObjectHandler::requestImages(TArray<FString> URLs)
{
	//this will filter out bad urls saving on compute
	TArray<FString> filteredUrls = this->filterURLs(URLs);
	this->syncer->incN(filteredUrls.Num());//inc for all requests
	for (FString url : filteredUrls)
	{
		this->request_raw_base(url);
	}
}

EImageFormat UObjectHandler::get_img_format(FString URL)
{
	EImageFormat fmt = EImageFormat::PNG;//default!

	if (URL.Contains(".jpg", ESearchCase::IgnoreCase))
		fmt = EImageFormat::JPEG;
	else if (URL.Contains(".png", ESearchCase::IgnoreCase))
		fmt = EImageFormat::PNG;
	else if (URL.Contains(".bmp", ESearchCase::IgnoreCase))
		fmt = EImageFormat::BMP;
	else if (URL.Contains(".hdr", ESearchCase::IgnoreCase))
		fmt = EImageFormat::HDR;
	else if (URL.Contains(".tiff", ESearchCase::IgnoreCase) || URL.Contains(".tif", ESearchCase::IgnoreCase))
		fmt = EImageFormat::TIFF;
	else if (URL.Contains(".tga", ESearchCase::IgnoreCase))
		fmt = EImageFormat::TGA;
	return fmt;
}

UTexture2D* UObjectHandler::tryBuildImage(TArray<uint8> imgData, EImageFormat format)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(format);
	UTexture2D* img = nullptr;
	EPixelFormat pxl_format = PF_B8G8R8A8;
	int32 width = 0, height = 0;

	if (ImageWrapper)
	{
		if (ImageWrapper.Get()->SetCompressed(imgData.GetData(), imgData.Num()))
		{
			TArray64<uint8>  Uncompressed;
			if (ImageWrapper.Get()->GetRaw(Uncompressed))
			{
				width = ImageWrapper.Get()->GetWidth();
				height = ImageWrapper.Get()->GetHeight();

				img = UTexture2D::CreateTransient(width, height, pxl_format);
				if (!img) return nullptr;//nothing to do if it doesn't load!

				void* TextureData = img->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, Uncompressed.GetData(), Uncompressed.Num());
				img->GetPlatformData()->Mips[0].BulkData.Unlock();

				img->UpdateResource();
			}//valid fetch
		}//valid compression
	}//valid image wrapper

	return img;
}

UTexture2D* UObjectHandler::build_img_data(TArray<uint8> img_data,FString URL)
{
	UE_LOG(LogTemp, Display, TEXT("Image size: [%d]"), img_data.Num());

	int32 width = 0, height = 0;
	UTexture2D* img = nullptr;
	EPixelFormat pxl_format = PF_B8G8R8A8;
	EImageFormat img_format;

	if (this->useCustomFormat)
	{
		img_format = this->customFormat;
	}
	else
	{
		img_format = get_img_format(URL);//get the image format nicely
	}

	img = this->tryBuildImage(img_data,img_format);

	if (!img)
	{//try again with forced Jpeg our default choice in the event of no .type specified is .png
		img = this->tryBuildImage(img_data, EImageFormat::JPEG);
	}

	if (!img)
	{//still no dice possible we don't support this in unreal!
		UE_LOG(LogTemp, Warning, TEXT("Bad Image format chosen for: [%s]"), *URL);
	}

	return img;
}