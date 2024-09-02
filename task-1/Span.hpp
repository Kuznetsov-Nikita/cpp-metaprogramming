#include <span>
#include <concepts>
#include <cstdlib>
#include <iterator>
#include <cassert>


namespace details {
    template <typename T, std::size_t S>
    struct SpanStorage {
        constexpr SpanStorage(): data(nullptr) {}

        constexpr SpanStorage(T* data, std::size_t): data(data) {}

        T* data;
        static constexpr std::size_t extent = S;
    };

    template <typename T>
    struct SpanStorage<T, std::dynamic_extent> {
        constexpr SpanStorage(): data(nullptr), extent(0) {}

        constexpr SpanStorage(T* data, std::size_t extent): data(data), extent(extent) {}

        T* data;
        std::size_t extent;
    };
}


template<class T, std::size_t Extent = std::dynamic_extent>
class Span {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = size_t;

    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;

    using difference_type = std::ptrdiff_t;

    using iterator = element_type*;
    using const_iterator = const element_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr Span() = default;

    template<class Iterator>
    constexpr Span(Iterator first, size_type count): storage_(storage_type(&*first, count)) {
        assert(Extent == std::dynamic_extent || Extent == count);
    }

    template<class Iterator>
    constexpr Span(Iterator first, Iterator last): Span(first, last - first) {
        assert(Extent == std::dynamic_extent || Extent == last - first);
    }

    template<std::size_t N>
    constexpr Span(std::type_identity_t<element_type> (&arr)[N]): Span(arr, N) {
        static_assert(Extent == std::dynamic_extent || Extent == N);
    }

    template<class U, std::size_t N>
    constexpr Span(std::array<U, N>& arr): Span(arr.data(), N) {
        static_assert(Extent == std::dynamic_extent || Extent == N);
    }

    template<class U, std::size_t N>
    constexpr Span(const std::array<U, N>& arr): Span(arr.data(), N) {
        static_assert(Extent == std::dynamic_extent || Extent == N);
    }

    template<class Container>
    constexpr Span(Container&& range): Span(std::data(range), std::size(range)) {
        assert(Extent == std::dynamic_extent || Extent == std::size(range));
    }

    template<class OtherT, std::size_t OtherExtent>
    constexpr Span(const Span<OtherT, OtherExtent>& other): Span(other.storage_.data, other.storage_.extent) {}

    constexpr Span& operator=(const Span& other) = default;

    ~Span() = default;

    constexpr reference operator[](size_type index) const {
        assert(index < storage_.extent);
        return *(storage_.data + index);
    }

    constexpr reference Front() const {
        assert(storage_.extent > 0);
        return *storage_.data;
    }

    constexpr reference Back() const {
        assert(storage_.extent > 0);
        return *(storage_.data + storage_.extent - 1);
    }

    template<size_t Count>
    constexpr Span<element_type, Count> First() const {
        assert(Count <= storage_.extent);
        return Span<element_type, Count>(storage_.data, Count);
    }

    constexpr Span<element_type, std::dynamic_extent> First(size_type count) const {
        assert(count <= storage_.extent);
        return Span<element_type, std::dynamic_extent>(storage_.data, count);
    }

    template<size_t Count>
    constexpr Span<element_type, Count> Last() const {
        assert(Count <= storage_.extent);
        return Span<element_type, Count>(storage_.data + storage_.extent - Count, Count);
    }

    constexpr Span<element_type, std::dynamic_extent> Last(size_type count) const {
        assert(count <= storage_.extent);
        return Span<element_type, std::dynamic_extent>(storage_.data + storage_.extent - count, count);
    }

    template<size_t Offset, size_t Count = std::dynamic_extent>
    constexpr Span<element_type, Count> Subspan() const {
        assert(Offset <= storage_.extent);
        assert(Count == std::dynamic_extent || Offset + Count <= storage_.extent);
        return Span<element_type, Count>(storage_.data + Offset, Count);
    }

    constexpr size_type Size() const {
        return storage_.extent;
    }

    constexpr size_type SizeBytes() const {
        return storage_.extent * sizeof(element_type);
    }

    constexpr bool Empty() const {
        return storage_.extent == 0;
    }

    constexpr pointer Data() const {
        return storage_.data;
    }

    constexpr iterator begin() const {
        return storage_.data;
    }

    constexpr iterator end() const {
        return storage_.data + storage_.extent;
    }

    constexpr iterator cbegin() const {
        return begin();
    }

    constexpr iterator cend() const {
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
    using storage_type = details::SpanStorage<T, Extent>;

    storage_type storage_;
};

template<class Iterator, class EndOrSize>
Span(Iterator, EndOrSize) -> Span<std::remove_reference_t<std::iter_reference_t<Iterator>>>;

template<class T, size_t N>
Span(T (&)[N]) -> Span<T, N>;

template<class T, size_t N>
Span(std::array<T, N>&) -> Span<T, N>;
  
template<class T, size_t N>
Span(const std::array<T, N>&) -> Span<const T, N>;

template<class Container>
Span(Container&&) -> Span<std::remove_reference_t<std::ranges::range_reference_t<Container>>>;
