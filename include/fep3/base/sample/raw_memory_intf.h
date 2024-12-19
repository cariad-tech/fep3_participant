/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <cstddef>

namespace fep3 {
namespace arya {
/**
 * @brief Raw Memory interface to access any kind of data by a raw memory pointer and its size
 */
class IRawMemory {
protected:
    /// DTOR
    ~IRawMemory() = default;

public:
    /**
     * @brief Gets the capacity of the memory
     *
     * @return size_t The size in bytes
     */
    virtual size_t capacity() const = 0;

    /**
     * @brief Gets the raw pointer to the memory
     *
     * @return const void* The pointer
     */
    virtual const void* cdata() const = 0;

    /**
     * @brief Gets the size of the current stored value in the memory
     *
     * @return size_t The size in bytes
     */
    virtual size_t size() const = 0;

    /**
     * @brief Resets the memory of the raw pointer
     *
     * @param[in] data The data to set
     * @param[in] data_size The size in bytes to set
     * @return size_t The size in bytes that were copied (if not equal to @p data_size, something
     * went wrong)
     */
    virtual size_t set(const void* data, size_t data_size) = 0;

    /**
     * @brief Resizes the memory
     *
     * @param[in] data_size The size in bytes to resize
     * @return size_t The new size in bytes (if not equal to @p data_size, something went wrong)
     */
    virtual size_t resize(size_t data_size) = 0;
};
} // namespace arya
using arya::IRawMemory;
} // namespace fep3
