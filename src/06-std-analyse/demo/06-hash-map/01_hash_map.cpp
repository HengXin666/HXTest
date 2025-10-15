#include <iostream>
#include <memory>
#include <vector>

/*
    MiAO 测试题目:
        完成下面两项内容, 语言不限:
        1、hashmap的数据结构
        2、hashmap的遍历、插入、删除功能
    
    说明:
        使用 C++标准为 C++20
        使用编译器为 Clang 20.1.8 x86_64-pc-linux-gnu
*/

// 使用链地址法, 实现应该简单些, 而且方便遍历

template <
    typename Key,
    typename Val,
    typename Hash,
    typename Eql
>
class HashTable;

template <typename T>
struct ListNode {
    template <typename U>
        requires (std::convertible_to<U, T>)
    ListNode(U&& u)
        : _data{std::forward<U>(u)}
    {}

    template <typename... Args>
        requires (std::is_constructible_v<T, Args...>)
    ListNode(Args&&... args)
        : _data{std::forward<Args>(args)...}
    {}

private:
    // 期望: 只读, 不可修改, 为视图
    ListNode* next() const {
        return _next.get();
    }

    template <typename, typename, typename, typename>
    friend struct HashIterator;

    template <typename, typename, typename, typename>
    friend class HashTable;

    T _data;
    std::unique_ptr<ListNode> _next{};
};

template <
    typename Key,
    typename Val,
    typename Hash,
    typename Eql
>
struct HashIterator {
    using key_type = Key;
    using value_type = Val;
    using mapped_type = std::pair<std::add_const_t<key_type>, value_type>;
    using table_type = std::vector<std::unique_ptr<ListNode<mapped_type>>>;
    using hash_type = Hash;
    using equal_type = Eql;

    HashIterator(std::nullptr_t)
        : _idx{static_cast<std::size_t>(-1)}
        , _node{nullptr}
        , _tables{nullptr}
    {}

    HashIterator(
        std::size_t idx,
        ListNode<mapped_type>* addr,
        HashTable<key_type, value_type, hash_type, equal_type>::table_type* tables
    )
        : _idx{idx}
        , _node{addr}
        , _tables{tables}
    {}

    mapped_type& operator*() {
        if (!_node) [[unlikely]] {
            // 迭代器无效, 不可解引用
            throw std::runtime_error{"iterator is invalid"};
        }
        return _node->_data;
    }

    mapped_type& operator*() const {
        if (!_node) [[unlikely]] {
            throw std::runtime_error{"iterator is invalid"};
        }
        return _node->_data;
    }

    mapped_type* operator->() {
        if (!_node) [[unlikely]] {
            throw std::runtime_error{"iterator is invalid"};
        }
        return &_node->_data;
    }

    mapped_type* operator->() const {
        if (!_node) [[unlikely]] {
            throw std::runtime_error{"iterator is invalid"};
        }
        return &_node->_data;
    }

    HashIterator const& operator++() {
        if (!(_node = _node->next())) {
            // 在哈希表中前进
            do {
                ++_idx;
            } while (_idx < _tables->size() && !_tables->at(_idx).get());
            if (_idx >= _tables->size()) [[unlikely]] {
                _node = nullptr; // end
            } else {
                _node = _tables->at(_idx).get();
            }
        }
        return *this;
    }

    HashIterator operator++(int) {
        HashIterator res{*this};
        this->operator++();
        return res;
    }

    bool operator==(HashIterator const& it) const noexcept {
        return _node == it._node;
    }

    bool operator!=(HashIterator const& it) const noexcept {
        return _node != it._node;
    }
private:
    template <typename, typename, typename, typename>
    friend class HashTable;

    std::size_t _idx;
    ListNode<mapped_type>* _node;
    HashTable<key_type, value_type, hash_type, equal_type>::table_type* _tables;
};

template <
    typename Key,
    typename Val,
    typename Hash = std::hash<Key>,
    typename Eql = std::equal_to<Key>
>
class HashTable {
    inline static constexpr float LoadFactor = 0.75; // 负载因子
    inline static constexpr std::size_t npos = static_cast<std::size_t>(-1);
public:
    using key_type = Key;
    using value_type = Val;
    using mapped_type = std::pair<std::add_const_t<key_type>, value_type>;
    using table_type = std::vector<std::unique_ptr<ListNode<mapped_type>>>;
    using hash_type = Hash;
    using equal_type = Eql;
    using iterator = HashIterator<key_type, value_type, hash_type, equal_type>;

    HashTable(std::size_t cnt)
        : _maxSize{cnt}
        , _size{0}
        , _tables(_maxSize)
    {}

    template <typename U>
        requires (std::convertible_to<U, Val>)
    void insert(Key const& key, U&& u) {
        if (static_cast<float>(_size) >= static_cast<float>(_maxSize) * LoadFactor) {
            expandCapacity();
        }
        std::size_t idx = hash(key);
        auto* head = &_tables[idx];
        if (!head->get()) {
            _tables[idx] = std::make_unique<ListNode<mapped_type>>(key, std::forward<U>(u));
            ++_size;
            if (_beginIdx > idx) {
                _beginIdx = idx;
            }
            return;
        }
        auto* mae = head->get();
        for (auto* node = head->get(); node; mae = node, node = node->next()) {
            if (Eql{}(key, node->_data.first)) {
                node->_data.second = std::forward<U>(u);
                return;
            }
        }
        ++_size;
        mae->_next.reset(new ListNode<mapped_type>{key, std::forward<U>(u)});
    }

