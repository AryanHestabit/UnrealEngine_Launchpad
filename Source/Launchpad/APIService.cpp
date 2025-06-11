#include "APIService.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

FString UAPIService::JWTToken = TEXT("");

void UAPIService::SetJWTToken(const FString& Token)
{
    JWTToken = Token;
    UE_LOG(LogTemp, Log, TEXT("JWT Token set: %s"), *Token);
}

void UAPIService::CallGeoAPI()
{
    // Create a GET request to the GeoIP API
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://ipapi.co/json/"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    if (!JWTToken.IsEmpty())
    {
        Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *JWTToken));
        UE_LOG(LogTemp, Log, TEXT("JWT token added to header."));
    }

    Request->OnProcessRequestComplete().BindStatic(&UAPIService::OnGeoResponse);
    Request->ProcessRequest();
}

void UAPIService::OnGeoResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("GeoAPI call failed."));
        return;
    }

    FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("Raw GeoAPI Response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FString IP = JsonObject->GetStringField("ip");
        FString City = JsonObject->GetStringField("city");
        FString Region = JsonObject->GetStringField("region");
        FString Country = JsonObject->GetStringField("country_name");

        UE_LOG(LogTemp, Log, TEXT("IP: %s, City: %s, Region: %s, Country: %s"), *IP, *City, *Region, *Country);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON from GeoAPI response."));
    }
}

