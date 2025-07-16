#include <functional>
#include <cstdint>

#include <array>
#include <iterator>
#include <string>
#include <utility>

#include <stdexcept>
#define FROZEN_THROW_OR_ABORT(err) throw err

namespace frozen { namespace bits {

// used as a fake argument for frozen::make_set and frozen::make_map in the case
// of N=0
struct ignored_arg {};

template <class T, std::size_t N>
class cvector {
    T data[N] = {}; // zero-initialization for scalar type T,
                    // default-initialized otherwise
    std::size_t dsize = 0;

public:
    // Container typdefs
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Constructors
    constexpr cvector(void) = default;
    constexpr cvector(size_type count, const T& value) : dsize(count) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = value;
    }

    // Iterators
    constexpr iterator begin() noexcept {
        return data;
    }
    constexpr iterator end() noexcept {
        return data + dsize;
    }

    // Capacity
    constexpr size_type size() const {
        return dsize;
    }

    // Element access
    constexpr reference operator[](std::size_t index) {
        return data[index];
    }
    constexpr const_reference operator[](std::size_t index) const {
        return data[index];
    }

    constexpr reference back() {
        return data[dsize - 1];
    }
    constexpr const_reference back() const {
        return data[dsize - 1];
    }

    // Modifiers
    constexpr void push_back(const T& a) {
        data[dsize++] = a;
    }
    constexpr void push_back(T&& a) {
        data[dsize++] = std::move(a);
    }
    constexpr void pop_back() {
        --dsize;
    }

    constexpr void clear() {
        dsize = 0;
    }
};

template <class T, std::size_t N>
class carray {
    T data_[N] = {}; // zero-initialization for scalar type T,
                     // default-initialized otherwise

    template <std::size_t M, std::size_t... I>
    constexpr carray(T const (&init)[M], std::index_sequence<I...>) :
        data_{init[I]...} {
    }
    template <class Iter, std::size_t... I>
    constexpr carray(Iter iter, std::index_sequence<I...>) :
        data_{((void)I, *iter++)...} {
    }

public:
    // Container typdefs
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Constructors
    constexpr carray(void) = default;
    template <std::size_t M>
    constexpr carray(T const (&init)[M]) :
        carray(init, std::make_index_sequence<N>()) {
        static_assert(M >= N,
                      "Cannot initialize a carray with an smaller array");
    }
    template <std::size_t M>
    constexpr carray(std::array<T, M> const& init) :
        carray(&init[0], std::make_index_sequence<N>()) {
        static_assert(M >= N,
                      "Cannot initialize a carray with an smaller array");
    }
    constexpr carray(std::initializer_list<T> init) :
        carray(init.begin(), std::make_index_sequence<N>()) {
        // clang & gcc doesn't recognize init.size() as a constexpr
        // static_assert(init.size() >= N, "Cannot initialize a carray with an
        // smaller initializer list");
    }

    // Iterators
    constexpr iterator begin() noexcept {
        return data_;
    }
    constexpr const_iterator begin() const noexcept {
        return data_;
    }
    constexpr const_iterator cbegin() const noexcept {
        return data_;
    }
    constexpr iterator end() noexcept {
        return data_ + N;
    }
    constexpr const_iterator end() const noexcept {
        return data_ + N;
    }
    constexpr const_iterator cend() const noexcept {
        return data_ + N;
    }

    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    // Capacity
    constexpr size_type size() const {
        return N;
    }
    constexpr size_type max_size() const {
        return N;
    }

    // Element access
    constexpr reference operator[](std::size_t index) {
        return data_[index];
    }
    constexpr const_reference operator[](std::size_t index) const {
        return data_[index];
    }

    constexpr reference at(std::size_t index) {
        if (index > N)
            FROZEN_THROW_OR_ABORT(std::out_of_range(
                "Index (" + std::to_string(index) + ") out of bound ("
                + std::to_string(N) + ')'));
        return data_[index];
    }
    constexpr const_reference at(std::size_t index) const {
        if (index > N)
            FROZEN_THROW_OR_ABORT(std::out_of_range(
                "Index (" + std::to_string(index) + ") out of bound ("
                + std::to_string(N) + ')'));
        return data_[index];
    }

