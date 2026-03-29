//! @file uuid.h
//! @brief Higher level utilities for using uuids with Link
//! @author Galen Helfter - Allegorithmic
//! @date 20190525
//! @copyright Allegorithmic. All rights reserved.

#pragma once

#include <substance/link/types.h>

// Operators must be in the global namespace, as the C struct is also
// in the global namespace.

//! @brief Less-than comparison operator for a Link uuid type
//! @param x Left-hand element to compare
//! @param y Right-handle element being compared to
//! @return True if x < y, false otherwise
bool operator <(const substance_link_uuid_t& x, const substance_link_uuid_t& y);

//! @brief Equality comparison operator for a Link uuid type
//! @param x Left-hand element to compare
//! @param y Right-handle element being compared to
//! @return True if x == y, false otherwise
bool operator ==(const substance_link_uuid_t& x, const substance_link_uuid_t& y);