    void erase(Key const& key) {
        std::size_t idx = hash(key);
        auto& v = _tables[idx];
        auto* head = &v;
        if (!head->get()) {
            return;
        }
        auto* mae = head->get();
        for (auto* node = head->get(); node; mae = node, node = node->next()) {
            if (Eql{}(key, node->_data.first)) {
                if (mae == node) [[unlikely]] {
                    v = std::move(v.get()->_next);
                    updateBegin(); // 更新 begin
                } else {
                    mae->_next = std::move(node->_next);
                }
                --_size;
                return;
            }
        }
    }

    void erase(iterator it) {
        if (!it._node)
            return; // 无效迭代器
        if (auto& v = _tables.at(it._idx); v.get() == it._node) {
            v = std::move(v.get()->_next);
            --_size;
            updateBegin();
        } else {
            auto* head = &v;
            auto* mae = head->get();
            for (auto* node = head->get(); node; mae = node, node = node->next()) {
                if (it._node == node) {
                    mae->_next = std::move(node->_next);
                    --_size;
                    return;
                }
            }
        }
    }

    iterator begin() noexcept {
        if (_beginIdx == npos) {
            return end();
        }
        return iterator{_beginIdx, _tables.at(_beginIdx).get(), &_tables};
    }

    iterator begin() const noexcept {
        if (_beginIdx == npos) {
            return end();
        }
        return iterator{_beginIdx, _tables.at(_beginIdx).get(), &_tables};
    }

    iterator end() noexcept {
        return iterator{nullptr};
    }

    iterator end() const noexcept {
        return iterator{nullptr};
    }

    void swap(HashTable& that) noexcept {
        std::swap(_maxSize, that._maxSize);
        std::swap(_size, that._size);
        std::swap(_tables, that._tables);
        std::swap(_beginIdx, that._beginIdx);
    }

    HashTable(HashTable const&) = delete;
    HashTable& operator=(HashTable const&) = delete;

    HashTable(HashTable&& that) noexcept {
        swap(that);
    }

    HashTable& operator=(HashTable&& that) noexcept {
        swap(that);
        return *this;
    }

    std::size_t size() const noexcept {
        return _size;
    }
private:
    std::size_t hash(Key const& key) const {
        // 需要保证 size 是 2 的幂次
        return _hash(key) & (_tables.size() - 1);
    }

    void updateBegin() noexcept {
        if (_size)
            for (; _beginIdx < _tables.size(); ++_beginIdx)
                if (_tables[_beginIdx])
                    return;
        _beginIdx = npos;
    }

    // 扩容
    void expandCapacity() {
        if (!_maxSize) [[unlikely]] {
            _maxSize = 1;
        }
        HashTable newTables(_maxSize << 1);
        for (auto&& [k, v] : *this) {
            newTables.insert(k, std::move(v));
        }
        swap(newTables);
    }

    std::size_t _maxSize;
    std::size_t _size;
    table_type _tables;
    std::size_t _beginIdx = npos;
    Hash _hash{};
};

template <
    typename Key,
    typename Val,
    typename Hash = std::hash<Key>,
    typename Eql = std::equal_to<Key>
>
class HashMap {
public:
    using key_type = Key;
    using value_type = Val;
    using mapped_type = std::pair<std::add_const_t<key_type>, value_type>;
    using hash_type = Hash;
    using equal_type = Eql;
    using iterator = HashIterator<key_type, value_type, hash_type, equal_type>;

    HashMap()
        : _tables{0}
    {}

    HashMap(std::size_t size)
        : _tables{std::bit_ceil(size ? size : 2u)} // 得 >= size 的二的幂
    {}

    template <typename U>
        requires (std::convertible_to<U, Val>)
    void insert(Key const& key, U&& u) {
        _tables.insert(key, std::forward<U>(u));
    }
    
    void erase(Key const& key) {
        return _tables.erase(key);
    }

    void erase(iterator it) {
        return _tables.erase(it);
    }

    iterator begin() {
        return _tables.begin();
    }

    iterator begin() const {
        return _tables.begin();
    }

    iterator end() {
        return _tables.end();
    }
    iterator end() const {
        return _tables.end();
    }

    std::size_t size() const noexcept {
        return _tables.size();
    }
private:
    HashTable<key_type, value_type, hash_type, equal_type> _tables;
};

int main() {
    HashMap<std::string, int> mp;
    // 1. 实现遍历 (基于迭代器)
    for (auto&& [k, v] : mp) {
        std::cout << k << ' ' << v << '\n';
    }
    std::cout << "=========\n\n";

    // 2. 可以插入数据
    mp.insert("123", 456);
    mp.insert("123", 666);
    mp.insert("你好", 2233);
    mp.insert("%#@!orz", -114514);
    for (auto&& [k, v] : mp) {
        std::cout << k << ' ' << v << '\n';
    }
    std::cout << "size: " << mp.size() << '\n';
    std::cout << "=========\n\n";

    // 3. 可以删除数据
    mp.erase("你好");
    for (auto&& [k, v] : mp) {
        std::cout << k << ' ' << v << '\n';
    }
    std::cout << "size: " << mp.size() << '\n';

    auto begin = mp.begin();
    mp.erase(begin++); // 支持通过 key / 迭代器 进行删除.

    std::cout << "=========\n\n";
    for (auto&& [k, v] : mp) {
        std::cout << k << ' ' << v << '\n';
    }
    std::cout << "size: " << mp.size() 
        << " begin == mp.begin(): " << (begin == mp.begin()) << '\n';

    std::cout << "=========\n\n";
    mp.erase(begin++);
    std::cout << "size: " << mp.size() 
        << " begin == mp.begin(): " << (begin == mp.begin()) 
        << " begin == mp.end(): " << (begin == mp.end()) << '\n';
}