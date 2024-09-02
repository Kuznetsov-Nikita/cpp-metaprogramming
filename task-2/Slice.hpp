#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>
#include <cassert>
#include <type_traits>


inline constexpr std::ptrdiff_t dynamic_stride = -1;


namespace details {
    template<std::size_t Extent>
    struct ExtentStorage {
        constexpr ExtentStorage() {}

        constexpr ExtentStorage(std::size_t) {}

        static constexpr std::size_t extent = Extent;
    };

    template<>
    struct ExtentStorage<std::dynamic_extent> {
        constexpr ExtentStorage(): extent(0) {}

        constexpr ExtentStorage(std::size_t extent): extent(extent) {}

        std::size_t extent;
    };

    template<std::ptrdiff_t Stride>
    struct StrideStorage {
        constexpr StrideStorage() {}

        constexpr StrideStorage(std::ptrdiff_t) {}

        static constexpr std::ptrdiff_t stride = Stride;
    };

    template<>
    struct StrideStorage<dynamic_stride> {
        constexpr StrideStorage(): stride(0) {}

        constexpr StrideStorage(std::ptrdiff_t stride): stride(stride) {}

        std::ptrdiff_t stride;
    };

    template<typename T, std::size_t Extent, std::ptrdiff_t Stride>
    struct SliceStorage: ExtentStorage<Extent>, StrideStorage<Stride> {
        constexpr SliceStorage() {}

        constexpr SliceStorage(T* data, std::size_t extent, std::ptrdiff_t stride)
            : ExtentStorage<Extent>(extent), 
              StrideStorage<Stride>(stride), 
              data(data) {}

        T* data;
    };

    template <class T>
    class StridedIterator {
    public:
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using difference_type = std::iter_difference_t<pointer>;

        template<class U>
        friend StridedIterator<U> operator+(
            const std::iter_difference_t<U*>&, 
            const StridedIterator<U>&
        );

        StridedIterator() = default;
 
        StridedIterator(pointer data, difference_type stride)
            : data_(data), stride_(stride) {}

        StridedIterator(const StridedIterator&) = default;

        StridedIterator(StridedIterator&&) noexcept = default;

        StridedIterator& operator=(const StridedIterator&) = default;

        ~StridedIterator() = default;
         
        auto operator<=>(const StridedIterator& other) const {
            return data_ <=> other.data_;
        }

        bool operator==(const StridedIterator& other) const {
            return data_ == other.data_;
        }

        difference_type operator-(const StridedIterator& other) const { 
            return (data_ - other.data_) / stride_; 
        }
         
        StridedIterator& operator+=(const difference_type& diff) { 
            data_ += diff * stride_; 
            return *this; 
        }

        StridedIterator& operator-=(const difference_type& diff) {
            data_ -= diff * stride_; 
            return *this; 
        }

        StridedIterator& operator++() {
            data_ += stride_; 
            return *this; 
        }

        StridedIterator operator++(int) { 
            StridedIterator copy = *this; 
            data_ += stride_; 
            return copy; 
        }

        StridedIterator& operator--() { 
            data_ -= stride_; 
            return *this; 
        }

        StridedIterator operator--(int) { 
            StridedIterator copy = *this; 
            data_ -= stride_; 
            return copy; 
        }

        StridedIterator operator+(const difference_type& diff) const { 
            StridedIterator copy = *this; 
            copy.data_ += diff * stride_; 
            return copy; 
        }

        StridedIterator operator-(const difference_type& diff) const { 
            StridedIterator copy = *this; 
            copy.data_ -= diff * stride_; 
            return copy; 
        }
         
        reference operator*() { 
            return *data_; 
        }

        const reference operator*() const { 
            return *data_;
        }

        pointer operator->() const { 
            return data_; 
        }

        reference operator[](const difference_type& diff) {
            return data_[diff * stride_];
        }

        const reference operator[](const difference_type& diff) const {
            return data_[diff * stride_];
        }
 
    private:
        pointer data_;
        difference_type stride_;
    };

    template<class T>
    StridedIterator<T> operator+(
        const std::iter_difference_t<T*>& diff, 
        const StridedIterator<T>& iter
    ) {
        return iter + diff;
    }
}


template<class T, std::size_t extent = std::dynamic_extent, std::ptrdiff_t stride = 1>
class Slice {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = size_t;

    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;