    constexpr reference front() {
        return data_[0];
    }
    constexpr const_reference front() const {
        return data_[0];
    }

    constexpr reference back() {
        return data_[N - 1];
    }
    constexpr const_reference back() const {
        return data_[N - 1];
    }

    constexpr value_type* data() noexcept {
        return data_;
    }
    constexpr const value_type* data() const noexcept {
        return data_;
    }

    // Modifiers
    constexpr void fill(const value_type& val) {
        for (std::size_t i = 0; i < N; ++i)
            data_[i] = val;
    }
};
template <class T>
class carray<T, 0> {
public:
    // Container typdefs
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Constructors
    constexpr carray(void) = default;
};

}} // namespace frozen::bits

#include <cassert>

#ifdef _MSC_VER

// FIXME: find a way to implement that correctly for msvc
#define constexpr_assert(cond, msg)

#else

#define constexpr_assert(cond, msg) assert(cond&& msg);
#endif

#include <type_traits>
#include <array>
#include <limits>

#include <limits>

namespace frozen {

template <class T = void>
struct elsa {
    static_assert(std::is_integral<T>::value || std::is_enum<T>::value,
                  "only supports integral types, specialize for other types");

    constexpr std::size_t operator()(T const& value, std::size_t seed) const {
        std::size_t key = seed ^ static_cast<std::size_t>(value);
        key = (~key) + (key << 21); // key = (key << 21) - key - 1;
        key = key ^ (key >> 24);
        key = (key + (key << 3)) + (key << 8); // key * 265
        key = key ^ (key >> 14);
        key = (key + (key << 2)) + (key << 4); // key * 21
        key = key ^ (key >> 28);
        key = key + (key << 31);
        return key;
    }
};

template <>
struct elsa<void> {
    template <class T>
    constexpr std::size_t operator()(T const& value, std::size_t seed) const {
        return elsa<T>{}(value, seed);
    }
};

template <class T>
using anna = elsa<T>;
} // namespace frozen

namespace frozen {

template <typename _CharT>
class basic_string {
  using chr_t = _CharT;

  chr_t const *data_;
  std::size_t size_;

 public:
  template <std::size_t N>
  constexpr basic_string(chr_t const (&data)[N]) : data_(data), size_(N - 1) {}
  constexpr basic_string(chr_t const *data, std::size_t size)
      : data_(data), size_(size) {}

  constexpr basic_string(const basic_string &) noexcept = default;
  constexpr basic_string &operator=(const basic_string &) noexcept = default;

  constexpr std::size_t size() const { return size_; }

  constexpr chr_t operator[](std::size_t i) const { return data_[i]; }

  constexpr bool operator==(basic_string other) const {
    if (size_ != other.size_)
      return false;
    for (std::size_t i = 0; i < size_; ++i)
      if (data_[i] != other.data_[i])
        return false;
    return true;
  }

  constexpr bool operator<(const basic_string &other) const {
    unsigned i = 0;
    while (i < size() && i < other.size()) {
      if ((*this)[i] < other[i]) {
        return true;
      }
      if ((*this)[i] > other[i]) {
        return false;
      }
      ++i;
    }
    return size() < other.size();
  }

