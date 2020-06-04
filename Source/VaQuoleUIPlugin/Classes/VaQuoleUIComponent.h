// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VaQuoleUIPluginPrivatePCH.h"
#include "VaQuoleUIComponent.generated.h"

/**
 * Holds texture data for upload to a rendering resource
 */
struct FVaQuoleTextureData
{
	FVaQuoleTextureData(uint32 InWidth = 0, uint32 InHeight = 0, uint32 InStride = 0, const TArray<uint32>& InBytes = TArray<uint32>())
		: Bytes(InBytes)
		, Width(InWidth)
		, Height(InHeight)
		, StrideBytes(InStride)
	{

	}

	FVaQuoleTextureData(const FVaQuoleTextureData &Other)
		: Bytes(Other.Bytes)
		, Width(Other.Width)
		, Height(Other.Height)
		, StrideBytes(Other.StrideBytes)
	{

	}

	FVaQuoleTextureData& operator=(const FVaQuoleTextureData& Other)
	{
		if (this != &Other)
		{
			SetRawData(Other.Width, Other.Height, Other.StrideBytes, Other.Bytes);
		}
		return *this;
	}

	void SetRawData(uint32 InWidth, uint32 InHeight, uint32 InStride, const TArray<uint32>& InBytes)
	{
		Width = InWidth;
		Height = InHeight;
		StrideBytes = InStride;
		Bytes = InBytes;
	}

	void Empty()
	{
		Bytes.Empty();
	}

	uint32 GetWidth() const { return Width; }
	uint32 GetHeight() const { return Height; }
	uint32 GetStride() const { return StrideBytes; }
	uint32 GetDataSize() const { return Width * Height * StrideBytes; }
	const TArray<uint32>& GetRawBytes() const { return Bytes; }

	/** Accesses the raw bytes of already sized texture data */
	uint32* GetRawBytesPtr() { return Bytes.GetData(); }

private:
	/** Raw uncompressed texture data */
	TArray<uint32> Bytes;

	/** Width of the texture */
	uint32 Width;

	/** Height of the texture */
	uint32 Height;

	/** The number of bytes of each pixel */
	uint32 StrideBytes;

};

typedef TSharedPtr<FVaQuoleTextureData, ESPMode::ThreadSafe> FVaQuoleTextureDataPtr;
typedef TSharedRef<FVaQuoleTextureData, ESPMode::ThreadSafe> FVaQuoleTextureDataRef;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScriptEvalResult, const FString&, EvalUuid, const FString&, EvalResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScriptEvent, const FString&, EventName, const FString&, EventMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoadFinished);