    using difference_type = std::ptrdiff_t;

    using iterator = details::StridedIterator<T>;
    using const_iterator = details::StridedIterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template<class OtherT, std::size_t OtherExtent, std::ptrdiff_t OtherStride>
    friend class Slice;

    template
        < class FirstT,
          class SecondT, 
          std::size_t Extent, 
          std::ptrdiff_t Stride, 
          std::size_t OtherExtent, 
          std::ptrdiff_t OtherStride
        >
    friend bool operator==(const Slice<FirstT, Extent, Stride>&, 
                           const Slice<SecondT, OtherExtent, OtherStride>&);

    constexpr Slice() = default;

    constexpr Slice(T* data, std::size_t extent_value, std::ptrdiff_t stride_value)
        : storage_(data, extent_value, stride_value) {}

    template <class Iterator>
    constexpr Slice(Iterator first, std::size_t count, std::ptrdiff_t skip)
        : storage_(&*first, count, skip) {
        assert(extent == std::dynamic_extent || extent == count);
    }

    template<class Container>
    constexpr Slice(Container& container) 
        requires requires { container.data(); container.size(); }
        : Slice(std::data(container), std::size(container), stride) {
        assert(extent == std::dynamic_extent || extent == std::size(container));
    }

    constexpr Slice(const Slice& other) = default;

    ~Slice() = default;

