#pragma once

#include <iostream>

class ArenaAllocator
{
private:
    size_t _size;
    std::byte* _buffer;
    std::byte* _offset;

public:
    ArenaAllocator(size_t bites) : _size(bites)
    {
        _buffer = static_cast<std::byte*>(malloc(_size));
        _offset = _buffer;
    }

    template <typename T>
    T* alloc()
    {
        void* offset = _offset;
        _offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    ArenaAllocator(const ArenaAllocator& o) = delete;

    ~ArenaAllocator()
    {
        free(_buffer);
    }

    ArenaAllocator operator=(const ArenaAllocator& o) = delete;
};
