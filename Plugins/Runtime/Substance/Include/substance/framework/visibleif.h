//! @file visibleif.h
//! @brief VisibleIf I/O expression evaluator method
//! @author Dan Stover - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_VISIBLEIF_H
#define _SUBSTANCE_AIR_FRAMEWORK_VISIBLEIF_H

#include <queue>
#include <stack>
#include <cctype>
#include <cstdio>

namespace SubstanceAir
{
// List of operators, globally ordered by precedence groups
typedef enum
{
	OpTypeOr,          // Logical operations on bool

	OpTypeAnd,         // AND has higher priority than OR

	OpTypeCompEQ,      // Equal/Diff comparisons
	OpTypeCompNEQ,

	OpTypeCompGT,      // Ordering comparisons
	OpTypeCompGE,
	OpTypeCompLT,
	OpTypeCompLE,

	OpTypePlus,        // Low priority arithmetics
	OpTypeMinus,

	OpTypeMul,         // High priority arithmetics
	OpTypeDiv,

	OpTypeUnaryMinus,  // Unary minus/plus
	OpTypeUnaryPlus,
	OpTypeNot,         // Negate

	OpTypeLeftParen,   // L&R Parentheses
	OpTypeRightParen,

	OpTypeGetComp1,    // Read of an input's 1st, 2nd, 3rd or 4th component
	OpTypeGetComp2,
	OpTypeGetComp3,
	OpTypeGetComp4,
	OpTypeCount
} OperatorType;

static int OperatorPrecedence[] =
{
	0,          // OR
	1,          // AND
	2, 2,       // EQ/NEQ
	3, 3, 3, 3, // GT/GE/LT/LE
	4, 4,       // +/-
	5, 5,       // */div
	6, 6, 6,    // u-/u+/!
	7, 7,       // (/)
	8, 8, 8, 8  // .x/.y/.z/.w
};

typedef enum
{
	NodeBoolValue,
	NodeFloatValue,
	NodeSubstanceInput,
	NodeOperator
} NodeType;

typedef struct
{
	NodeType type;
	OperatorType operatorType;
	union
	{
		bool boolValue;
		float floatValue;
		const SubstanceAir::InputInstanceBase* substanceInputValue;
	} content;
} Node;

typedef std::queue<Node> NodeQueue;
typedef std::stack<Node> NodeStack;

static bool asBool(const Node& node)
{
	switch (node.type)
	{
	case NodeBoolValue:
		return node.content.boolValue;
	case NodeFloatValue:
		return node.content.floatValue != 0.0f;
	case NodeSubstanceInput:
	case NodeOperator:
	default:
		return true;
	}
}

static float asFloat(const Node& node)
{
	switch (node.type)
	{
	case NodeBoolValue:
		return node.content.boolValue ? 1.0f : 0.0f;
	case NodeFloatValue:
		return node.content.floatValue;
	case NodeSubstanceInput:
	case NodeOperator:
	default:
		return 1.0f;
	}
}

#define BUILD_OPERATOR_NODE(node, opType) \
	(node)->type = NodeOperator; \
	(node)->operatorType = (opType);

#define TEST_AND_BUILD_1CHAR_OPERATOR(token, opChar, node, opType) \
	if (token[0] == opChar) \
	{ \
		BUILD_OPERATOR_NODE(node, opType); \
		return true; \
	}

#define TEST_AND_BUILD_2CHAR_OPERATOR(token, opString, node, opType) \
	if (strncmp(token, opString, 2) == 0) \
	{ \
		BUILD_OPERATOR_NODE(node, opType); \
		return true; \
	}

//! User defined functor to obtain values from the input identifier
class VisibleIfFunctor
{
public:
	typedef Node ValueType;

public:
	virtual ~VisibleIfFunctor() {}
	//! Return false if the input identifier cannot be found in user data
	virtual bool isValid(const char* inputIdentifier) const = 0;
	//! Return the user data value for the input identifier at the given index
	//! If index is not valid, then indexInvalid must be set to 'true'
	virtual ValueType operator()(const char* inputIdentifier, unsigned int inputIndex, bool& indexInvalid) const = 0;
};

//! The idea here would be to pass each graph here based on the integration.
//! The developer can then determine what to do with the graph when its not visible
class VisibleIf
{
public:
	//! Entry point to determine if an input is visible
	static bool EvalVisibleIf(const SubstanceAir::InputInstanceBase* substanceInput);

