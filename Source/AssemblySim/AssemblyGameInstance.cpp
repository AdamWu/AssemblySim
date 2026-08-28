#include "AssemblyGameInstance.h"

UAssemblyGameInstance::UAssemblyGameInstance()
{
}

void UAssemblyGameInstance::Init()
{
	Super::Init();

	// 在这里进行全局初始化，比如读取外部 SOP 配置文件、初始化 Web 接口等
	ResetSessionData();
}

void UAssemblyGameInstance::ResetSessionData()
{
	TotalScore = 100.0f;
	TotalElapsedTime = 0.0f;
}