/**
 * Class that handles view of one web page
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class VAQUOLEUIPLUGIN_API UVaQuoleUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVaQuoleUIComponent();

	// Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	// End UActorComponent Interface

	// Begin UObject Interface
	virtual void BeginDestroy() override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITORONLY_DATA
	// End UObject Interface


	//////////////////////////////////////////////////////////////////////////
	// View configuration

	/** Indicates whether the View enabled (receive player input or not) */
	UPROPERTY(EditAnywhere, Category = "View")
	bool bEnabled;

	/** Indicates whether the View is transparent or composed on white */
	UPROPERTY(EditAnywhere, Category = "View")
	bool bTransparent;

	/** Width of target texture */
	UPROPERTY(EditAnywhere, Category = "View", meta = (ClampMin = "0", UIMin = "0", UIMax = "4096"))
	int32 Width;

	/** Height of target texture */
	UPROPERTY(EditAnywhere, Category = "View", meta = (ClampMin = "0", UIMin = "0", UIMax = "4096"))
	int32 Height;

	/** URL that will be opened on startup */
	UPROPERTY(EditAnywhere, Category = "View")
	FString DefaultURL;

	/** Should widget reveice any input? */
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bInputEnabled;

	/** Should widget consume mouse input? (mouse events won't be delivered to other actors) */
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bConsumeMouseInput;

	/** Should widget consume keyboard input? (key events won't be delivered to other actors) */
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bConsumeKeyboardInput;

	/** Material that will be instanced to load UI texture into it */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	UMaterialInterface* BaseMaterial;

	/** Name of parameter to load UI texture into material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	FName TextureParameterName;


	//////////////////////////////////////////////////////////////////////////
	// View control

	/** Changes view activity state. Attn.! qApp will be still active in background! */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	void SetEnabled(bool Enabled);

	/** Changes background transparency */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	void SetTransparent(bool Transparent);

	/** Determine should widget receive input or not */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI|Input")
	void SetInputEnabled(bool EnableInput);

	/** Set consume input state for mouse events */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI|Input")
	void SetConsumeMouseInput(bool ConsumeInput);

	/** Set consume input state for keyboard events */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI|Input")
	void SetConsumeKeyboardInput(bool ConsumeInput);

	/** Resizes the View */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	virtual void Resize(int32 NewWidth, int32 NewHeight);

	/** Evaluates the JavaScript defined by ScriptSource and returns the call UUID
	 * to get the result of the last executed statement from OnScriptReturn callback */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	FString EvaluateJavaScript(const FString& ScriptSource);

	/** Requests a new URL to be loaded in the View */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	void OpenURL(const FString& URL);


	//////////////////////////////////////////////////////////////////////////
	// Content access

	/** Is current view enabled? */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	bool IsEnabled() const;

	/** Is input enabled for widget? */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI|Input")
	bool IsInputEnabled() const;

	/** Texture that stores current widget UI */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	int32 GetWidth() const;

	/** Texture that stores current widget UI */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	int32 GetHeight() const;

	/** Texture that stores current widget UI */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	UTexture2D* GetTexture() const;

	/** Material instance that contains texture inside it */
	UFUNCTION(BlueprintCallable, Category = "UI|VaQuoleUI")
	UMaterialInstanceDynamic* GetMaterialInstance() const;


	//////////////////////////////////////////////////////////////////////////
	// Blueprintable events

	/** Called when JavaScript function evaluated with return value */
	UPROPERTY(BlueprintAssignable)
	FScriptEvalResult ScriptEvalResult;

	/** Called when JavaScript emits a special event for engine */
	UPROPERTY(BlueprintAssignable)
	FScriptEvent ScriptEvent;

	/** Called when desired URL loading is finished */
	UPROPERTY(BlueprintAssignable)
	FLoadFinished LoadFinished;


	//////////////////////////////////////////////////////////////////////////
	// Player input

	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent EventType, float AmountDepressed = 1.f, bool bGamepad = false);

	void SetMousePosition(float X, float Y);

protected:
	/** Send MouseMove event to the widget */
	void UpdateMousePosition();

	/** Cached mouse position */
	FVector2D MouseWidgetPosition;


	//////////////////////////////////////////////////////////////////////////
	// Materials setup

protected:
	/** Release Texture resources */
	void DestroyUITexture();

	/** Release current Texture and create new with desired size */
	void ResetUITexture();

	/** Material instance update */
	void ResetMaterialInstance();
	
	/** Update Texture with WebView cached data */
	void UpdateUITexture();

	/** Texture that stores current widget UI */
	UTexture2D* Texture;

	/** Material instance that contains texture inside it */
	UMaterialInstanceDynamic* MaterialInstance;


	//////////////////////////////////////////////////////////////////////////
	// WebUI setup

	/** Reset all WebUI states to defaults */
	void ResetWebUI();

	/** Get return values of evaluated JS code and emit events */
	void UpdateScriptResults();

	/** Check WebUI for queued events and emit them */
	void UpdateScriptEvents();

	/** Check that desired URL is opened */
	void UpdateLoadingState();

	/** Web view loaded from library */
	VaQuole::VaQuoleWebUI* WebUI;

	/** Is last URL successfully loaded? */
	bool bPageLoaded;


	//////////////////////////////////////////////////////////////////////////
	// Input helpers

	/** Player mouse screen position */
	bool GetMouseScreenPosition(FVector2D& MousePosition);

};