	//! Entry point to determine if an input is visible
	static bool EvalVisibleIf(
		const char* inputIdentifier,
		const VisibleIfFunctor& functor);

protected:
	//! Construct the proper ordered queue to evaluate
	static NodeQueue BuildEvalQueue(const char* inputIdentifier, const VisibleIfFunctor& functor);

	//! Evaluate queue to get VisibleIf
	static bool ProcessEvalQueue(NodeQueue& queue);

	//! Gets the current token
	static bool GetToken(const char* ptr, Node* node, int* consumedChars, bool* abort, const VisibleIfFunctor& functor);

	//! Get component count of an input value
	static int GetRequiredInputComponentCount(const SubstanceAir::InputInstanceBase* input);

	//! Checks a node to see if it is a command
	static bool IsCommand(const Node& node);

	//! Resolves Substance Input Node
	static void ResolveSubstanceInputNode(Node* node);

	static SubstanceAir::InputInstanceBase* FindSubstanceInput(const SubstanceAir::GraphInstance* Graph, const char* Input);

private:
	//! Default functor
	//! will be used if the user eval a substance air input
	class DefaultFunctor : public VisibleIfFunctor
	{
	public:
		DefaultFunctor(const SubstanceAir::InputInstanceBase* input) : input_(input) {}
		virtual bool isValid(const char* inputIdentifier) const
		{
			return VisibleIf::FindSubstanceInput(&input_->mParentGraph, inputIdentifier) != NULL;
		}
		virtual ValueType operator()(const char* inputIdentifier, unsigned int inputIndex, bool& indexInvalid) const
		{
			indexInvalid = false;
			Node node;
			SubstanceAir::InputInstanceBase* substanceInput = VisibleIf::FindSubstanceInput(&input_->mParentGraph, inputIdentifier);
			// Not possible since the isValid call must be done before
			assert(substanceInput != NULL);
			node.type = NodeSubstanceInput;
			node.content.substanceInputValue = substanceInput;
			node.operatorType = (OperatorType) (OpTypeGetComp1 + inputIndex);

			if (inputIndex + 1 > (unsigned int) VisibleIf::GetRequiredInputComponentCount(substanceInput))
			{
				indexInvalid = true;
			}

			return node;
		}
	private:
		const SubstanceAir::InputInstanceBase* input_;
	};

private:
	//! Lock instance creation
	VisibleIf();
	~VisibleIf();
};

//! Find a given input of a graph instance based on the identifier string
inline SubstanceAir::InputInstanceBase* VisibleIf::FindSubstanceInput(const SubstanceAir::GraphInstance* Graph, const char* Input)
{
	SubstanceAir::GraphInstance::Inputs::const_iterator GraphItr = Graph->getInputs().begin();
	for (; GraphItr != Graph->getInputs().end(); ++GraphItr)
	{
		if ((*GraphItr)->mDesc.mIdentifier == Input)
		{
			return (*GraphItr);
		}
	}

	return NULL;
}

inline bool VisibleIf::IsCommand(const Node& node)
{
	return node.type == NodeOperator && node.operatorType <= OpTypeLeftParen;
}

inline void VisibleIf::ResolveSubstanceInputNode(Node* node)
{
	const SubstanceAir::InputInstanceBase* input = node->content.substanceInputValue;

	//! @note - This is currently the best way to determine if this is a boolean value.
	if (input->mDesc.mGuiWidget == Input_Togglebutton && input->mDesc.mType == Substance_IOType_Integer)
	{
		//! This should be an int value in this case
		//! Cast to numerical input to get value
		const SubstanceAir::InputInstanceNumerical<int>* Numeric =
		    static_cast<const SubstanceAir::InputInstanceNumerical<int>*>(input);
		node->content.boolValue = Numeric->getValue() != 0.0f;
		node->type = NodeBoolValue;
	}
	else
	{
		//! Get the value based on all of the other types
		switch (input->mDesc.mType)
		{
		case Substance_IOType_Float:
			{
				const SubstanceAir::InputInstanceNumerical<float>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<float>*>(input);
				node->content.floatValue = Numeric->getValue();
				break;
			}
		case Substance_IOType_Float2:
			{
				const SubstanceAir::InputInstanceNumerical<Vec2Float>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<Vec2Float>*>(input);
				node->content.floatValue = Numeric->getValue()[(int)node->operatorType - OpTypeGetComp1];
				break;
			}
		case Substance_IOType_Float3:
			{
				const SubstanceAir::InputInstanceNumerical<Vec3Float>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<Vec3Float>*>(input);
				node->content.floatValue = Numeric->getValue()[(int)node->operatorType - OpTypeGetComp1];
				break;
			}
		case Substance_IOType_Float4:
			{
				const SubstanceAir::InputInstanceNumerical<Vec4Float>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<Vec4Float>*>(input);
				node->content.floatValue = Numeric->getValue()[(int)node->operatorType - OpTypeGetComp1];
				break;
			}
		case Substance_IOType_Integer:
			{
				const SubstanceAir::InputInstanceNumerical<int>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<int>*>(input);
				node->content.floatValue = (float)Numeric->getValue();
				break;
			}
		case Substance_IOType_Integer2:
			{
				const SubstanceAir::InputInstanceNumerical<Vec2Int>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<Vec2Int>*>(input);
				node->content.floatValue = (float)Numeric->getValue()[(int)node->operatorType - OpTypeGetComp1];
				break;
			}
		case Substance_IOType_Integer3:
			{
				const SubstanceAir::InputInstanceNumerical<Vec3Int>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<Vec3Int>*>(input);
				node->content.floatValue = (float)Numeric->getValue()[(int)node->operatorType - OpTypeGetComp1];
				break;
			}
		case Substance_IOType_Integer4:
			{
				const SubstanceAir::InputInstanceNumerical<Vec4Int>* Numeric =
				    static_cast<const SubstanceAir::InputInstanceNumerical<Vec4Int>*>(input);
				node->content.floatValue = (float)Numeric->getValue()[(int)node->operatorType - OpTypeGetComp1];
				break;
			}
		default:
			node->content.floatValue = 0.f;
		}

		node->type = NodeFloatValue;
	}
}

inline int VisibleIf::GetRequiredInputComponentCount(const SubstanceAir::InputInstanceBase* input)
{
	switch (input->mDesc.mType)
	{
	case Substance_IOType_Float:
		return 1;
	case Substance_IOType_Float2:
		return 2;
	case Substance_IOType_Float3:
		return 3;
	case Substance_IOType_Float4:
		return 4;
	case Substance_IOType_Integer:
		return 1;
	case Substance_IOType_Integer2:
		return 2;
	case Substance_IOType_Integer3:
		return 3;
	case Substance_IOType_Integer4:
		return 4;
	case Substance_IOType_Font:
	case Substance_IOType_Image:
	case Substance_IOType_String:
		return 1;
	}

	return 1;
}

//! @todo:: This needs to be passed a graph instance in order to find a given input
inline bool VisibleIf::GetToken(const char* ptr, Node* node, int* consumedChars, bool* abort, const VisibleIfFunctor& functor)
{
	// '+' and '-' need to be caught before float values, otherwise when confronted with '5+2',
	// the parser will output a node with 5, and a node with 2, and the plus in the middle will
	// disappear.
	char plusMinusString[2];
	plusMinusString[0] = '\0';
	if (sscanf(ptr, "%1s%n", plusMinusString, consumedChars) > 0)
	{
		TEST_AND_BUILD_1CHAR_OPERATOR(plusMinusString, '+', node, OpTypePlus);
		TEST_AND_BUILD_1CHAR_OPERATOR(plusMinusString, '-', node, OpTypeMinus);
	}

	// Grab a float/int numerical value
	float f;
	if (sscanf(ptr, "%f%n", &f, consumedChars) > 0)
	{
		node->type = NodeFloatValue;
		node->content.floatValue = f;
		return true;
	}

	// Grab 2-chars operators
	char opString[3];
	opString[0] = '\0';
	if (sscanf(ptr, "%2s%n", opString, consumedChars) > 0)
	{
		TEST_AND_BUILD_2CHAR_OPERATOR(opString, "||", node, OpTypeOr);
		TEST_AND_BUILD_2CHAR_OPERATOR(opString, "&&", node, OpTypeAnd);
		TEST_AND_BUILD_2CHAR_OPERATOR(opString, "==", node, OpTypeCompEQ);
		TEST_AND_BUILD_2CHAR_OPERATOR(opString, "!=", node, OpTypeCompNEQ);
		TEST_AND_BUILD_2CHAR_OPERATOR(opString, ">=", node, OpTypeCompGE);
		TEST_AND_BUILD_2CHAR_OPERATOR(opString, "<=", node, OpTypeCompLE);
	}

	// Grab 1-char operators
	opString[0] = '\0';
	if (sscanf(ptr, "%1s%n", opString, consumedChars) > 0)
	{
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '>', node, OpTypeCompGT);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '<', node, OpTypeCompLT);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '*', node, OpTypeMul);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '/', node, OpTypeDiv);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '(', node, OpTypeLeftParen);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, ')', node, OpTypeRightParen);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '!', node, OpTypeNot);

		// Some Substances published by Allegorithmic use [,] instead of (,) to group expressions :(
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, '[', node, OpTypeLeftParen);
		TEST_AND_BUILD_1CHAR_OPERATOR(opString, ']', node, OpTypeRightParen);
	}

	// Try to grab input["MyInput"] or input.MyInput first, then try to grab the suffix
	char inputParam[128] = { '\0' };
	if (sscanf(ptr, " input[\"%127[a-zA-Z0-9_$]\"]%n", inputParam, consumedChars) > 0 ||
	        sscanf(ptr, " input.%127[a-zA-Z0-9_$]%n", inputParam, consumedChars) > 0)
	{
		if (!functor.isValid(inputParam))
		{
			// Defaulting to a 'true' bool for missing inputs
			// This is usually caused by referencing inputs that are not used in the Substance graph itself but that
			// are referenced in visibleIf expressions to trigger the generation of some outputs and the visibility
			// of inputs related to these 'conditional' outputs. These inputs are currently discarded by the Substance
			// Cooker, which is the real underlying bug and which will be fixed in a future Substance Designer release.
			node->type = NodeBoolValue;
			node->content.boolValue = true;
			return true;
		}

		// Try to grab a potential .[xyzw] suffix
		const char* suffixPtr = ptr + *consumedChars;
		char component[2] = { '\0' };
		int suffixConsumedChars;
		bool invalidIndex = false;
		if (sscanf(suffixPtr, ".%1[xyzw]%n", component, &suffixConsumedChars) > 0)
		{
			switch (component[0])
			{
			case 'x':
				*node = functor(inputParam, 0, invalidIndex);
				break;
			case 'y':
				*node = functor(inputParam, 1, invalidIndex);
				break;
			case 'z':
				*node = functor(inputParam, 2, invalidIndex);
				break;
			case 'w':
				*node = functor(inputParam, 3, invalidIndex);
				break;
			}

			*consumedChars += suffixConsumedChars;
		}
		else
		{
			*node = functor(inputParam, 0, invalidIndex);
		}

		if (invalidIndex)
		{
			*abort = true;
			return false;
		}
		return true;
	}

	// Grab a literal bool (true / false)
	char boolString[6];
	boolString[0] = '\0';
	if (sscanf(ptr, "%4s%n", boolString, consumedChars) > 0)
	{
		if (strncmp("true", boolString, 4) == 0)
		{
			node->type = NodeBoolValue;
			node->content.boolValue = true;
			return true;
		}
	}
	boolString[0] = '\0';
	if (sscanf(ptr, "%5s%n", boolString, consumedChars) > 0)
	{
		if (strncmp("false", boolString, 5) == 0)
		{
			node->type = NodeBoolValue;
			node->content.boolValue = false;
			return true;
		}
	}

	// If the input string is not empty and we reach this point, this means a token was not recognized
	const char* checkPtr = ptr;
	while (*checkPtr && std::isspace(*checkPtr))
	{
		checkPtr++;
	}
	if (*checkPtr != '\0')
	{
		*abort = true;
	}

	return false;
}