    constexpr operator Slice<T, std::dynamic_extent, dynamic_stride>() const 
        requires (extent != std::dynamic_extent && stride != dynamic_stride) {
        return Slice<T, std::dynamic_extent, dynamic_stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr operator Slice<T, extent, dynamic_stride>() const 
        requires (stride != dynamic_stride) {
        return Slice<T, extent, dynamic_stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr operator Slice<const T, std::dynamic_extent, stride>() const 
        requires (extent != std::dynamic_extent) {
        return Slice<T, std::dynamic_extent, stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr operator Slice<const T, std::dynamic_extent, dynamic_stride>() const 
        requires (extent != std::dynamic_extent && 
                  stride != dynamic_stride && !std::is_const_v<T>) {
        return Slice<T, std::dynamic_extent, dynamic_stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr operator Slice<const T, extent, dynamic_stride>() const 
        requires (stride != dynamic_stride && !std::is_const_v<T>) {
        return Slice<T, extent, dynamic_stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr operator Slice<T, std::dynamic_extent, stride>() const 
        requires (extent != std::dynamic_extent && !std::is_const_v<T>) {
        return Slice<T, std::dynamic_extent, stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr operator Slice<const T, extent, stride>() const {
        return Slice<const T, extent, stride>(
            storage_.data, 
            storage_.extent, 
            storage_.stride
        );
    }

    constexpr reference operator[](size_type index) const {
        assert(index < storage_.extent);
        return *(storage_.data + index * storage_.stride);
    }

    constexpr pointer Data() const {
        return storage_.data;
    }

    constexpr size_type Size() const {
        return storage_.extent;
    }

    constexpr bool Empty() const {
        return storage_.extent == 0;
    }

    constexpr difference_type Stride() const {
        return storage_.stride;
    }

    constexpr reference Front() const {
        assert(storage_.extent > 0);
        return *storage_.data;
    }

    constexpr reference Back() const {
        assert(storage_.extent > 0);
        return *(storage_.data + (storage_.extent - 1) * storage_.stride);
    }

    Slice<T, std::dynamic_extent, stride> First(std::size_t count) const {
        assert(count < storage_.extent);
        return Slice<T, std::dynamic_extent, stride>(
            storage_.data, 
            count, 
            storage_.stride
        );
    }

    template <std::size_t count>
    Slice<T, count, stride> First() const {
        assert(count < storage_.extent);
        return Slice<T, count, stride>(storage_.data, count, storage_.stride);
    }

    Slice<T, std::dynamic_extent, stride> Last(std::size_t count) const {
        assert(count < storage_.extent);
        return Slice<T, std::dynamic_extent, stride>(
            storage_.data + (storage_.extent - count) * storage_.stride, 
            count, 
            storage_.stride
        );
    }

    template <std::size_t count>
    Slice<T, count, stride> Last() const {
        assert(count < storage_.extent);
        return Slice<T, count, stride>(
            storage_.data + (storage_.extent - count) * storage_.stride, 
            count, 
            storage_.stride
        );
    }

    Slice<T, std::dynamic_extent, stride> DropFirst(std::size_t count) const {
        assert(count < storage_.extent);
        return Slice<T, std::dynamic_extent, stride>(
            storage_.data + count * storage_.stride,
            storage_.extent - count, 
            storage_.stride
        );
    }

    template <std::size_t count>
    Slice
        < T, 
          (extent == std::dynamic_extent) ? extent : extent - count, 
          stride
        > DropFirst() const {
        assert(count < storage_.extent);
        return Slice<T, (extent == std::dynamic_extent) ? extent : extent - count, stride>(
            storage_.data + count * storage_.stride, 
            storage_.extent - count, 
            storage_.stride
        );
    }

    Slice<T, std::dynamic_extent, stride> DropLast(std::size_t count) const {
        assert(count < storage_.extent);
        return Slice<T, std::dynamic_extent, stride>(
            storage_.data, 
            storage_.extent - count, 
            storage_.stride
        );
    }

    template <std::size_t count>
    Slice
        < T, 
          (extent == std::dynamic_extent) ? extent : extent - count, 
          stride
        > DropLast() const {
        assert(count < storage_.extent);
        return Slice<T, (extent == std::dynamic_extent) ? extent : extent - count, stride>(
            storage_.data, 
            storage_.extent - count, 
            storage_.stride
        );
    }

    Slice<T, std::dynamic_extent, dynamic_stride> Skip(std::ptrdiff_t skip) const {
        return Slice<T, std::dynamic_extent, dynamic_stride>(
            storage_.data, 
            storage_.extent / skip + (storage_.extent % skip != 0), 
            skip * storage_.stride
        );
    }

    template <std::ptrdiff_t skip>
    Slice
        < T, 
          (extent == std::dynamic_extent) ? extent : extent / skip + (extent % skip != 0), 
          (stride == dynamic_stride) ? dynamic_stride : skip * stride
        > Skip() const {
        return 
            Slice
                < T, 
                  (extent == std::dynamic_extent) ? extent : extent / skip + (extent % skip != 0), 
                  (stride == dynamic_stride) ? dynamic_stride : skip * stride
                >(
                storage_.data, 
                storage_.extent / skip + (storage_.extent % skip != 0), 
                skip * storage_.stride
            );
    }

    constexpr iterator begin() const {
        return iterator(storage_.data, storage_.stride);
    }

    constexpr iterator end() const {
        return iterator(storage_.data + storage_.extent * storage_.stride, storage_.stride);
    }

    constexpr const_iterator cbegin() const {
        return begin();
    }

    constexpr const_iterator cend() const {
        return end();
    }

    constexpr reverse_iterator rbegin() const {
        return reverse_iterator{end()};
    }

    constexpr reverse_iterator rend() const {
        return reverse_iterator{begin()};
    }

    constexpr const_reverse_iterator crbegin() const {
        return rbegin();
    }

    constexpr const_reverse_iterator crend() const {
        return rend();
    }

private:
    using storage_type = details::SliceStorage<T, extent, stride>;

    storage_type storage_;
};

template
    < class T,
      class OtherT, 
      std::size_t Extent, 
      std::ptrdiff_t Stride, 
      std::size_t OtherExtent, 
      std::ptrdiff_t OtherStride
    >
bool operator==(const Slice<T, Extent, Stride>& first, 
                const Slice<OtherT, OtherExtent, OtherStride>& second) {
    if (first.storage_.extent != second.storage_.extent || first.storage_.stride != second.storage_.stride) {
        return false;
    }

    bool result = true;
    for (size_t i = 0; i < first.storage_.extent; ++i) {
        if (first[i] != second[i]) {
            result = false;
            break;
        }
    }

    return result;
}

template<class Iterator, class N, class M>
Slice(Iterator, N, M) -> Slice<std::remove_reference_t<std::iter_reference_t<Iterator>>, std::dynamic_extent, dynamic_stride>;

template<class T, std::size_t N>
Slice(T (&)[N]) -> Slice<T, N>;

template<class T, std::size_t N>
Slice(std::array<T, N>&) -> Slice<T, N>;
    
template<class T, std::size_t N>
Slice(const std::array<T, N>&) -> Slice<const T, N>;

template<class Container>
Slice(Container&&) -> Slice<std::remove_reference_t<std::ranges::range_reference_t<Container>>>;
