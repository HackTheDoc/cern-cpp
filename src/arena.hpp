#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>
#include <iostream>

class ArenaAllocator final
{
private:
    size_t _size;
    std::byte* _buffer;
    std::byte* _offset;

public:
    ArenaAllocator(const std::size_t max_num_bytes)
        : _size { max_num_bytes }
        , _buffer { new std::byte[max_num_bytes] }
        , _offset { _buffer }
    {
    }

    ArenaAllocator(const ArenaAllocator&) = delete;

    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    ArenaAllocator(ArenaAllocator&& other) noexcept
        : _size { std::exchange(other._size, 0) }
        , _buffer { std::exchange(other._buffer, nullptr) }
        , _offset { std::exchange(other._offset, nullptr) }
    {
    }

    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept
    {
        std::swap(_size, other._size);
        std::swap(_buffer, other._buffer);
        std::swap(_offset, other._offset);
        return *this;
    }

    template <typename T>
    [[nodiscard]] T* alloc()
    {
        std::size_t remaining_num_bytes = _size - static_cast<std::size_t>(_offset - _buffer);
        auto pointer = static_cast<void*>(_offset);
        const auto aligned_address = std::align(alignof(T), sizeof(T), pointer, remaining_num_bytes);
        if (aligned_address == nullptr) {
            throw std::bad_alloc {};
        }
        _offset = static_cast<std::byte*>(aligned_address) + sizeof(T);
        return static_cast<T*>(aligned_address);
    }

    template <typename T, typename... Args>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        const auto allocated_memory = alloc<T>();
        return new (allocated_memory) T { std::forward<Args>(args)... };
    }

    ~ArenaAllocator()
    {
        delete[] _buffer;
    }
};
