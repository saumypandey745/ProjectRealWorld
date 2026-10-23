// ============================================================================
// RealWorldWorldMapWidget.cpp
// Project RealWorld — GTA5-class Interactive Fullscreen World Map
// ============================================================================

#include "UI/RealWorldWorldMapWidget.h"
#include "Character/RealWorldCharacter.h"
#include "Kismet/GameplayStatics.h"

void URealWorldWorldMapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CurrentZoom = 1.0f;
    MapPanOffset = FVector2D::ZeroVector;
    bHasActiveWaypoint = false;
    bIsDraggingMap = false;

    PopulateDistrictLandmarks();
    CenterOnPlayer();
}

void URealWorldWorldMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update real-time player position marker
    if (ARealWorldCharacter* Player = Cast<ARealWorldCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
    {
        for (FMapMarkerData& Marker : Markers)
        {
            if (Marker.MarkerType == EMapMarkerType::Player)
            {
                Marker.WorldLocation = Player->GetActorLocation();
                break;
            }
        }
    }
}

FReply URealWorldWorldMapWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
    LastTouchPosition = InGestureEvent.GetScreenSpacePosition();
    bIsDraggingMap = true;
    return FReply::Handled();
}

FReply URealWorldWorldMapWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
    if (bIsDraggingMap)
    {
        const FVector2D CurrentPos = InGestureEvent.GetScreenSpacePosition();
        const FVector2D Delta = CurrentPos - LastTouchPosition;
        PanMap(Delta);
        LastTouchPosition = CurrentPos;
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply URealWorldWorldMapWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
    bIsDraggingMap = false;
    return FReply::Handled();
}

void URealWorldWorldMapWidget::SetZoomLevel(float NewZoom)
{
    CurrentZoom = FMath::Clamp(NewZoom, MinZoom, MaxZoom);
}

void URealWorldWorldMapWidget::PanMap(FVector2D DeltaPan)
{
    MapPanOffset += DeltaPan / CurrentZoom;
}

