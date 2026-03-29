//! @file graph.h
//! @brief Substance Air graph structures definition
//! @author Antoine Gonzalez - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//! @version 2.0 Graph I/O are now sorted in GUI order.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_GRAPH_H
#define _SUBSTANCE_AIR_FRAMEWORK_GRAPH_H

#include "output.h"
#include "input.h"
#include "preset.h"





namespace SubstanceAir
{

class PackageDesc;
class Renderer;

namespace Details
{
	class States;
}


//! @brief Substance graph description
//! Each graph has outputs (images computed at rendering) and optional inputs
//! that parametrize rendering results.
struct GraphDesc
{
	//! @brief URL of this graph inside Substance packages tree
	string mPackageUrl;

	//! @brief UI purpose fields, strings in UTF8 format
	//! @{
	string mLabel;
	string mDescription;
	string mCategory;
	string mKeywords;
	string mAuthor;
	string mAuthorUrl;
	string mUserTag;
	//! @}

	//! @brief Container of outputs description
	typedef vector<OutputDesc> Outputs;

	//! @brief The outputs of the graph
	//! @note Sorted by their order in the UI
	Outputs mOutputs;

	//! @brief Container of input description
	//! (the current graph instance has ownership)
	typedef vector<const InputDescBase*> Inputs;

	//! @brief The inputs of the graph
	//! @note Sorted by their order in the UI
	Inputs mInputs;

	//! @brief Array of internal (embedded) input presets
	//! Most often, presets are not embedded but they are stored in separate
	//! files. To loading these files, use parsePresets() in preset.h header.
	//!
	//! These presets are dedicated for this graph, but they can be applied on
	//!	any graph that have at least one similar input (input value with same
	//! identifier and same type).
	Presets mPresets;

	//! Physical Size
	//! @note Filled with 0.0f values if not available in the XML
	Vec3Float mPhysicalSize;

	//! @brief Type for the thumbnail image buffer
	typedef vector<uint8_t> Thumbnail;

	//! @brief Copy of the graph thumbnail as an encoded image
	Thumbnail mThumbnail;

	//! @brief Parent package description
	PackageDesc* mParent;

	//! @brief Default constructor
	GraphDesc();

	//! @brief Copy constructor
	GraphDesc(const GraphDesc&);

	//! @brief Affectation operator
	GraphDesc& operator=(const GraphDesc&);

	//! @brief Destructor
	~GraphDesc();

	//! @brief Helper: Find an output in mOutputs array by its UID
	//! @param uid The uid of the output to retrieve
	//! @return Return the index on the output found or outputs array size
	//!		if unknown uid.
	size_t findOutput(UInt uid) const;

	//! @brief Helper: Find an input in mInputs array by its UID
	//! @param uid The uid of the input to retrieve
	//! @return Return the iterator on the input found or inputs array size
	//!		if unknown uid.
	size_t findInput(UInt uid) const;

	void commitInputs();                         //!< Internal use only
	void commitOutputs();                        //!< Internal use only
protected:
	typedef vector<std::pair<UInt,size_t> > SortedIndices;

	SortedIndices mSortedOutputs, mSortedInputs;

	void clearInputs();
};


//! @brief Substance graph instance
//! A graph instance corresponds to a graph description. Each graph instance
//! is independent: it has its own input values. Graph instances are the
//! objects handled by the renderer (see renderer.h).
class GraphInstance
{
public:
	//! @brief Container of output instances
	typedef vector<OutputInstance*> Outputs;

	//! @brief Container of input instances
	typedef vector<InputInstanceBase*> Inputs;

	//! @brief Constructor from description
	//! @param src Source graph description to instantiate
	//! @invariant The graph description 'src' must remain valid during the
	//! 	instance lifetime.
	GraphInstance(const GraphDesc& src);

	//! @brief Destructor
	~GraphInstance();

	//! @brief Accessor on array of Outputs
	//! @invariant Sorted by order in the UI, match with corresponding outputs
	//!		description (mDesc.mOutputs)
	const Outputs& getOutputs() const { return mOutputs; }

	//! @brief Accessor on array of texture Outputs
	//! @invariant Sorted by order in the UI, match with corresponding outputs
	//!		description (mDesc.mOutputs)
	const Outputs& getTextureOutputs() const { return mTextureOutputs; }

	//! @brief Accessor on array of value Outputs
	//! @invariant Sorted by order in the UI, match with corresponding outputs
	//!		description (mDesc.mOutputs)
	const Outputs& getValueOutputs() const { return mValueOutputs; }

	//! @brief Accessor on array of Inputs
	//! @invariant Sorted by order in the UI, match with corresponding inputs
	//!		description (mDesc.mInputs)
	const Inputs& getInputs() const { return mInputs; }

	//! @brief Helper: Find an output by its UID
	//! @param uid The uid of the output to retrieve
	//! @return Return the pointer on the output found or nullptr if unknown uid.
	OutputInstance* findOutput(UInt uid) const;

	//! @brief Helper: Find an input by its UID
	//! @param uid The uid of the input to retrieve
	//! @return Return the pointer on the input found or nullptr if unknown uid.
	InputInstanceBase* findInput(UInt uid) const;

	//! @brief Helper: Create a new output instance from a format override
	//! @param fmt the OutputFormat to use to initialize this output instance
	//! @param desc the OutputDesc to attach to the new OutputInstance
	//! @return Return the pointer of the OutputInstance created
	OutputInstance* createOutput(const OutputFormat& fmt, const OutputDesc& desc);

	//! @brief Reference on corresponding graph description
	const GraphDesc &mDesc;

	//! @brief UID of this instance
	//! Each instance has a different UID in the current execution context.
	const UInt mInstanceUid;

	//! @brief User data, can be used to store pointer or other stuff
	//! Useful in callback functions.
	size_t mUserData;

	void plugState(Details::States*);         //!< Internal use only
	void unplugState(Details::States*);       //!< Internal use only
protected:
	typedef vector<Details::States*> States;
	typedef deque<OutputDesc> OutputDescs;

	Outputs mOutputs;
	Outputs mTextureOutputs; // Shallow copy of all texture outputs
	Outputs mValueOutputs;   // Shallow copy of all value outputs
	Inputs mInputs;
	States mStates;
	OutputDescs mOutputDescs;
};


//! @brief Shared pointer on graph instances
typedef shared_ptr<GraphInstance> GraphInstanceSPtr;

//! @brief Container of graph instances
typedef vector<GraphInstanceSPtr> GraphInstances;


} // namespace SubstanceAir

#endif //_SUBSTANCE_AIR_FRAMEWORK_GRAPH_H
