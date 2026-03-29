// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCallbacks.h

#include "SubstanceCallbacks.h"
#include "SubstanceCorePrivatePCH.h"

#include"substance/framework/graph.h"
#include"substance/framework/output.h"

void Substance::RenderCallbacks::outputComputed(SubstanceAir::UInt Uid, size_t userData, const SubstanceAir::GraphInstance* graph, SubstanceAir::OutputInstance* output)
{
	FScopeLock slock(&mMutex);
	mOutputQueue.AddUnique(output);
}

TArray<SubstanceAir::OutputInstance*> Substance::RenderCallbacks::getComputedOutputs(bool throttleOutputGrab)
{
	FScopeLock slock(&mMutex);
	TArray<SubstanceAir::OutputInstance*> outputs;

	//Note - Throttle outputs used to be true within the editor and only 1 output was updated per frame.
	if (mOutputQueue.Num())
	{
		outputs = mOutputQueue;
		mOutputQueue.Empty();
	}

	return outputs;
}

void Substance::RenderCallbacks::clearComputedOutputs(SubstanceAir::OutputInstance* Output)
{
	FScopeLock slock(&mMutex);
	mOutputQueue.Remove(Output);
}

void Substance::RenderCallbacks::clearAllComputedOutputs()
{
	FScopeLock slock(&mMutex);
	mOutputQueue.Empty();
}