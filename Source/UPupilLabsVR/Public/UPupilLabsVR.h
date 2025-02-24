// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUPupilLabsVRModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	/** Handle to the test dll we will load */
	void*	LibZmqDLLHandle;
	void* MsgPackHandle;
	FString	DLLPathLibZmq;
	FString	DLLPathMsgPack;
	
};