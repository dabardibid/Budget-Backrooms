//! @file dynamicload.h
//! @brief Handle dynamic loading and calling of functions
//! @author Galen Helfter - Adobe
//! @date 20191029
//! @copyright Adobe. All rights reserved.

#pragma once

#include <substance/link/link.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
namespace Details
{
#if defined(SUBSTANCE_LINK_FRAMEWORK_DYNAMIC_LOAD)
extern const substance_link_function_table* linkFunctionTable;
#define LINK_FRAMEWORK_CALL(fn) ::Alg::Link::Framework::Details::linkFunctionTable->fn
#else
#define LINK_FRAMEWORK_CALL(fn) substance_link_ ## fn
#endif

//! @brief Acquire the dynamic loaded function table from the module
//! @param module Pointer to the dynamically loaded module
//! @return True on success, false on failure or if built without dynamic loading
bool initDynamicLoad(void* module);

//! @brief Clear the dynamically loaded function table
//! @return True on success, false on failure or if built without dynamic loading
bool shutdownDynamicLoad();
} // namespace Details
} // namespace Framework
} // namespace Link
} // namespace Alg
