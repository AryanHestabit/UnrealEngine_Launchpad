// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "APIService.generated.h"

/**
 * API Service Blueprint Library for handling HTTP requests
 */
UCLASS()
class LAUNCHPAD_API UAPIService : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Sets the JWT token for authenticated API calls */
    UFUNCTION(BlueprintCallable, Category = "API")
    static void SetJWTToken(const FString& Token);

    /** Calls the GeoIP API and logs location information */
    UFUNCTION(BlueprintCallable, Category = "API")
    static void CallGeoAPI();

private:
    static FString JWTToken;

    /** Internal callback for handling GeoAPI HTTP responses */
    static void OnGeoResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