  constexpr const chr_t *data() const { return data_; }
  constexpr const chr_t *begin() const { return data(); }
  constexpr const chr_t *end() const { return data() + size(); }
};

template <typename String>
constexpr std::size_t hash_string(const String& value) {
  std::size_t d = 5381;
  for (const auto& c : value) d = d * 33 + static_cast<size_t>(c);
  return d;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// With the lowest bits removed, based on experimental setup.
template <typename String>
constexpr std::size_t hash_string(const String& value, std::size_t seed) {
  std::size_t d = (0x811c9dc5 ^ seed) * static_cast<size_t>(0x01000193);
  for (const auto& c : value)
    d = (d ^ static_cast<size_t>(c)) * static_cast<size_t>(0x01000193);
  return d >> 8;
}

template <typename _CharT>
struct elsa<basic_string<_CharT>> {
  constexpr std::size_t operator()(basic_string<_CharT> value) const {
    return hash_string(value);
  }
  constexpr std::size_t operator()(basic_string<_CharT> value,
                                   std::size_t seed) const {
    return hash_string(value, seed);
  }
};

}

namespace frozen { namespace bits {

auto constexpr next_highest_power_of_two(std::size_t v) {
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    constexpr auto trip_count = std::numeric_limits<decltype(v)>::digits;
    v--;
    for (std::size_t i = 1; i < trip_count; i <<= 1)
        v |= v >> i;
    v++;
    return v;
}

template <class T>
auto constexpr log(T v) {
    std::size_t n = 0;
    while (v > 1) {
        n += 1;
        v >>= 1;
    }
    return n;
}

constexpr std::size_t bit_weight(std::size_t n) {
    return (n <= 8 * sizeof(unsigned int)) + (n <= 8 * sizeof(unsigned long))
           + (n <= 8 * sizeof(unsigned long long)) + (n <= 128);
}

unsigned int select_uint_least(std::integral_constant<std::size_t, 4>);
unsigned long select_uint_least(std::integral_constant<std::size_t, 3>);


template <std::size_t N>
using select_uint_least_t = decltype(select_uint_least(
    std::integral_constant<std::size_t, bit_weight(N)>()));


template <class T>
constexpr void cswap(T& a, T& b) {
    auto tmp = a;
    a = b;
    b = tmp;
}

template <typename Iterator, class Compare>
constexpr Iterator partition(Iterator left, Iterator right,
                             Compare const& compare) {
    auto pivot = left + (right - left) / 2;
    auto value = *pivot;
    cswap(*right, *pivot);
    for (auto it = left; 0 < right - it; ++it) {
        if (compare(*it, value)) {
            cswap(*it, *left);
            left++;
        }
    }
    cswap(*right, *left);
    return left;
}

template <typename Iterator, class Compare>
constexpr void quicksort(Iterator left, Iterator right,
                         Compare const& compare) {
    while (0 < right - left) {
        auto new_pivot = bits::partition(left, right, compare);
        quicksort(left, new_pivot, compare);
        left = new_pivot + 1;
    }
}

template <typename T, std::size_t N, class Compare>
constexpr bits::carray<T, N> quicksort(bits::carray<T, N> const& array,
                                       Compare const& compare) {
    bits::carray<T, N> res = array;
    quicksort(res.begin(), res.end() - 1, compare);
    return res;
}


}} // namespace frozen::bits

namespace frozen { namespace bits {

// Function object for sorting buckets in decreasing order of size
struct bucket_size_compare {
    template <typename B>
    bool constexpr operator()(B const& b0, B const& b1) const {
        return b0.size() > b1.size();
    }
};

// Step One in pmh routine is to take all items and hash them into buckets,
// with some collisions. Then process those buckets further to build a perfect
// hash function.
// pmh_buckets represents the initial placement into buckets.

template <size_t M>
struct pmh_buckets {
    // Step 0: Bucket max is 2 * sqrt M
    // TODO: Come up with justification for this, should it not be O(log M)?
    static constexpr auto bucket_max = 2 * (1u << (log(M) / 2));

    using bucket_t = cvector<std::size_t, bucket_max>;
    carray<bucket_t, M> buckets;
    uint64_t seed;

    // Represents a reference to a bucket. This is used because the buckets
    // have to be sorted, but buckets are big, making it slower than sorting
    // refs
    struct bucket_ref {
        unsigned hash;
        const bucket_t* ptr;

        // Forward some interface of bucket
        using value_type = typename bucket_t::value_type;
        using const_iterator = typename bucket_t::const_iterator;

        constexpr auto size() const {
            return ptr->size();
        }
        constexpr const auto& operator[](std::size_t idx) const {
            return (*ptr)[idx];
        }
        constexpr auto begin() const {
            return ptr->begin();
        }
        constexpr auto end() const {
            return ptr->end();
        }
    };

