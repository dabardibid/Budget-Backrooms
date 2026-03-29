//! @file callbacks.h
//! @brief Register callback types for messages
//! @author Galen Helfter - Allegorithmic
//! @date 20190411
//! @copyright Allegorithmic. All rights reserved.

#pragma once

#include <string>

#include <substance/link/framework/callbackstype.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
namespace Details
{
//! @brief Bind the callback to the message type
//! @param messageType UUID for the type of message
//! @param callback Callback function called by link
//! @return True on success, false if the message type already exists
bool bindCallback(const substance_link_uuid_t& messageType, MessageCallback callback);

//! @brief Free the binding for the message type
//! @param messageType UUID for the type of message
//! @return True on success, false if no message of the type is bound
bool unbindCallback(const substance_link_uuid_t& messageType);

//! @brief Search for the callback bound to the message type
//! @param messageType UUID for the type of message
//! @return Valid function on success, nullptr on failure
MessageCallback findCallback(const substance_link_uuid_t& messageType);
} // namespace Details
} // namespace Framework
} // namespace Link
} // namespace Alg
