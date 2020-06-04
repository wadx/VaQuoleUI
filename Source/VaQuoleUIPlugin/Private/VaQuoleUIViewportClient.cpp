// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "VaQuoleUIPluginPrivatePCH.h"

UVaQuoleUIViewportClient::UVaQuoleUIViewportClient()
{
}

void UVaQuoleUIViewportClient::RegisterHudUI(UVaQuoleUIComponent* WebView)
{
	UE_LOG(LogVaQuole, Warning, TEXT("HUD UI registered"));
	HudViews.AddUnique(WebView);
}

void UVaQuoleUIViewportClient::UnregisterHudUI(UVaQuoleUIComponent* WebView)
{
	UE_LOG(LogVaQuole, Log, TEXT("HUD UI unregistered"));
	HudViews.Remove(WebView);
}

void UVaQuoleUIViewportClient::RegisterSceneUI(UVaQuoleUIComponent* WebView)
{
	UE_LOG(LogVaQuole, Log, TEXT("Scene UI registered"));
	SceneViews.AddUnique(WebView);
}

void UVaQuoleUIViewportClient::UnregisterSceneUI(UVaQuoleUIComponent* WebView)
{
	UE_LOG(LogVaQuole, Log, TEXT("Scene UI unregistered"));
	SceneViews.Remove(WebView);
}

//bool UVaQuoleUIViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
bool UVaQuoleUIViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Viewport->IsPlayInEditorViewport() && EventArgs.Key.IsGamepadKey())
	{
		int32 ControllerId = EventArgs.ControllerId;
		GEngine->RemapGamepadControllerIdForPIE(this, ControllerId);
	}

	// Route to subsystems that care
	bool bResult = (ViewportConsole ? ViewportConsole->InputKey(EventArgs.ControllerId, EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.bIsTouchEvent) : false);
	if (!bResult)
	{
		ULocalPlayer* const TargetPlayer = GEngine->GetLocalPlayerFromControllerId(this, EventArgs.ControllerId);
		if (TargetPlayer && TargetPlayer->PlayerController)
		{
			//UE_LOG(LogVaQuole, Warning, TEXT("Key: %s"), *Key.ToString());

			// Process input with HUD views
			for (auto HudView : HudViews)
			{
				bResult = HudView->InputKey(Viewport, EventArgs.ControllerId, EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.bIsTouchEvent);

				// If view consumed input, break the loop
				if (bResult)
				{
					break;
				}
			}

			// Give a chance to UIs we're looking at
			for (auto SceneView : SceneViews)
			{
				bResult = SceneView->InputKey(Viewport, EventArgs.ControllerId, EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.bIsTouchEvent);

				// If view consumed input, break the loop
				if (bResult)
				{
					break;
				}
			}

			// Give a chance to player controller
			if (!bResult)
			{
				bResult = TargetPlayer->PlayerController->InputKey(EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.bIsTouchEvent);
			}
		}

		// A gameviewport is always considered to have responded to a mouse buttons to avoid throttling
		if (!bResult && EventArgs.Key.IsMouseButton())
		{
			bResult = true;
		}
	}

	// For PIE, let the next PIE window handle the input if we didn't
	// (this allows people to use multiple controllers to control each window)
	if (!bResult && EventArgs.ControllerId > 0 && EventArgs.Viewport->IsPlayInEditorViewport())
	{
		UGameViewportClient *NextViewport = GEngine->GetNextPIEViewport(this);
		if (NextViewport)
		{
			bResult = NextViewport->InputKey(EventArgs.Viewport, EventArgs.ControllerId - 1, EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.bIsTouchEvent);
		}
	}

	return bResult;
}