    // Make a bucket_ref for each bucket
    template <std::size_t... Is>
    carray<bucket_ref, M> constexpr make_bucket_refs(
        std::index_sequence<Is...>) const {
        return {{bucket_ref{Is, &buckets[Is]}...}};
    }

    // Makes a bucket_ref for each bucket and sorts them by size
    carray<bucket_ref, M> constexpr get_sorted_buckets() const {
        carray<bucket_ref, M> result{
            this->make_bucket_refs(std::make_index_sequence<M>())};
        bits::quicksort(result.begin(), result.end() - 1,
                        bucket_size_compare{});
        return result;
    }
};

template <size_t M, class Item, size_t N, class Hash, class Key, class PRG>
pmh_buckets<M> constexpr make_pmh_buckets(const carray<Item, N>& items,
                                          Hash const& hash, Key const& key,
                                          PRG& prg) {
    using result_t = pmh_buckets<M>;
    result_t result{};
    bool rejected = false;
    // Continue until all items are placed without exceeding bucket_max
    while (1) {
        for (auto& b : result.buckets) {
            b.clear();
        }
        result.seed = prg();
        rejected = false;
        for (std::size_t i = 0; i < N; ++i) {
            auto& bucket = result.buckets[hash(key(items[i]),
                                               static_cast<size_t>(result.seed))
                                          % M];
            if (bucket.size() >= result_t::bucket_max) {
                rejected = true;
                break;
            }
            bucket.push_back(i);
        }
        if (!rejected) {
            return result;
        }
    }
}

// Check if an item appears in a cvector
template <class T, size_t N>
constexpr bool all_different_from(cvector<T, N>& data, T& a) {
    for (std::size_t i = 0; i < data.size(); ++i)
        if (data[i] == a)
            return false;

    return true;
}

// Represents either an index to a data item array, or a seed to be used with
// a hasher. Seed must have high bit of 1, value has high bit of zero.
struct seed_or_index {
    using value_type = uint64_t;

private:
    static constexpr value_type MINUS_ONE =
        (std::numeric_limits<value_type>::max)();
    static constexpr value_type HIGH_BIT = ~(MINUS_ONE >> 1);

    value_type value_ = 0;

public:
    constexpr value_type value() const {
        return value_;
    }
    constexpr bool is_seed() const {
        return value_ & HIGH_BIT;
    }

    constexpr seed_or_index(bool is_seed, value_type value) :
        value_(is_seed ? (value | HIGH_BIT) : (value & ~HIGH_BIT)) {
    }

    constexpr seed_or_index() = default;
    constexpr seed_or_index(const seed_or_index&) = default;
    constexpr seed_or_index& operator=(const seed_or_index&) = default;
};

// Represents the perfect hash function created by pmh algorithm
template <std::size_t M, class Hasher>
struct pmh_tables {
    uint64_t first_seed_;
    carray<seed_or_index, M> first_table_;
    carray<std::size_t, M> second_table_;
    Hasher hash_;

    template <typename KeyType>
    constexpr std::size_t lookup(const KeyType& key) const {
        return lookup(key, hash_);
    }

