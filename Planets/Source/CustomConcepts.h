#pragma once

template<std::integral T>
constexpr bool IsPowerOfTwo(const T value)
{
    assert(value > 0);

    if (value == 1)
    {
        // 2 ^ 0 = 1
        return true;
    }

    float half = (float)value / 2.0f;

    // If "half" at any point is equal to 1, we know that the number is a power of 2
    while (half != 1.0f)
    {
        int halfInt = (int)half;
        // Test whether or not half is a whole number, i.e., if half * 2 was
        // evenly divisible by 2
        if (float(halfInt) != half)
        {
            // If half * 2 was not evenly divisible by 2, the number
            // is not a power of 2
            return false;
        }

        half /= 2.0f;
    }

    // At some point "half" was equal to 1, so we know that the number is a power of 2
    return true;
}

template<class T>
concept Clock = requires
{
    typename T::rep;
    typename T::period;
    typename T::duration;
    typename T::time_point;
    T::is_steady;
    T::now();
};

// Iterator concepts
template<class T>
concept Referenceable = std::is_reference<T&>::value;
template<class I>
concept LegacyIterator =
requires(I i) {
    {   *i } -> std::_Can_reference;
    {  ++i } -> std::same_as<I&>;
    { *i++ } -> std::_Can_reference;
}&& std::copyable<I>;

template<class I>
concept LegacyInputIterator =
LegacyIterator<I> && std::equality_comparable<I> && requires(I i) {
    typename std::incrementable_traits<I>::difference_type;
    typename std::iterator_traits<I>::value_type;
    typename std::common_reference_t<std::iter_reference_t<I>&&,
        typename std::iterator_traits<I>::value_type&>;
    *i++;
    typename std::common_reference_t<decltype(*i++)&&,
        typename std::iterator_traits<I>::value_type&>;
    requires std::signed_integral<typename std::incrementable_traits<I>::difference_type>;
};

template<class I>
concept LegacyForwardIterator =
LegacyInputIterator<I> && std::constructible_from<I> &&
std::is_lvalue_reference_v<std::iter_reference_t<I>> &&
std::same_as<
    std::remove_cvref_t<std::iter_reference_t<I>>,
    typename std::iterator_traits<I>::value_type> &&
    requires(I i) {
        {  i++ } -> std::convertible_to<const I&>;
        { *i++ } -> std::same_as<std::iter_reference_t<I>>;
};

template<class I>
concept LegacyBidirectionalIterator =
LegacyForwardIterator<I> && requires(I i) {
    {  --i } -> std::same_as<I&>;
    {  i-- } -> std::convertible_to<const I&>;
    { *i-- } -> std::same_as<std::iter_reference_t<I>>;
};

template<class I>
concept LegacyRandomAccessIterator =
LegacyBidirectionalIterator<I> && std::totally_ordered<I> &&
requires(I i, typename std::incrementable_traits<I>::difference_type n) {
    { i += n } -> std::same_as<I&>;
    { i -= n } -> std::same_as<I&>;
    { i + n } -> std::same_as<I>;
    { n + i } -> std::same_as<I>;
    { i - n } -> std::same_as<I>;
    { i - i } -> std::same_as<decltype(n)>;
    {  i[n]  } -> std::convertible_to<std::iter_reference_t<I>>;
};