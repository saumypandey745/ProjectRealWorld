// ============================================================================
// RealWorldStockMarketSubsystem.h
// Project RealWorld — GTA V LCN & BAWSAQ Dynamic Stock Market Simulation
// Features: Corporate sabotage, rival company price surges,
//           in-game portfolio, buy/sell trading directly via smartphone
// ============================================================================
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RealWorldStockMarketSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FStockCompany
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Ticker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CompanyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentPrice = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChangePercent = 0.0f; // e.g. +12.5%

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RivalTicker; // Corporate rival
};

USTRUCT(BlueprintType)
struct FPlayerStockHolding
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Ticker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 SharesOwned = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AveragePurchasePrice = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStockTransaction, const FString&, Ticker, int32, Shares, float, TotalAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStockPriceChanged, const FString&, Ticker, float, NewPrice);

UCLASS()
class PROJECTREALWORLD_API URealWorldStockMarketSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Trading Actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "StockMarket|Trading")
    bool BuyShares(const FString& Ticker, int32 Shares, int32& OutTotalCost);

    UFUNCTION(BlueprintCallable, Category = "StockMarket|Trading")
    bool SellShares(const FString& Ticker, int32 Shares, int32& OutTotalRevenue);

    UFUNCTION(BlueprintCallable, Category = "StockMarket|Events")
    void ReportCorporateSabotage(const FString& DamagedCompanyTicker);

    // ─── Queries ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "StockMarket|Queries")
    TArray<FStockCompany> GetAllStocks() const;

    UFUNCTION(BlueprintPure, Category = "StockMarket|Queries")
    FStockCompany GetStock(const FString& Ticker) const;

    UFUNCTION(BlueprintPure, Category = "StockMarket|Queries")
    int32 GetSharesOwned(const FString& Ticker) const;

    UFUNCTION(BlueprintPure, Category = "StockMarket|Queries")
    float GetPortfolioTotalValue() const;

    // ─── Delegates ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "StockMarket|Events")
    FOnStockTransaction OnStockTransaction;

    UPROPERTY(BlueprintAssignable, Category = "StockMarket|Events")
    FOnStockPriceChanged OnStockPriceChanged;

private:
    UPROPERTY()
    TMap<FString, FStockCompany> MarketListings;

    UPROPERTY()
    TMap<FString, FPlayerStockHolding> PlayerPortfolio;

    FTimerHandle MarketFlukeTimer;

    void SetupInitialListings();
    void SimulateMarketFluctuations();
};