    // Looks up a given key, to find its expected index in carray<Item, N>
    // Always returns a valid index, must use KeyEqual test after to confirm.
    template <typename KeyType, typename HasherType>
    constexpr std::size_t lookup(const KeyType& key,
                                 const HasherType& hasher) const {
        auto const d =
            first_table_[hasher(key, static_cast<size_t>(first_seed_)) % M];
        if (!d.is_seed()) {
            return static_cast<std::size_t>(d.value());
        } // this is narrowing uint64 -> size_t but should be fine
        else {
            return second_table_[hasher(key,
                                        static_cast<std::size_t>(d.value()))
                                 % M];
        }
    }
};

// Make pmh tables for given items, hash function, prg, etc.
template <std::size_t M, class Item, std::size_t N, class Hash, class Key,
          class PRG>
pmh_tables<M, Hash> constexpr make_pmh_tables(const carray<Item, N>& items,
                                              Hash const& hash, Key const& key,
                                              PRG prg) {
    // Step 1: Place all of the keys into buckets
    auto step_one = make_pmh_buckets<M>(items, hash, key, prg);

    // Step 2: Sort the buckets to process the ones with the most items first.
    auto buckets = step_one.get_sorted_buckets();

    // G becomes the first hash table in the resulting pmh function
    carray<seed_or_index, M> G; // Default constructed to "index 0"

    // H becomes the second hash table in the resulting pmh function
    constexpr std::size_t UNUSED = (std::numeric_limits<std::size_t>::max)();
    carray<std::size_t, M> H;
    H.fill(UNUSED);

    // Step 3: Map the items in buckets into hash tables.
    for (const auto& bucket : buckets) {
        auto const bsize = bucket.size();

        if (bsize == 1) {
            // Store index to the (single) item in G
            // assert(bucket.hash == hash(key(items[bucket[0]]), step_one.seed)
            // % M);
            G[bucket.hash] = {false, static_cast<uint64_t>(bucket[0])};
        } else if (bsize > 1) {
            // Repeatedly try different H of d until we find a hash function
            // that places all items in the bucket into free slots
            seed_or_index d{true, prg()};
            cvector<std::size_t, decltype(step_one)::bucket_max> bucket_slots;

            while (bucket_slots.size() < bsize) {
                auto slot = hash(key(items[bucket[bucket_slots.size()]]),
                                 static_cast<size_t>(d.value()))
                            % M;

                if (H[slot] != UNUSED
                    || !all_different_from(bucket_slots, slot)) {
                    bucket_slots.clear();
                    d = {true, prg()};
                    continue;
                }

                bucket_slots.push_back(slot);
            }

            // Put successful seed in G, and put indices to items in their slots
            // assert(bucket.hash == hash(key(items[bucket[0]]), step_one.seed)
            // % M);
            G[bucket.hash] = d;
            for (std::size_t i = 0; i < bsize; ++i)
                H[bucket_slots[i]] = bucket[i];
        }
    }

    // Any unused entries in the H table have to get changed to zero.
    // This is because hashing should not fail or return an out-of-bounds entry.
    // A lookup fails after we apply user-supplied KeyEqual to the query and the
    // key found by hashing. Sending such queries to zero cannot hurt.
    for (std::size_t i = 0; i < M; ++i)
        if (H[i] == UNUSED)
            H[i] = 0;

    return {step_one.seed, G, H, hash};
}

}} // namespace frozen::bits

namespace frozen {
template <class UIntType, UIntType a, UIntType c, UIntType m>
class linear_congruential_engine {
    static_assert(std::is_unsigned<UIntType>::value,
                  "UIntType must be an unsigned integral type");

    template <class T>
    static constexpr UIntType modulo(T val,
                                     std::integral_constant<UIntType, 0>) {
        return static_cast<UIntType>(val);
    }

    template <class T, UIntType M>
    static constexpr UIntType modulo(T val,
                                     std::integral_constant<UIntType, M>) {
        // the static cast below may end up doing a truncation
        return static_cast<UIntType>(val % M);
    }

public:
    using result_type = UIntType;
    static constexpr result_type multiplier = a;
    static constexpr result_type increment = c;
    static constexpr result_type modulus = m;
    static constexpr result_type default_seed = 1u;

    linear_congruential_engine() = default;
    constexpr linear_congruential_engine(result_type s) {
        seed(s);
    }

