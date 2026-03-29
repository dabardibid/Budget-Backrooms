//! @file callbackstype.h
//! @brief Callback function pointer definition
//! @author Galen Helfter - Allegorithmic
//! @date 20190411
//! @copyright Allegorithmic. All rights reserved.

#pragma once

#include <substance/link/types.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
typedef void (*MessageCallback)(unsigned int, const substance_link_uuid_t *uuid, const char*);
} // namespace Framework
} // namespace Link
} // namespace Alg
