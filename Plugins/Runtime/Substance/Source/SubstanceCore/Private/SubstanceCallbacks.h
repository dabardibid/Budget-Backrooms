// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCallbacks.h

#pragma once
#include "substance/framework/typedefs.h"
#include "substance/framework/callbacks.h"

namespace Substance
{
class GraphInstance;
class OutputInstance;

struct RenderCallbacks : public SubstanceAir::RenderCallbacks
{
public:

	/** Call back which is called when a substance has been computed and outputs are ready to be read */
	virtual void outputComputed(SubstanceAir::UInt Uid, size_t userData, const SubstanceAir::GraphInstance*, SubstanceAir::OutputInstance*) override;

	/** Returns the computed outputs from the framework */
	TArray<SubstanceAir::OutputInstance*> getComputedOutputs(bool throttleOutputGrab = true);

	/** Clears a computed output instance stored within the framework */
	void clearComputedOutputs(SubstanceAir::OutputInstance* Output);

	/** Clears all computed outputs */
	void clearAllComputedOutputs();

	/** Checks the queue to determine if more outputs are to be computed */
	bool isOutputQueueEmpty()
	{
		return mOutputQueue.Num() == 0;
	}

protected:
	/** The queue that stores the OutputInstances */
	TArray<SubstanceAir::OutputInstance*> mOutputQueue;

	/** Lock for thread safe operations */
	FCriticalSection mMutex;
};

}// Substance Namespace