    void seed(result_type s = default_seed) {
        state_ = s;
    }
    constexpr result_type operator()() {
        using uint_least_t =
            bits::select_uint_least_t<bits::log(a) + bits::log(m) + 4>;
        uint_least_t tmp =
            static_cast<uint_least_t>(multiplier) * state_ + increment;

        state_ = modulo(tmp, std::integral_constant<UIntType, modulus>());
        return state_;
    }
    // constexpr void discard(unsigned long long n) {
    //   while (n--) operator()();
    // }
    // static constexpr result_type min() { return increment == 0u ? 1u : 0u; }
    // static constexpr result_type max() { return modulus - 1u; }
    friend constexpr bool operator==(linear_congruential_engine const& self,
                                     linear_congruential_engine const& other) {
        return self.state_ == other.state_;
    }
    friend constexpr bool operator!=(linear_congruential_engine const& self,
                                     linear_congruential_engine const& other) {
        return !(self == other);
    }

private:
    result_type state_ = default_seed;
};


using minstd_rand =
    linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647>;

// This generator is used by default in unordered frozen containers
using default_prg_t = minstd_rand;

} // namespace frozen

namespace frozen {

namespace bits {

struct GetKey {
    template <class KV>
    constexpr auto const& operator()(KV const& kv) const {
        return kv.first;
    }
};

} // namespace bits

template <class Key, class Value, std::size_t N, typename Hash = anna<Key>,
          class KeyEqual = std::equal_to<Key>>
class unordered_map {
    static constexpr std::size_t storage_size =
        bits::next_highest_power_of_two(N)
        * (N < 32 ? 2 : 1); // size adjustment to prevent high collision rate
                            // for small sets
    using container_type = bits::carray<std::pair<Key, Value>, N>;
    using tables_type = bits::pmh_tables<storage_size, Hash>;

    KeyEqual const equal_;
    container_type items_;
    tables_type tables_;

public:
    /* typedefs */
    using Self = unordered_map<Key, Value, N, Hash, KeyEqual>;
    using key_type = Key;
    using mapped_type = Value;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using difference_type = typename container_type::difference_type;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer = typename container_type::pointer;
    using const_pointer = typename container_type::const_pointer;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

public:
    /* constructors */
    unordered_map(unordered_map const&) = default;
    constexpr unordered_map(container_type items, Hash const& hash,
                            KeyEqual const& equal) :
        equal_{equal}, items_{items},
        tables_{bits::make_pmh_tables<storage_size>(
            items_, hash, bits::GetKey{}, default_prg_t{})} {
    }

    explicit constexpr unordered_map(container_type items) :
        unordered_map{items, Hash{}, KeyEqual{}} {
    }

    constexpr unordered_map(std::initializer_list<value_type> items,
                            Hash const& hash, KeyEqual const& equal) :
        unordered_map{container_type{items}, hash, equal} {
        constexpr_assert(
            items.size() == N,
            "Inconsistent initializer_list size and type size argument");
    }

    constexpr unordered_map(std::initializer_list<value_type> items) :
        unordered_map{items, Hash{}, KeyEqual{}} {
    }

    /* iterators */
    constexpr iterator begin() {
        return items_.begin();
    }
    constexpr iterator end() {
        return items_.end();
    }
    constexpr const_iterator begin() const {
        return items_.begin();
    }
    constexpr const_iterator end() const {
        return items_.end();
    }
    constexpr const_iterator cbegin() const {
        return items_.cbegin();
    }
    constexpr const_iterator cend() const {
        return items_.cend();
    }

    /* capacity */
    constexpr bool empty() const {
        return !N;
    }
    constexpr size_type size() const {
        return N;
    }
    constexpr size_type max_size() const {
        return N;
    }

    /* lookup */
    template <class KeyType, class Hasher, class Equal>
    constexpr std::size_t count(KeyType const& key, Hasher const& hash,
                                Equal const& equal) const {
        auto const& kv = lookup(key, hash);
        return equal(kv.first, key);
    }
    template <class KeyType>
    constexpr std::size_t count(KeyType const& key) const {
        return count(key, hash_function(), key_eq());
    }

    template <class KeyType, class Hasher, class Equal>
    constexpr Value const& at(KeyType const& key, Hasher const& hash,
                              Equal const& equal) const {
        return at_impl(*this, key, hash, equal);
    }
    template <class KeyType, class Hasher, class Equal>
    constexpr Value& at(KeyType const& key, Hasher const& hash,
                        Equal const& equal) {
        return at_impl(*this, key, hash, equal);
    }
    template <class KeyType>
    constexpr Value const& at(KeyType const& key) const {
        return at(key, hash_function(), key_eq());
    }
    template <class KeyType>
    constexpr Value& at(KeyType const& key) {
        return at(key, hash_function(), key_eq());
    }

