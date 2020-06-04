// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "VaQuoleBlueprintFunctionLibrary.h"
#include "VaQuoleUIPluginPrivatePCH.h"

bool UVaQuoleBlueprintFunctionLibrary::ActorHasWebUI(UObject* WorldContextObject, const AActor *Actor, UVaQuoleUIComponent*& WebUI)
{
	if (Actor == nullptr)
	{
		return false;
	}

	WebUI = Actor->FindComponentByClass<UVaQuoleUIComponent>();
	if (WebUI != nullptr)
	{
		return true;
	}

	return false;
}
