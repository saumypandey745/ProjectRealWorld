#include "ProjectRealWorld.h"

DEFINE_LOG_CATEGORY(LogProjectRealWorld);

void FProjectRealWorldModule::StartupModule()
{
	UE_LOG(LogProjectRealWorld, Log, TEXT("ProjectRealWorld: Module Initialized. Ready for AAA Subsystems."));
}

void FProjectRealWorldModule::ShutdownModule()
{
	UE_LOG(LogProjectRealWorld, Log, TEXT("ProjectRealWorld: Module Shutdown."));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FProjectRealWorldModule, ProjectRealWorld, "ProjectRealWorld");
