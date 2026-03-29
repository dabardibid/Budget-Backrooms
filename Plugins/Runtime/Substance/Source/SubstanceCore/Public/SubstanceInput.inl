// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceInput.inl

#pragma once
#include "substance/framework/input.h"

namespace Substance
{
	template <typename T> void GetNumericalInputValue(SubstanceAir::InputInstanceNumericalBase* BaseInput, TArray<T>& OutValue)
	{
		switch ((SubstanceIOType)BaseInput->mDesc.mType)
		{
		case Substance_IOType_Float:
		{
			SubstanceAir::InputInstanceFloat* Input = (SubstanceAir::InputInstanceFloat*)BaseInput;
			OutValue.Push(Input->getValue());
		}
		break;
		case Substance_IOType_Float2:
		{
			SubstanceAir::InputInstanceFloat2* Input = (SubstanceAir::InputInstanceFloat2*)BaseInput;
			SubstanceAir::Vec2Float InputValue = Input->getValue();
			OutValue.Push(InputValue.x);
			OutValue.Push(InputValue.y);
		}
		break;
		case Substance_IOType_Float3:
		{
			SubstanceAir::InputInstanceFloat3* Input = (SubstanceAir::InputInstanceFloat3*)BaseInput;
			SubstanceAir::Vec3Float InputValue = Input->getValue();
			OutValue.Push(InputValue.x);
			OutValue.Push(InputValue.y);
			OutValue.Push(InputValue.z);
		}
		break;
		case Substance_IOType_Float4:
		{
			SubstanceAir::InputInstanceFloat4* Input = (SubstanceAir::InputInstanceFloat4*)BaseInput;
			SubstanceAir::Vec4Float InputValue = Input->getValue();
			OutValue.Push(InputValue.x);
			OutValue.Push(InputValue.y);
			OutValue.Push(InputValue.z);
			OutValue.Push(InputValue.w);
		}
		break;
		case Substance_IOType_Integer:
		{
			SubstanceAir::InputInstanceInt* Input = (SubstanceAir::InputInstanceInt*)BaseInput;
			OutValue.Push(Input->getValue());
		}
		break;
		case Substance_IOType_Integer2:
		{
			SubstanceAir::InputInstanceInt2* Input = (SubstanceAir::InputInstanceInt2*)BaseInput;
			SubstanceAir::Vec2Int InputValue = Input->getValue();
			OutValue.Push(InputValue.x);
			OutValue.Push(InputValue.y);
		}
		break;
		case Substance_IOType_Integer3:
		{
			SubstanceAir::InputInstanceInt3* Input = (SubstanceAir::InputInstanceInt3*)BaseInput;
			SubstanceAir::Vec3Int InputValue = Input->getValue();
			OutValue.Push(InputValue.x);
			OutValue.Push(InputValue.y);
			OutValue.Push(InputValue.z);
		}
		break;
		case Substance_IOType_Integer4:
		{
			SubstanceAir::InputInstanceInt4* Input = (SubstanceAir::InputInstanceInt4*)BaseInput;
			SubstanceAir::Vec4Int InputValue = Input->getValue();
			OutValue.Push(InputValue.x);
			OutValue.Push(InputValue.y);
			OutValue.Push(InputValue.z);
			OutValue.Push(InputValue.w);
		}
		break;
		default:
			break;
		}
	}
} // namespace Substance