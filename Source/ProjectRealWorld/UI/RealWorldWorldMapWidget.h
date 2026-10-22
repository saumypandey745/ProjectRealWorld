// ============================================================================
// RealWorldWorldMapWidget.h
// Project RealWorld — GTA5-class Interactive Fullscreen World Map
// 5 districts, pan & pinch-zoom, custom GPS waypoint dropper, legend filters
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RealWorldWorldMapWidget.generated.h"

class UCanvasPanel;
class UImage;
class UTextBlock;
class UButton;

UENUM(BlueprintType)
enum class EMapMarkerType : uint8
{
    Player          UMETA(DisplayName = "Player Location"),
    Waypoint        UMETA(DisplayName = "Custom GPS Waypoint"),
    StoryMission    UMETA(DisplayName = "Story Mission"),
    AmmuNation      UMETA(DisplayName = "Ammu-Nation"),
    PayNSpray       UMETA(DisplayName = "Pay 'n' Spray"),
    Safehouse       UMETA(DisplayName = "Safehouse"),
    ChopShop        UMETA(DisplayName = "Chop Shop")
};

USTRUCT(BlueprintType)
struct FMapMarkerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MarkerID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMapMarkerType MarkerType = EMapMarkerType::StoryMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Label;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVisible = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGPSWaypointSet, FVector, WaypointWorldLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGPSWaypointCleared);

UCLASS(BlueprintType, Blueprintable)
class PROJECTREALWORLD_API URealWorldWorldMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ─── Touch / Mouse Navigation ────────────────────────────────────────────
    virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // ─── Map Controls ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "WorldMap")
    void SetZoomLevel(float NewZoom);

    UFUNCTION(BlueprintCallable, Category = "WorldMap")
    void PanMap(FVector2D DeltaPan);

    UFUNCTION(BlueprintCallable, Category = "WorldMap")
    void CenterOnPlayer();

    UFUNCTION(BlueprintCallable, Category = "WorldMap")
    void DropCustomWaypoint(FVector2D ScreenPosition);

    UFUNCTION(BlueprintCallable, Category = "WorldMap")
    void ClearCustomWaypoint();

    UFUNCTION(BlueprintCallable, Category = "WorldMap|Filter")
    void SetMarkerFilter(EMapMarkerType FilterType, bool bShow);

    // ─── Coordinate Conversion ───────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "WorldMap")
    FVector2D WorldToMapSpace(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category = "WorldMap")
    FVector MapSpaceToWorld(const FVector2D& MapPosition) const;

    // ─── State ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WorldMap|State")
    float CurrentZoom = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WorldMap|State")
    FVector2D MapPanOffset = FVector2D::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WorldMap|State")
    bool bHasActiveWaypoint = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WorldMap|State")
    FVector ActiveWaypointWorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMap|Markers")
    TArray<FMapMarkerData> Markers;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "WorldMap|Events")
    FOnGPSWaypointSet OnGPSWaypointSet;

    UPROPERTY(BlueprintAssignable, Category = "WorldMap|Events")
    FOnGPSWaypointCleared OnGPSWaypointCleared;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMap|Config")
    float MinZoom = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMap|Config")
    float MaxZoom = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldMap|Config")
    float WorldBoundsSize = 30000.0f; // 300m x 300m world space scaling

    FVector2D LastTouchPosition = FVector2D::ZeroVector;
    bool bIsDraggingMap = false;

    void PopulateDistrictLandmarks();
};