void URealWorldWorldMapWidget::CenterOnPlayer()
{
    if (ARealWorldCharacter* Player = Cast<ARealWorldCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
    {
        const FVector2D PlayerMapPos = WorldToMapSpace(Player->GetActorLocation());
        MapPanOffset = -PlayerMapPos;
    }
}

void URealWorldWorldMapWidget::DropCustomWaypoint(FVector2D ScreenPosition)
{
    ActiveWaypointWorldLocation = MapSpaceToWorld(ScreenPosition - MapPanOffset);
    bHasActiveWaypoint = true;

    // Update waypoint marker
    bool bFound = false;
    for (FMapMarkerData& Marker : Markers)
    {
        if (Marker.MarkerType == EMapMarkerType::Waypoint)
        {
            Marker.WorldLocation = ActiveWaypointWorldLocation;
            Marker.bIsVisible = true;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        FMapMarkerData WpMarker;
        WpMarker.MarkerID = TEXT("CustomWaypoint");
        WpMarker.MarkerType = EMapMarkerType::Waypoint;
        WpMarker.WorldLocation = ActiveWaypointWorldLocation;
        WpMarker.Label = FText::FromString(TEXT("Target Waypoint"));
        WpMarker.bIsVisible = true;
        Markers.Add(WpMarker);
    }

    OnGPSWaypointSet.Broadcast(ActiveWaypointWorldLocation);
}

void URealWorldWorldMapWidget::ClearCustomWaypoint()
{
    bHasActiveWaypoint = false;
    for (FMapMarkerData& Marker : Markers)
    {
        if (Marker.MarkerType == EMapMarkerType::Waypoint)
        {
            Marker.bIsVisible = false;
        }
    }
    OnGPSWaypointCleared.Broadcast();
}

void URealWorldWorldMapWidget::SetMarkerFilter(EMapMarkerType FilterType, bool bShow)
{
    for (FMapMarkerData& Marker : Markers)
    {
        if (Marker.MarkerType == FilterType)
        {
            Marker.bIsVisible = bShow;
        }
    }
}

FVector2D URealWorldWorldMapWidget::WorldToMapSpace(const FVector& WorldLocation) const
{
    const float HalfBounds = WorldBoundsSize * 0.5f;
    const float NormalizedX = (WorldLocation.X + HalfBounds) / WorldBoundsSize;
    const float NormalizedY = (WorldLocation.Y + HalfBounds) / WorldBoundsSize;
    return FVector2D(NormalizedX * 1024.0f, NormalizedY * 1024.0f);
}

FVector URealWorldWorldMapWidget::MapSpaceToWorld(const FVector2D& MapPosition) const
{
    const float HalfBounds = WorldBoundsSize * 0.5f;
    const float WorldX = (MapPosition.X / 1024.0f) * WorldBoundsSize - HalfBounds;
    const float WorldY = (MapPosition.Y / 1024.0f) * WorldBoundsSize - HalfBounds;
    return FVector(WorldX, WorldY, 0.0f);
}

void URealWorldWorldMapWidget::PopulateDistrictLandmarks()
{
    Markers.Empty();

    // Player Marker
    {
        FMapMarkerData Marker;
        Marker.MarkerID = TEXT("PlayerLocation");
        Marker.MarkerType = EMapMarkerType::Player;
        Marker.Label = FText::FromString(TEXT("You"));
        Marker.bIsVisible = true;
        Markers.Add(Marker);
    }
    // District 1: Skyline Core (Downtown)
    {
        FMapMarkerData Marker;
        Marker.MarkerID = TEXT("AmmuNation_Skyline");
        Marker.MarkerType = EMapMarkerType::AmmuNation;
        Marker.WorldLocation = FVector(0.0f, 0.0f, 0.0f);
        Marker.Label = FText::FromString(TEXT("Ammu-Nation (Skyline Core)"));
        Marker.bIsVisible = true;
        Markers.Add(Marker);
    }
    // District 2: Old Docks
    {
        FMapMarkerData Marker;
        Marker.MarkerID = TEXT("Mission_HarborHeist");
        Marker.MarkerType = EMapMarkerType::StoryMission;
        Marker.WorldLocation = FVector(-6800.0f, 9100.0f, 0.0f);
        Marker.Label = FText::FromString(TEXT("Heist: Harbor Vault"));
        Marker.bIsVisible = true;
        Markers.Add(Marker);
    }
    // District 3: Eastgate Industrial
    {
        FMapMarkerData Marker;
        Marker.MarkerID = TEXT("PayNSpray_Eastgate");
        Marker.MarkerType = EMapMarkerType::PayNSpray;
        Marker.WorldLocation = FVector(5500.0f, -3500.0f, 0.0f);
        Marker.Label = FText::FromString(TEXT("Pay 'n' Spray Garage"));
        Marker.bIsVisible = true;
        Markers.Add(Marker);
    }
    // District 4: Palm Ridge Beach
    {
        FMapMarkerData Marker;
        Marker.MarkerID = TEXT("Safehouse_PalmRidge");
        Marker.MarkerType = EMapMarkerType::Safehouse;
        Marker.WorldLocation = FVector(-8000.0f, -6000.0f, 0.0f);
        Marker.Label = FText::FromString(TEXT("Beachfront Condo Safehouse"));
        Marker.bIsVisible = true;
        Markers.Add(Marker);
    }
    // District 5: Highland Pass
    {
        FMapMarkerData Marker;
        Marker.MarkerID = TEXT("ChopShop_Highland");
        Marker.MarkerType = EMapMarkerType::ChopShop;
        Marker.WorldLocation = FVector(6000.0f, 8000.0f, 0.0f);
        Marker.Label = FText::FromString(TEXT("Highland Canyon Chop Shop"));
        Marker.bIsVisible = true;
        Markers.Add(Marker);
    }
}