    template <class KeyType, class Hasher, class Equal>
    constexpr const_iterator find(KeyType const& key, Hasher const& hash,
                                  Equal const& equal) const {
        return find_impl(*this, key, hash, equal);
    }
    template <class KeyType, class Hasher, class Equal>
    constexpr iterator find(KeyType const& key, Hasher const& hash,
                            Equal const& equal) {
        return find_impl(*this, key, hash, equal);
    }
    template <class KeyType>
    constexpr const_iterator find(KeyType const& key) const {
        return find(key, hash_function(), key_eq());
    }
    template <class KeyType>
    constexpr iterator find(KeyType const& key) {
        return find(key, hash_function(), key_eq());
    }

    template <class KeyType, class Hasher, class Equal>
    constexpr std::pair<const_iterator, const_iterator>
    equal_range(KeyType const& key, Hasher const& hash,
                Equal const& equal) const {
        return equal_range_impl(*this, key, hash, equal);
    }
    template <class KeyType, class Hasher, class Equal>
    constexpr std::pair<iterator, iterator>
    equal_range(KeyType const& key, Hasher const& hash, Equal const& equal) {
        return equal_range_impl(*this, key, hash, equal);
    }
    template <class KeyType>
    constexpr std::pair<const_iterator, const_iterator>
    equal_range(KeyType const& key) const {
        return equal_range(key, hash_function(), key_eq());
    }
    template <class KeyType>
    constexpr std::pair<iterator, iterator> equal_range(KeyType const& key) {
        return equal_range(key, hash_function(), key_eq());
    }

    /* bucket interface */
    constexpr std::size_t bucket_count() const {
        return storage_size;
    }
    constexpr std::size_t max_bucket_count() const {
        return storage_size;
    }

    /* observers*/
    constexpr const hasher& hash_function() const {
        return tables_.hash_;
    }
    constexpr const key_equal& key_eq() const {
        return equal_;
    }

private:
    template <class This, class KeyType, class Hasher, class Equal>
    static inline constexpr auto& at_impl(This&& self, KeyType const& key,
                                          Hasher const& hash,
                                          Equal const& equal) {
        auto& kv = self.lookup(key, hash);
        if (equal(kv.first, key))
            return kv.second;
        else
            FROZEN_THROW_OR_ABORT(std::out_of_range("unknown key"));
    }

    template <class This, class KeyType, class Hasher, class Equal>
    static inline constexpr auto find_impl(This&& self, KeyType const& key,
                                           Hasher const& hash,
                                           Equal const& equal) {
        auto& kv = self.lookup(key, hash);
        if (equal(kv.first, key))
            return &kv;
        else
            return self.items_.end();
    }

    template <class This, class KeyType, class Hasher, class Equal>
    static inline constexpr auto
    equal_range_impl(This&& self, KeyType const& key, Hasher const& hash,
                     Equal const& equal) {
        auto& kv = self.lookup(key, hash);
        using kv_ptr = decltype(&kv);
        if (equal(kv.first, key))
            return std::pair<kv_ptr, kv_ptr>{&kv, &kv + 1};
        else
            return std::pair<kv_ptr, kv_ptr>{self.items_.end(),
                                             self.items_.end()};
    }

    template <class This, class KeyType, class Hasher>
    static inline constexpr auto& lookup_impl(This&& self, KeyType const& key,
                                              Hasher const& hash) {
        return self.items_[self.tables_.lookup(key, hash)];
    }

    template <class KeyType, class Hasher>
    constexpr auto const& lookup(KeyType const& key, Hasher const& hash) const {
        return lookup_impl(*this, key, hash);
    }
    template <class KeyType, class Hasher>
    constexpr auto& lookup(KeyType const& key, Hasher const& hash) {
        return lookup_impl(*this, key, hash);
    }
};

using string = basic_string<char>;

} // namespace frozen

int main() {
    frozen::unordered_map<frozen::string, std::size_t, 5> mp{
        {"1", 1},
        {"2", 1},
        {"3", 1},
        {"4", 1},
        {"5", 1},
    };
    if (mp.find("1") != mp.end())
        printf("1");
    if (mp.at("2") == 1)
        printf("2");
}

