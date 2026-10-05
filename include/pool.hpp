#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <vector>

struct Pool {
    Pool(std::size_t obj_size, std::size_t capacity)
        : stride_(aligned_stride(obj_size)), capacity_(capacity),
          storage_(capacity == 0 ? nullptr : new std::byte[checked_size(stride_, capacity)]),
          allocated_(capacity, 0), free_head_(nullptr) {
        for (std::size_t i = capacity_; i > 0; --i) {
            void* slot = storage_.get() + (i - 1) * stride_;
            *reinterpret_cast<void**>(slot) = free_head_;
            free_head_ = slot;
        }
    }

    void* alloc() {
        if (free_head_ == nullptr) return nullptr;
        void* slot = free_head_;
        free_head_ = *reinterpret_cast<void**>(slot);
        const std::size_t index = (static_cast<std::byte*>(slot) - storage_.get()) / stride_;
        allocated_[index] = 1;
        return slot;
    }

    void free(void* p) {
        if (p == nullptr || storage_ == nullptr) return;
        auto* bytes = static_cast<std::byte*>(p);
        if (bytes < storage_.get() || bytes >= storage_.get() + stride_ * capacity_) return;
        const std::size_t offset = static_cast<std::size_t>(bytes - storage_.get());
        if (offset % stride_ != 0) return;
        const std::size_t index = offset / stride_;
        if (!allocated_[index]) return;
        allocated_[index] = 0;
        *reinterpret_cast<void**>(p) = free_head_;
        free_head_ = p;
    }

private:
    static std::size_t aligned_stride(std::size_t obj_size) {
        const std::size_t alignment = alignof(std::max_align_t);
        const std::size_t size = obj_size < sizeof(void*) ? sizeof(void*) : obj_size;
        if (size > std::numeric_limits<std::size_t>::max() - (alignment - 1)) {
            throw std::bad_array_new_length();
        }
        return (size + alignment - 1) / alignment * alignment;
    }

    static std::size_t checked_size(std::size_t stride, std::size_t capacity) {
        if (capacity != 0 && stride > std::numeric_limits<std::size_t>::max() / capacity) {
            throw std::bad_array_new_length();
        }
        return stride * capacity;
    }

    std::size_t stride_;
    std::size_t capacity_;
    std::unique_ptr<std::byte[]> storage_;
    std::vector<unsigned char> allocated_;
    void* free_head_;
};
