// ============================================================================
// RealWorldStockMarketSubsystem.cpp
// Project RealWorld — GTA V LCN & BAWSAQ Dynamic Stock Market Simulation
// ============================================================================

#include "Economy/RealWorldStockMarketSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void URealWorldStockMarketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SetupInitialListings();

    // Natural market fluctuations every 15 seconds
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MarketFlukeTimer,
            this, &URealWorldStockMarketSubsystem::SimulateMarketFluctuations,
            15.0f,
            true
        );
    }
}

void URealWorldStockMarketSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MarketFlukeTimer);
    }
    MarketListings.Empty();
    PlayerPortfolio.Empty();
    Super::Deinitialize();
}

void URealWorldStockMarketSubsystem::SetupInitialListings()
{
    MarketListings.Empty();

    auto AddListing = [this](const FString& Ticker, const FString& Name, float Price, const FString& Rival)
    {
        FStockCompany Co;
        Co.Ticker = Ticker;
        Co.CompanyName = Name;
        Co.CurrentPrice = Price;
        Co.ChangePercent = 0.0f;
        Co.RivalTicker = Rival;
        MarketListings.Add(Ticker, Co);
    };

    // Iconic GTA parody corporations
    AddListing(TEXT("LFI"), TEXT("Lifeinvader Social"), 240.50f, TEXT("EYE"));
    AddListing(TEXT("EYE"), TEXT("Eyefind Search"), 185.20f, TEXT("LFI"));
    AddListing(TEXT("CLU"), TEXT("Cluckin' Bell Poultry"), 65.40f, TEXT("BUR"));
    AddListing(TEXT("BUR"), TEXT("Burger Shot Fast Food"), 82.10f, TEXT("CLU"));
    AddListing(TEXT("AUG"), TEXT("Augury Insurance"), 310.00f, TEXT("VAP"));
    AddListing(TEXT("VAP"), TEXT("Vapid Motors"), 145.75f, TEXT("AUG"));
    AddListing(TEXT("MAZ"), TEXT("Maze Bank International"), 420.00f, TEXT(""));
}

void URealWorldStockMarketSubsystem::SimulateMarketFluctuations()
{
    for (auto& Pair : MarketListings)
    {
        FStockCompany& Co = Pair.Value;
        // Natural jitter between -2.5% and +2.5%
        const float DeltaPct = FMath::RandRange(-0.025f, 0.025f);
        const float OldPrice = Co.CurrentPrice;
        Co.CurrentPrice = FMath::Max(Co.CurrentPrice * (1.0f + DeltaPct), 1.0f);
        Co.ChangePercent = ((Co.CurrentPrice - OldPrice) / OldPrice) * 100.0f;

        OnStockPriceChanged.Broadcast(Co.Ticker, Co.CurrentPrice);
    }
}

bool URealWorldStockMarketSubsystem::BuyShares(const FString& Ticker, int32 Shares, int32& OutTotalCost)
{
    OutTotalCost = 0;
    if (Shares <= 0 || !MarketListings.Contains(Ticker)) return false;

    const FStockCompany& Co = MarketListings[Ticker];
    OutTotalCost = FMath::RoundToInt(Co.CurrentPrice * Shares);

    FPlayerStockHolding& Holding = PlayerPortfolio.FindOrAdd(Ticker);
    Holding.Ticker = Ticker;
    
    // Weighted average purchase price
    const float TotalSpentBefore = Holding.SharesOwned * Holding.AveragePurchasePrice;
    const float TotalSpentNew = OutTotalCost;
    Holding.SharesOwned += Shares;
    Holding.AveragePurchasePrice = (TotalSpentBefore + TotalSpentNew) / Holding.SharesOwned;

    OnStockTransaction.Broadcast(Ticker, Shares, OutTotalCost);
    return true;
}

bool URealWorldStockMarketSubsystem::SellShares(const FString& Ticker, int32 Shares, int32& OutTotalRevenue)
{
    OutTotalRevenue = 0;
    if (Shares <= 0 || !MarketListings.Contains(Ticker) || !PlayerPortfolio.Contains(Ticker)) return false;

    FPlayerStockHolding& Holding = PlayerPortfolio[Ticker];
    if (Holding.SharesOwned < Shares) return false;

    const FStockCompany& Co = MarketListings[Ticker];
    OutTotalRevenue = FMath::RoundToInt(Co.CurrentPrice * Shares);
    Holding.SharesOwned -= Shares;

    if (Holding.SharesOwned <= 0)
    {
        PlayerPortfolio.Remove(Ticker);
    }

    OnStockTransaction.Broadcast(Ticker, -Shares, OutTotalRevenue);
    return true;
}

void URealWorldStockMarketSubsystem::ReportCorporateSabotage(const FString& DamagedCompanyTicker)
{
    if (!MarketListings.Contains(DamagedCompanyTicker)) return;

    FStockCompany& Victim = MarketListings[DamagedCompanyTicker];
    
    // Victim crashes by 35%
    Victim.CurrentPrice = FMath::Max(Victim.CurrentPrice * 0.65f, 1.0f);
    Victim.ChangePercent = -35.0f;
    OnStockPriceChanged.Broadcast(Victim.Ticker, Victim.CurrentPrice);

    // Rival company surges by 50%
    if (!Victim.RivalTicker.IsEmpty() && MarketListings.Contains(Victim.RivalTicker))
    {
        FStockCompany& Rival = MarketListings[Victim.RivalTicker];
        Rival.CurrentPrice = Rival.CurrentPrice * 1.50f;
        Rival.ChangePercent = +50.0f;
        OnStockPriceChanged.Broadcast(Rival.Ticker, Rival.CurrentPrice);
    }
}

TArray<FStockCompany> URealWorldStockMarketSubsystem::GetAllStocks() const
{
    TArray<FStockCompany> Result;
    MarketListings.GenerateValueArray(Result);
    return Result;
}

FStockCompany URealWorldStockMarketSubsystem::GetStock(const FString& Ticker) const
{
    if (const FStockCompany* Found = MarketListings.Find(Ticker))
    {
        return *Found;
    }
    return FStockCompany();
}

int32 URealWorldStockMarketSubsystem::GetSharesOwned(const FString& Ticker) const
{
    if (const FPlayerStockHolding* Found = PlayerPortfolio.Find(Ticker))
    {
        return Found->SharesOwned;
    }
    return 0;
}

float URealWorldStockMarketSubsystem::GetPortfolioTotalValue() const
{
    float Total = 0.0f;
    for (const auto& Pair : PlayerPortfolio)
    {
        if (const FStockCompany* Co = MarketListings.Find(Pair.Key))
        {
            Total += Pair.Value.SharesOwned * Co->CurrentPrice;
        }
    }
    return Total;
}