inline NodeQueue VisibleIf::BuildEvalQueue(const char* inputIdentifier, const VisibleIfFunctor& functor)
{
	NodeQueue outputQueue;
	NodeStack operatorStack;

	Node node, lastNode;
	// Initialize lastNode to an 'operator' with a command type to allow the fix up of
	// a unary plus/minus operator appearing right at the beginning of the string.
	lastNode.type = NodeOperator;
	lastNode.operatorType = OpTypeLeftParen;

	int consumedChars = 0;
	bool abort = false;
	while (GetToken(inputIdentifier, &node, &consumedChars, &abort, functor))
	{
		if (abort)
		{
			while (!outputQueue.empty())
			{
				outputQueue.pop();
			}
			return outputQueue;
		}

		// Fix up unary plus/minus operators
		if (node.type == NodeOperator && IsCommand(lastNode))
		{
			if (node.operatorType == OpTypeMinus)
			{
				BUILD_OPERATOR_NODE(&node, OpTypeUnaryMinus);
			}
			else if (node.operatorType == OpTypePlus)
			{
				BUILD_OPERATOR_NODE(&node, OpTypeUnaryPlus);
			}
		}

		// The core of the shunting-yard algorithm is implemented below
		if (node.type != NodeOperator)
		{
			outputQueue.push(node);
		}
		else
		{
			if (node.operatorType == OpTypeLeftParen)
			{
				operatorStack.push(node);
			}
			else if (node.operatorType == OpTypeRightParen)
			{
				// Try to find the matching left parenthesis, popping everything in-between to the output queue.
				// Once the matching left parenthesis is found, the left/right parenthesis pair is discarded.
				bool leftParenFound = false;
				while (!operatorStack.empty())
				{
					Node topStackNode = operatorStack.top();
					operatorStack.pop();
					if (topStackNode.operatorType == OpTypeLeftParen)
					{
						leftParenFound = true;
						break;
					}
					else
					{
						outputQueue.push(topStackNode);
					}
				}
				if (operatorStack.empty() && !leftParenFound)
				{
					while (!outputQueue.empty())
					{
						outputQueue.pop();
					}
					return outputQueue;
				}
			}
			else
			{
				while (!operatorStack.empty())
				{
					Node topStackNode = operatorStack.top();
					if (topStackNode.operatorType != OpTypeLeftParen &&
					        OperatorPrecedence[node.operatorType] <= OperatorPrecedence[topStackNode.operatorType])
					{
						operatorStack.pop();
						outputQueue.push(topStackNode);
					}
					else
					{
						break;
					}
				}
				operatorStack.push(node);
			}
		}

		inputIdentifier += consumedChars;
		consumedChars = 0;

		lastNode = node;
	}

	// Pop the remaining operators from the operator stack and push them to the output queue
	while (!operatorStack.empty())
	{
		Node topStackNode = operatorStack.top();
		operatorStack.pop();
		outputQueue.push(topStackNode);
	}

	return outputQueue;
}

inline bool VisibleIf::ProcessEvalQueue(NodeQueue& queue)
{
	/* If an empty queue is passed, this means an error occurred earlier, in which case
	the routine must return true in order not to hide parameters, without emitting
	extra error messages.
	*/
	if (queue.empty())
		return true;

	NodeStack evalStack;

	while (!queue.empty())
	{
		Node node = queue.front();
		queue.pop();
		if (node.type != NodeOperator)
		{
			if (node.type == NodeSubstanceInput)
				ResolveSubstanceInputNode(&node);
			evalStack.push(node);
			continue;
		}

		// Check number of needed operands and pop the right operand if two are needed
		Node op1, op2;
		switch (node.operatorType)
		{
		case OpTypeOr:
		case OpTypeAnd:
		case OpTypeCompEQ:
		case OpTypeCompNEQ:
		case OpTypeCompGT:
		case OpTypeCompLT:
		case OpTypeCompGE:
		case OpTypeCompLE:
		case OpTypePlus:
		case OpTypeMinus:
		case OpTypeMul:
		case OpTypeDiv:
			if (evalStack.size() < 2)
			{
				return true; // This will be used as the "visible" flag, so it's best to leave things visible if an error occurs
			}
			op2 = evalStack.top();
			evalStack.pop();
			break;
		case OpTypeUnaryMinus:
		case OpTypeUnaryPlus:
		case OpTypeNot:
			if (evalStack.empty())
			{
				return true; // This will be used as the "visible" flag, so it's best to leave things visible if an error occurs
			}
			break;
		default:
			break;
		}

		// Pop the left argument, which is always needed
		op1 = evalStack.top();
		evalStack.pop();

		// Perform the required operation and push the result to the stack
		Node resultNode;
		switch (node.operatorType)
		{
#define PERFORM_OPERATION(operatorType, resultNodeType, resultValueType, conversion, operation) \
case (operatorType): \
	resultNode.type = resultNodeType; \
	resultNode.content.resultValueType##Value = conversion(op1) operation conversion(op2); \
	break;
			PERFORM_OPERATION(OpTypeOr, NodeBoolValue, bool, asBool, ||)
			PERFORM_OPERATION(OpTypeAnd, NodeBoolValue, bool, asBool, &&)
			PERFORM_OPERATION(OpTypeCompEQ, NodeBoolValue, bool, asFloat, ==)
			PERFORM_OPERATION(OpTypeCompNEQ, NodeBoolValue, bool, asFloat, !=)
			PERFORM_OPERATION(OpTypeCompGT, NodeBoolValue, bool, asFloat, >)
			PERFORM_OPERATION(OpTypeCompLT, NodeBoolValue, bool, asFloat, <)
			PERFORM_OPERATION(OpTypeCompGE, NodeBoolValue, bool, asFloat, >=)
			PERFORM_OPERATION(OpTypeCompLE, NodeBoolValue, bool, asFloat, <=)
			PERFORM_OPERATION(OpTypePlus, NodeFloatValue, float, asFloat, +)
			PERFORM_OPERATION(OpTypeMinus, NodeFloatValue, float, asFloat, -)
			PERFORM_OPERATION(OpTypeMul, NodeFloatValue, float, asFloat, *)
			PERFORM_OPERATION(OpTypeDiv, NodeFloatValue, float, asFloat, /)

		case OpTypeUnaryMinus:
			resultNode.type = NodeFloatValue;
			resultNode.content.floatValue = -asFloat(op1);
			break;
		case OpTypeUnaryPlus:
			resultNode.type = NodeFloatValue;
			resultNode.content.floatValue = asFloat(op1);
			break;
		case OpTypeNot:
			resultNode.type = NodeBoolValue;
			resultNode.content.boolValue = !asBool(op1);
			break;
		default:
			break;
		}

		evalStack.push(resultNode);
	}

	// Once eval is done, there should be exactly one value left on the stack
	if (evalStack.empty())
	{
		return true;
	}
	if (evalStack.size() > 1)
	{
		return true;
	}

	return asBool(evalStack.top());
}

//! @brief entry point into evaluating a visible if
inline bool VisibleIf::EvalVisibleIf(const SubstanceAir::InputInstanceBase* substanceInput)
{
	if (substanceInput->mDesc.mGuiVisibleIf.empty())
		return true;

	DefaultFunctor functor(substanceInput);
	NodeQueue rpnQueue = VisibleIf::BuildEvalQueue(substanceInput->mDesc.mGuiVisibleIf.c_str(), functor);
	return VisibleIf::ProcessEvalQueue(rpnQueue);
}

inline bool VisibleIf::EvalVisibleIf(
	const char* inputIdentifier,
	const VisibleIfFunctor& functor)
{
	if (!inputIdentifier)
		return true;

	NodeQueue rpnQueue = VisibleIf::BuildEvalQueue(inputIdentifier, functor);
	return VisibleIf::ProcessEvalQueue(rpnQueue);
}

}  // namespace SubstanceAir

#endif //_SUBSTANCE_AIR_FRAMEWORK_VISIBLEIF_H
