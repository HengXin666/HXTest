#include <array>
#include <cassert>
#include <string_view>
#include <cstdint>

// 简单 FNV-1a 哈希, 可添加 seed
constexpr uint64_t fnv1a_seed(std::string_view str, uint64_t seed = 0) {
    constexpr uint64_t FNV_OFFSET = 1469598103934665603ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t h = FNV_OFFSET ^ seed;
    for (char c : str) {
        h ^= static_cast<uint8_t>(c);
        h *= FNV_PRIME;
    }
    return h;
}

constexpr uint64_t mix(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

// 通用哈希函数：整型原样返回，字符串使用 FNV-1a
template <typename Key>
constexpr uint64_t hash_func(const Key& key, uint64_t seed = 0) {
    if constexpr (std::is_integral_v<Key>) {
        uint64_t x = static_cast<uint64_t>(key);
        return mix(x + seed * 0x9e3779b97f4a7c15ULL);
    } else if constexpr (std::is_same_v<Key, std::string_view>) {
        return fnv1a_seed(key, seed);
    } else {
        static_assert(!sizeof(Key*), "Unsupported key type");
    }
}

template <typename Key, typename Value, size_t N, size_t M>
struct StaticPerfectHash {
    std::array<std::pair<Key, Value>, M> table{};
    std::array<bool, M> occupied{};

    constexpr size_t probe(const Key& key, size_t i) const {
        return (fnv1a_seed(key) + i) % M;
    }

    constexpr Value get(const Key& key) const {
        for (size_t i = 0; i < M; ++i) {
            size_t idx = probe(key, i);
            if (!occupied[idx])
                break; // 不存在
            if (table[idx].first == key)
                return table[idx].second;
        }
        return Value{-1};
    }
};

// 生成最小完美哈希函数（consteval 表示必须在编译期执行
// M 会影响编译速度, 实际测试, m 为 1.1 就 ok, 保险是 1.23, 快速编译就选 1.5,
// 最高设置为 2, 大于 2 的几乎没有收益
template <typename Key, typename Value, size_t N, size_t M = N>
consteval StaticPerfectHash<Key, Value, N, M>
make_perfect_hash(const std::array<std::pair<Key, Value>, N>& data) {
    StaticPerfectHash<Key, Value, N, M> ph{};

    for (auto [key, val] : data) {
        size_t dist = 0;
        size_t idx = fnv1a_seed(key) % M;

        while (true) {
            if (!ph.occupied[idx]) {
                ph.table[idx] = {key, val};
                ph.occupied[idx] = true;
                break;
            }
            // Robin Hood: 比较探测距离，交换
            size_t existing_idx = fnv1a_seed(ph.table[idx].first) % M;
            size_t existing_dist = (idx + M - existing_idx) % M;

            if (dist > existing_dist) {
                // 交换
                auto tmp = ph.table[idx];
                ph.table[idx] = {key, val};
                key = tmp.first;
                val = tmp.second;
                dist = existing_dist;
            }
            idx = (idx + 1) % M;
            ++dist;
            if (dist >= M) [[unlikely]]
                throw "Hash table is full, increase k!";
        }
    }
    return ph;
}

int main() {
}

constexpr std::array<std::pair<std::string_view, int>, 1> testData1() {
    return {{
        {"key000", 0},
    }};
}

static constexpr auto table1 = make_perfect_hash(testData1());

constexpr bool testCompileTime1() {
    constexpr auto data = testData1();
    for (size_t j = 0; j < 1; ++j) {
        if (table1.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime1(),
              "Perfect hash compile-time test failed for 1 keys.");

constexpr std::array<std::pair<std::string_view, int>, 2> testData2() {
    return {{
        {"key000", 0},
        {"key001", 1},
    }};
}

static constexpr auto table2 = make_perfect_hash(testData2());

constexpr bool testCompileTime2() {
    constexpr auto data = testData2();
    for (size_t j = 0; j < 2; ++j) {
        if (table2.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime2(),
              "Perfect hash compile-time test failed for 2 keys.");

constexpr std::array<std::pair<std::string_view, int>, 3> testData3() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
    }};
}

static constexpr auto table3 = make_perfect_hash(testData3());

constexpr bool testCompileTime3() {
    constexpr auto data = testData3();
    for (size_t j = 0; j < 3; ++j) {
        if (table3.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime3(),
              "Perfect hash compile-time test failed for 3 keys.");

constexpr std::array<std::pair<std::string_view, int>, 4> testData4() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
    }};
}

static constexpr auto table4 = make_perfect_hash(testData4());

constexpr bool testCompileTime4() {
    constexpr auto data = testData4();
    for (size_t j = 0; j < 4; ++j) {
        if (table4.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime4(),
              "Perfect hash compile-time test failed for 4 keys.");

constexpr std::array<std::pair<std::string_view, int>, 5> testData5() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
    }};
}

static constexpr auto table5 = make_perfect_hash(testData5());

constexpr bool testCompileTime5() {
    constexpr auto data = testData5();
    for (size_t j = 0; j < 5; ++j) {
        if (table5.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime5(),
              "Perfect hash compile-time test failed for 5 keys.");

constexpr std::array<std::pair<std::string_view, int>, 6> testData6() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
    }};
}

static constexpr auto table6 = make_perfect_hash(testData6());

constexpr bool testCompileTime6() {
    constexpr auto data = testData6();
    for (size_t j = 0; j < 6; ++j) {
        if (table6.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime6(),
              "Perfect hash compile-time test failed for 6 keys.");

constexpr std::array<std::pair<std::string_view, int>, 7> testData7() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
    }};
}

static constexpr auto table7 = make_perfect_hash(testData7());

constexpr bool testCompileTime7() {
    constexpr auto data = testData7();
    for (size_t j = 0; j < 7; ++j) {
        if (table7.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime7(),
              "Perfect hash compile-time test failed for 7 keys.");

constexpr std::array<std::pair<std::string_view, int>, 8> testData8() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
    }};
}

static constexpr auto table8 = make_perfect_hash(testData8());

constexpr bool testCompileTime8() {
    constexpr auto data = testData8();
    for (size_t j = 0; j < 8; ++j) {
        if (table8.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime8(),
              "Perfect hash compile-time test failed for 8 keys.");

constexpr std::array<std::pair<std::string_view, int>, 9> testData9() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
    }};
}

static constexpr auto table9 = make_perfect_hash(testData9());

constexpr bool testCompileTime9() {
    constexpr auto data = testData9();
    for (size_t j = 0; j < 9; ++j) {
        if (table9.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime9(),
              "Perfect hash compile-time test failed for 9 keys.");

constexpr std::array<std::pair<std::string_view, int>, 10> testData10() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
    }};
}

static constexpr auto table10 = make_perfect_hash(testData10());

constexpr bool testCompileTime10() {
    constexpr auto data = testData10();
    for (size_t j = 0; j < 10; ++j) {
        if (table10.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime10(),
              "Perfect hash compile-time test failed for 10 keys.");

constexpr std::array<std::pair<std::string_view, int>, 11> testData11() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
    }};
}

static constexpr auto table11 = make_perfect_hash(testData11());

constexpr bool testCompileTime11() {
    constexpr auto data = testData11();
    for (size_t j = 0; j < 11; ++j) {
        if (table11.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime11(),
              "Perfect hash compile-time test failed for 11 keys.");

constexpr std::array<std::pair<std::string_view, int>, 12> testData12() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
    }};
}

static constexpr auto table12 = make_perfect_hash(testData12());

constexpr bool testCompileTime12() {
    constexpr auto data = testData12();
    for (size_t j = 0; j < 12; ++j) {
        if (table12.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime12(),
              "Perfect hash compile-time test failed for 12 keys.");

constexpr std::array<std::pair<std::string_view, int>, 13> testData13() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
        {"key012", 12},
    }};
}

static constexpr auto table13 = make_perfect_hash(testData13());

constexpr bool testCompileTime13() {
    constexpr auto data = testData13();
    for (size_t j = 0; j < 13; ++j) {
        if (table13.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime13(),
              "Perfect hash compile-time test failed for 13 keys.");

constexpr std::array<std::pair<std::string_view, int>, 14> testData14() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
        {"key012", 12},
        {"key013", 13},
    }};
}

static constexpr auto table14 = make_perfect_hash(testData14());

constexpr bool testCompileTime14() {
    constexpr auto data = testData14();
    for (size_t j = 0; j < 14; ++j) {
        if (table14.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime14(),
              "Perfect hash compile-time test failed for 14 keys.");

constexpr std::array<std::pair<std::string_view, int>, 15> testData15() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
        {"key012", 12},
        {"key013", 13},
        {"key014", 14},
    }};
}

static constexpr auto table15 = make_perfect_hash(testData15());

constexpr bool testCompileTime15() {
    constexpr auto data = testData15();
    for (size_t j = 0; j < 15; ++j) {
        if (table15.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime15(),
              "Perfect hash compile-time test failed for 15 keys.");

constexpr std::array<std::pair<std::string_view, int>, 16> testData16() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
        {"key012", 12},
        {"key013", 13},
        {"key014", 14},
        {"key015", 15},
    }};
}

static constexpr auto table16 = make_perfect_hash(testData16());

constexpr bool testCompileTime16() {
    constexpr auto data = testData16();
    for (size_t j = 0; j < 16; ++j) {
        if (table16.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime16(),
              "Perfect hash compile-time test failed for 16 keys.");

constexpr std::array<std::pair<std::string_view, int>, 17> testData17() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
        {"key012", 12},
        {"key013", 13},
        {"key014", 14},
        {"key015", 15},
        {"key016", 16},
    }};
}

static constexpr auto table17 = make_perfect_hash(testData17());

constexpr bool testCompileTime17() {
    constexpr auto data = testData17();
    for (size_t j = 0; j < 17; ++j) {
        if (table17.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime17(),
              "Perfect hash compile-time test failed for 17 keys.");

constexpr std::array<std::pair<std::string_view, int>, 18> testData18() {
    return {{
        {"key000", 0},
        {"key001", 1},
        {"key002", 2},
        {"key003", 3},
        {"key004", 4},
        {"key005", 5},
        {"key006", 6},
        {"key007", 7},
        {"key008", 8},
        {"key009", 9},
        {"key010", 10},
        {"key011", 11},
        {"key012", 12},
        {"key013", 13},
        {"key014", 14},
        {"key015", 15},
        {"key016", 16},
        {"key017", 17},
    }};
}

static constexpr auto table18 = make_perfect_hash(testData18());

constexpr bool testCompileTime18() {
    constexpr auto data = testData18();
    for (size_t j = 0; j < 18; ++j) {
        if (table18.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime18(),
              "Perfect hash compile-time test failed for 18 keys.");

constexpr std::array<std::pair<std::string_view, int>, 19> testData19() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18},
    }};
}

static constexpr auto table19 = make_perfect_hash(testData19());

constexpr bool testCompileTime19() {
    constexpr auto data = testData19();
    for (size_t j = 0; j < 19; ++j) {
        if (table19.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime19(),
              "Perfect hash compile-time test failed for 19 keys.");

constexpr std::array<std::pair<std::string_view, int>, 20> testData20() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
    }};
}

static constexpr auto table20 = make_perfect_hash(testData20());

constexpr bool testCompileTime20() {
    constexpr auto data = testData20();
    for (size_t j = 0; j < 20; ++j) {
        if (table20.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime20(),
              "Perfect hash compile-time test failed for 20 keys.");

constexpr std::array<std::pair<std::string_view, int>, 21> testData21() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20},
    }};
}

static constexpr auto table21 = make_perfect_hash(testData21());

constexpr bool testCompileTime21() {
    constexpr auto data = testData21();
    for (size_t j = 0; j < 21; ++j) {
        if (table21.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime21(),
              "Perfect hash compile-time test failed for 21 keys.");

constexpr std::array<std::pair<std::string_view, int>, 22> testData22() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21},
    }};
}

static constexpr auto table22 = make_perfect_hash(testData22());

constexpr bool testCompileTime22() {
    constexpr auto data = testData22();
    for (size_t j = 0; j < 22; ++j) {
        if (table22.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime22(),
              "Perfect hash compile-time test failed for 22 keys.");

constexpr std::array<std::pair<std::string_view, int>, 23> testData23() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22},
    }};
}

static constexpr auto table23 = make_perfect_hash(testData23());

constexpr bool testCompileTime23() {
    constexpr auto data = testData23();
    for (size_t j = 0; j < 23; ++j) {
        if (table23.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime23(),
              "Perfect hash compile-time test failed for 23 keys.");

constexpr std::array<std::pair<std::string_view, int>, 24> testData24() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
    }};
}

static constexpr auto table24 = make_perfect_hash(testData24());

constexpr bool testCompileTime24() {
    constexpr auto data = testData24();
    for (size_t j = 0; j < 24; ++j) {
        if (table24.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime24(),
              "Perfect hash compile-time test failed for 24 keys.");

constexpr std::array<std::pair<std::string_view, int>, 25> testData25() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24},
    }};
}

static constexpr auto table25 = make_perfect_hash(testData25());

constexpr bool testCompileTime25() {
    constexpr auto data = testData25();
    for (size_t j = 0; j < 25; ++j) {
        if (table25.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime25(),
              "Perfect hash compile-time test failed for 25 keys.");

constexpr std::array<std::pair<std::string_view, int>, 26> testData26() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25},
    }};
}

static constexpr auto table26 = make_perfect_hash(testData26());

constexpr bool testCompileTime26() {
    constexpr auto data = testData26();
    for (size_t j = 0; j < 26; ++j) {
        if (table26.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime26(),
              "Perfect hash compile-time test failed for 26 keys.");

constexpr std::array<std::pair<std::string_view, int>, 27> testData27() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26},
    }};
}

static constexpr auto table27 = make_perfect_hash(testData27());

constexpr bool testCompileTime27() {
    constexpr auto data = testData27();
    for (size_t j = 0; j < 27; ++j) {
        if (table27.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime27(),
              "Perfect hash compile-time test failed for 27 keys.");

constexpr std::array<std::pair<std::string_view, int>, 28> testData28() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
    }};
}

static constexpr auto table28 = make_perfect_hash(testData28());

constexpr bool testCompileTime28() {
    constexpr auto data = testData28();
    for (size_t j = 0; j < 28; ++j) {
        if (table28.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime28(),
              "Perfect hash compile-time test failed for 28 keys.");

constexpr std::array<std::pair<std::string_view, int>, 29> testData29() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28},
    }};
}

static constexpr auto table29 = make_perfect_hash(testData29());

constexpr bool testCompileTime29() {
    constexpr auto data = testData29();
    for (size_t j = 0; j < 29; ++j) {
        if (table29.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime29(),
              "Perfect hash compile-time test failed for 29 keys.");

constexpr std::array<std::pair<std::string_view, int>, 30> testData30() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29},
    }};
}

static constexpr auto table30 = make_perfect_hash(testData30());

constexpr bool testCompileTime30() {
    constexpr auto data = testData30();
    for (size_t j = 0; j < 30; ++j) {
        if (table30.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime30(),
              "Perfect hash compile-time test failed for 30 keys.");

constexpr std::array<std::pair<std::string_view, int>, 31> testData31() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30},
    }};
}

static constexpr auto table31 = make_perfect_hash(testData31());

constexpr bool testCompileTime31() {
    constexpr auto data = testData31();
    for (size_t j = 0; j < 31; ++j) {
        if (table31.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime31(),
              "Perfect hash compile-time test failed for 31 keys.");

constexpr std::array<std::pair<std::string_view, int>, 32> testData32() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
    }};
}

static constexpr auto table32 = make_perfect_hash(testData32());

constexpr bool testCompileTime32() {
    constexpr auto data = testData32();
    for (size_t j = 0; j < 32; ++j) {
        if (table32.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime32(),
              "Perfect hash compile-time test failed for 32 keys.");

constexpr std::array<std::pair<std::string_view, int>, 33> testData33() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32},
    }};
}

static constexpr auto table33 = make_perfect_hash(testData33());

constexpr bool testCompileTime33() {
    constexpr auto data = testData33();
    for (size_t j = 0; j < 33; ++j) {
        if (table33.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime33(),
              "Perfect hash compile-time test failed for 33 keys.");

constexpr std::array<std::pair<std::string_view, int>, 34> testData34() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33},
    }};
}

static constexpr auto table34 = make_perfect_hash(testData34());

constexpr bool testCompileTime34() {
    constexpr auto data = testData34();
    for (size_t j = 0; j < 34; ++j) {
        if (table34.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime34(),
              "Perfect hash compile-time test failed for 34 keys.");

constexpr std::array<std::pair<std::string_view, int>, 35> testData35() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34},
    }};
}

static constexpr auto table35 = make_perfect_hash(testData35());

constexpr bool testCompileTime35() {
    constexpr auto data = testData35();
    for (size_t j = 0; j < 35; ++j) {
        if (table35.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime35(),
              "Perfect hash compile-time test failed for 35 keys.");

constexpr std::array<std::pair<std::string_view, int>, 36> testData36() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
    }};
}

static constexpr auto table36 = make_perfect_hash(testData36());

constexpr bool testCompileTime36() {
    constexpr auto data = testData36();
    for (size_t j = 0; j < 36; ++j) {
        if (table36.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime36(),
              "Perfect hash compile-time test failed for 36 keys.");

constexpr std::array<std::pair<std::string_view, int>, 37> testData37() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36},
    }};
}

static constexpr auto table37 = make_perfect_hash(testData37());

constexpr bool testCompileTime37() {
    constexpr auto data = testData37();
    for (size_t j = 0; j < 37; ++j) {
        if (table37.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime37(),
              "Perfect hash compile-time test failed for 37 keys.");

constexpr std::array<std::pair<std::string_view, int>, 38> testData38() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37},
    }};
}

static constexpr auto table38 = make_perfect_hash(testData38());

constexpr bool testCompileTime38() {
    constexpr auto data = testData38();
    for (size_t j = 0; j < 38; ++j) {
        if (table38.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime38(),
              "Perfect hash compile-time test failed for 38 keys.");

constexpr std::array<std::pair<std::string_view, int>, 39> testData39() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38},
    }};
}

static constexpr auto table39 = make_perfect_hash(testData39());

constexpr bool testCompileTime39() {
    constexpr auto data = testData39();
    for (size_t j = 0; j < 39; ++j) {
        if (table39.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime39(),
              "Perfect hash compile-time test failed for 39 keys.");

constexpr std::array<std::pair<std::string_view, int>, 40> testData40() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
    }};
}

static constexpr auto table40 = make_perfect_hash(testData40());

constexpr bool testCompileTime40() {
    constexpr auto data = testData40();
    for (size_t j = 0; j < 40; ++j) {
        if (table40.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime40(),
              "Perfect hash compile-time test failed for 40 keys.");

constexpr std::array<std::pair<std::string_view, int>, 41> testData41() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40},
    }};
}

static constexpr auto table41 = make_perfect_hash(testData41());

constexpr bool testCompileTime41() {
    constexpr auto data = testData41();
    for (size_t j = 0; j < 41; ++j) {
        if (table41.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime41(),
              "Perfect hash compile-time test failed for 41 keys.");

constexpr std::array<std::pair<std::string_view, int>, 42> testData42() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41},
    }};
}

static constexpr auto table42 = make_perfect_hash(testData42());

constexpr bool testCompileTime42() {
    constexpr auto data = testData42();
    for (size_t j = 0; j < 42; ++j) {
        if (table42.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime42(),
              "Perfect hash compile-time test failed for 42 keys.");

constexpr std::array<std::pair<std::string_view, int>, 43> testData43() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42},
    }};
}

static constexpr auto table43 = make_perfect_hash(testData43());

constexpr bool testCompileTime43() {
    constexpr auto data = testData43();
    for (size_t j = 0; j < 43; ++j) {
        if (table43.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime43(),
              "Perfect hash compile-time test failed for 43 keys.");

constexpr std::array<std::pair<std::string_view, int>, 44> testData44() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
    }};
}

static constexpr auto table44 = make_perfect_hash(testData44());

constexpr bool testCompileTime44() {
    constexpr auto data = testData44();
    for (size_t j = 0; j < 44; ++j) {
        if (table44.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime44(),
              "Perfect hash compile-time test failed for 44 keys.");

constexpr std::array<std::pair<std::string_view, int>, 45> testData45() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44},
    }};
}

static constexpr auto table45 = make_perfect_hash(testData45());

constexpr bool testCompileTime45() {
    constexpr auto data = testData45();
    for (size_t j = 0; j < 45; ++j) {
        if (table45.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime45(),
              "Perfect hash compile-time test failed for 45 keys.");

constexpr std::array<std::pair<std::string_view, int>, 46> testData46() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45},
    }};
}

static constexpr auto table46 = make_perfect_hash(testData46());

constexpr bool testCompileTime46() {
    constexpr auto data = testData46();
    for (size_t j = 0; j < 46; ++j) {
        if (table46.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime46(),
              "Perfect hash compile-time test failed for 46 keys.");

constexpr std::array<std::pair<std::string_view, int>, 47> testData47() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46},
    }};
}

static constexpr auto table47 = make_perfect_hash(testData47());

constexpr bool testCompileTime47() {
    constexpr auto data = testData47();
    for (size_t j = 0; j < 47; ++j) {
        if (table47.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime47(),
              "Perfect hash compile-time test failed for 47 keys.");

constexpr std::array<std::pair<std::string_view, int>, 48> testData48() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
    }};
}

static constexpr auto table48 = make_perfect_hash(testData48());

constexpr bool testCompileTime48() {
    constexpr auto data = testData48();
    for (size_t j = 0; j < 48; ++j) {
        if (table48.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime48(),
              "Perfect hash compile-time test failed for 48 keys.");

constexpr std::array<std::pair<std::string_view, int>, 49> testData49() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48},
    }};
}

static constexpr auto table49 = make_perfect_hash(testData49());

constexpr bool testCompileTime49() {
    constexpr auto data = testData49();
    for (size_t j = 0; j < 49; ++j) {
        if (table49.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime49(),
              "Perfect hash compile-time test failed for 49 keys.");

constexpr std::array<std::pair<std::string_view, int>, 50> testData50() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49},
    }};
}

static constexpr auto table50 = make_perfect_hash(testData50());

constexpr bool testCompileTime50() {
    constexpr auto data = testData50();
    for (size_t j = 0; j < 50; ++j) {
        if (table50.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime50(),
              "Perfect hash compile-time test failed for 50 keys.");

constexpr std::array<std::pair<std::string_view, int>, 51> testData51() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50},
    }};
}

static constexpr auto table51 = make_perfect_hash(testData51());

constexpr bool testCompileTime51() {
    constexpr auto data = testData51();
    for (size_t j = 0; j < 51; ++j) {
        if (table51.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime51(),
              "Perfect hash compile-time test failed for 51 keys.");

constexpr std::array<std::pair<std::string_view, int>, 52> testData52() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
    }};
}

static constexpr auto table52 = make_perfect_hash(testData52());

constexpr bool testCompileTime52() {
    constexpr auto data = testData52();
    for (size_t j = 0; j < 52; ++j) {
        if (table52.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime52(),
              "Perfect hash compile-time test failed for 52 keys.");

constexpr std::array<std::pair<std::string_view, int>, 53> testData53() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52},
    }};
}

static constexpr auto table53 = make_perfect_hash(testData53());

constexpr bool testCompileTime53() {
    constexpr auto data = testData53();
    for (size_t j = 0; j < 53; ++j) {
        if (table53.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime53(),
              "Perfect hash compile-time test failed for 53 keys.");

constexpr std::array<std::pair<std::string_view, int>, 54> testData54() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53},
    }};
}

static constexpr auto table54 = make_perfect_hash(testData54());

constexpr bool testCompileTime54() {
    constexpr auto data = testData54();
    for (size_t j = 0; j < 54; ++j) {
        if (table54.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime54(),
              "Perfect hash compile-time test failed for 54 keys.");

constexpr std::array<std::pair<std::string_view, int>, 55> testData55() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54},
    }};
}

static constexpr auto table55 = make_perfect_hash(testData55());

constexpr bool testCompileTime55() {
    constexpr auto data = testData55();
    for (size_t j = 0; j < 55; ++j) {
        if (table55.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime55(),
              "Perfect hash compile-time test failed for 55 keys.");

constexpr std::array<std::pair<std::string_view, int>, 56> testData56() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
    }};
}

static constexpr auto table56 = make_perfect_hash(testData56());

constexpr bool testCompileTime56() {
    constexpr auto data = testData56();
    for (size_t j = 0; j < 56; ++j) {
        if (table56.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime56(),
              "Perfect hash compile-time test failed for 56 keys.");

constexpr std::array<std::pair<std::string_view, int>, 57> testData57() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56},
    }};
}

static constexpr auto table57 = make_perfect_hash(testData57());

constexpr bool testCompileTime57() {
    constexpr auto data = testData57();
    for (size_t j = 0; j < 57; ++j) {
        if (table57.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime57(),
              "Perfect hash compile-time test failed for 57 keys.");

constexpr std::array<std::pair<std::string_view, int>, 58> testData58() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57},
    }};
}

static constexpr auto table58 = make_perfect_hash(testData58());

constexpr bool testCompileTime58() {
    constexpr auto data = testData58();
    for (size_t j = 0; j < 58; ++j) {
        if (table58.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime58(),
              "Perfect hash compile-time test failed for 58 keys.");

constexpr std::array<std::pair<std::string_view, int>, 59> testData59() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58},
    }};
}

static constexpr auto table59 = make_perfect_hash(testData59());

constexpr bool testCompileTime59() {
    constexpr auto data = testData59();
    for (size_t j = 0; j < 59; ++j) {
        if (table59.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime59(),
              "Perfect hash compile-time test failed for 59 keys.");

constexpr std::array<std::pair<std::string_view, int>, 60> testData60() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
    }};
}

static constexpr auto table60 = make_perfect_hash(testData60());

constexpr bool testCompileTime60() {
    constexpr auto data = testData60();
    for (size_t j = 0; j < 60; ++j) {
        if (table60.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime60(),
              "Perfect hash compile-time test failed for 60 keys.");

constexpr std::array<std::pair<std::string_view, int>, 61> testData61() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60},
    }};
}

static constexpr auto table61 = make_perfect_hash(testData61());

constexpr bool testCompileTime61() {
    constexpr auto data = testData61();
    for (size_t j = 0; j < 61; ++j) {
        if (table61.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime61(),
              "Perfect hash compile-time test failed for 61 keys.");

constexpr std::array<std::pair<std::string_view, int>, 62> testData62() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61},
    }};
}

static constexpr auto table62 = make_perfect_hash(testData62());

constexpr bool testCompileTime62() {
    constexpr auto data = testData62();
    for (size_t j = 0; j < 62; ++j) {
        if (table62.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime62(),
              "Perfect hash compile-time test failed for 62 keys.");

constexpr std::array<std::pair<std::string_view, int>, 63> testData63() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62},
    }};
}

static constexpr auto table63 = make_perfect_hash(testData63());

constexpr bool testCompileTime63() {
    constexpr auto data = testData63();
    for (size_t j = 0; j < 63; ++j) {
        if (table63.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime63(),
              "Perfect hash compile-time test failed for 63 keys.");

constexpr std::array<std::pair<std::string_view, int>, 64> testData64() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
    }};
}

static constexpr auto table64 = make_perfect_hash(testData64());

constexpr bool testCompileTime64() {
    constexpr auto data = testData64();
    for (size_t j = 0; j < 64; ++j) {
        if (table64.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime64(),
              "Perfect hash compile-time test failed for 64 keys.");

constexpr std::array<std::pair<std::string_view, int>, 65> testData65() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64},
    }};
}

static constexpr auto table65 = make_perfect_hash(testData65());

constexpr bool testCompileTime65() {
    constexpr auto data = testData65();
    for (size_t j = 0; j < 65; ++j) {
        if (table65.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime65(),
              "Perfect hash compile-time test failed for 65 keys.");

constexpr std::array<std::pair<std::string_view, int>, 66> testData66() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65},
    }};
}

static constexpr auto table66 = make_perfect_hash(testData66());

constexpr bool testCompileTime66() {
    constexpr auto data = testData66();
    for (size_t j = 0; j < 66; ++j) {
        if (table66.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime66(),
              "Perfect hash compile-time test failed for 66 keys.");

constexpr std::array<std::pair<std::string_view, int>, 67> testData67() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66},
    }};
}

static constexpr auto table67 = make_perfect_hash(testData67());

constexpr bool testCompileTime67() {
    constexpr auto data = testData67();
    for (size_t j = 0; j < 67; ++j) {
        if (table67.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime67(),
              "Perfect hash compile-time test failed for 67 keys.");

constexpr std::array<std::pair<std::string_view, int>, 68> testData68() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
    }};
}

static constexpr auto table68 = make_perfect_hash(testData68());

constexpr bool testCompileTime68() {
    constexpr auto data = testData68();
    for (size_t j = 0; j < 68; ++j) {
        if (table68.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime68(),
              "Perfect hash compile-time test failed for 68 keys.");

constexpr std::array<std::pair<std::string_view, int>, 69> testData69() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68},
    }};
}

static constexpr auto table69 = make_perfect_hash(testData69());

constexpr bool testCompileTime69() {
    constexpr auto data = testData69();
    for (size_t j = 0; j < 69; ++j) {
        if (table69.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime69(),
              "Perfect hash compile-time test failed for 69 keys.");

constexpr std::array<std::pair<std::string_view, int>, 70> testData70() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69},
    }};
}

static constexpr auto table70 = make_perfect_hash(testData70());

constexpr bool testCompileTime70() {
    constexpr auto data = testData70();
    for (size_t j = 0; j < 70; ++j) {
        if (table70.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime70(),
              "Perfect hash compile-time test failed for 70 keys.");

constexpr std::array<std::pair<std::string_view, int>, 71> testData71() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70},
    }};
}

static constexpr auto table71 = make_perfect_hash(testData71());

constexpr bool testCompileTime71() {
    constexpr auto data = testData71();
    for (size_t j = 0; j < 71; ++j) {
        if (table71.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime71(),
              "Perfect hash compile-time test failed for 71 keys.");

constexpr std::array<std::pair<std::string_view, int>, 72> testData72() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
    }};
}

static constexpr auto table72 = make_perfect_hash(testData72());

constexpr bool testCompileTime72() {
    constexpr auto data = testData72();
    for (size_t j = 0; j < 72; ++j) {
        if (table72.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime72(),
              "Perfect hash compile-time test failed for 72 keys.");

constexpr std::array<std::pair<std::string_view, int>, 73> testData73() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72},
    }};
}

static constexpr auto table73 = make_perfect_hash(testData73());

constexpr bool testCompileTime73() {
    constexpr auto data = testData73();
    for (size_t j = 0; j < 73; ++j) {
        if (table73.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime73(),
              "Perfect hash compile-time test failed for 73 keys.");

constexpr std::array<std::pair<std::string_view, int>, 74> testData74() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73},
    }};
}

static constexpr auto table74 = make_perfect_hash(testData74());

constexpr bool testCompileTime74() {
    constexpr auto data = testData74();
    for (size_t j = 0; j < 74; ++j) {
        if (table74.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime74(),
              "Perfect hash compile-time test failed for 74 keys.");

constexpr std::array<std::pair<std::string_view, int>, 75> testData75() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74},
    }};
}

static constexpr auto table75 = make_perfect_hash(testData75());

constexpr bool testCompileTime75() {
    constexpr auto data = testData75();
    for (size_t j = 0; j < 75; ++j) {
        if (table75.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime75(),
              "Perfect hash compile-time test failed for 75 keys.");

constexpr std::array<std::pair<std::string_view, int>, 76> testData76() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
    }};
}

static constexpr auto table76 = make_perfect_hash(testData76());

constexpr bool testCompileTime76() {
    constexpr auto data = testData76();
    for (size_t j = 0; j < 76; ++j) {
        if (table76.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime76(),
              "Perfect hash compile-time test failed for 76 keys.");

constexpr std::array<std::pair<std::string_view, int>, 77> testData77() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76},
    }};
}

static constexpr auto table77 = make_perfect_hash(testData77());

constexpr bool testCompileTime77() {
    constexpr auto data = testData77();
    for (size_t j = 0; j < 77; ++j) {
        if (table77.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime77(),
              "Perfect hash compile-time test failed for 77 keys.");

constexpr std::array<std::pair<std::string_view, int>, 78> testData78() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77},
    }};
}

static constexpr auto table78 = make_perfect_hash(testData78());

constexpr bool testCompileTime78() {
    constexpr auto data = testData78();
    for (size_t j = 0; j < 78; ++j) {
        if (table78.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime78(),
              "Perfect hash compile-time test failed for 78 keys.");

constexpr std::array<std::pair<std::string_view, int>, 79> testData79() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78},
    }};
}

static constexpr auto table79 = make_perfect_hash(testData79());

constexpr bool testCompileTime79() {
    constexpr auto data = testData79();
    for (size_t j = 0; j < 79; ++j) {
        if (table79.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime79(),
              "Perfect hash compile-time test failed for 79 keys.");

constexpr std::array<std::pair<std::string_view, int>, 80> testData80() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
    }};
}

static constexpr auto table80 = make_perfect_hash(testData80());

constexpr bool testCompileTime80() {
    constexpr auto data = testData80();
    for (size_t j = 0; j < 80; ++j) {
        if (table80.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime80(),
              "Perfect hash compile-time test failed for 80 keys.");

constexpr std::array<std::pair<std::string_view, int>, 81> testData81() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80},
    }};
}

static constexpr auto table81 = make_perfect_hash(testData81());

constexpr bool testCompileTime81() {
    constexpr auto data = testData81();
    for (size_t j = 0; j < 81; ++j) {
        if (table81.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime81(),
              "Perfect hash compile-time test failed for 81 keys.");

constexpr std::array<std::pair<std::string_view, int>, 82> testData82() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81},
    }};
}

static constexpr auto table82 = make_perfect_hash(testData82());

constexpr bool testCompileTime82() {
    constexpr auto data = testData82();
    for (size_t j = 0; j < 82; ++j) {
        if (table82.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime82(),
              "Perfect hash compile-time test failed for 82 keys.");

constexpr std::array<std::pair<std::string_view, int>, 83> testData83() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82},
    }};
}

static constexpr auto table83 = make_perfect_hash(testData83());

constexpr bool testCompileTime83() {
    constexpr auto data = testData83();
    for (size_t j = 0; j < 83; ++j) {
        if (table83.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime83(),
              "Perfect hash compile-time test failed for 83 keys.");

constexpr std::array<std::pair<std::string_view, int>, 84> testData84() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
    }};
}

static constexpr auto table84 = make_perfect_hash(testData84());

constexpr bool testCompileTime84() {
    constexpr auto data = testData84();
    for (size_t j = 0; j < 84; ++j) {
        if (table84.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime84(),
              "Perfect hash compile-time test failed for 84 keys.");

constexpr std::array<std::pair<std::string_view, int>, 85> testData85() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84},
    }};
}

static constexpr auto table85 = make_perfect_hash(testData85());

constexpr bool testCompileTime85() {
    constexpr auto data = testData85();
    for (size_t j = 0; j < 85; ++j) {
        if (table85.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime85(),
              "Perfect hash compile-time test failed for 85 keys.");

constexpr std::array<std::pair<std::string_view, int>, 86> testData86() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85},
    }};
}

static constexpr auto table86 = make_perfect_hash(testData86());

constexpr bool testCompileTime86() {
    constexpr auto data = testData86();
    for (size_t j = 0; j < 86; ++j) {
        if (table86.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime86(),
              "Perfect hash compile-time test failed for 86 keys.");

constexpr std::array<std::pair<std::string_view, int>, 87> testData87() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86},
    }};
}

static constexpr auto table87 = make_perfect_hash(testData87());

constexpr bool testCompileTime87() {
    constexpr auto data = testData87();
    for (size_t j = 0; j < 87; ++j) {
        if (table87.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime87(),
              "Perfect hash compile-time test failed for 87 keys.");

constexpr std::array<std::pair<std::string_view, int>, 88> testData88() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
    }};
}

static constexpr auto table88 = make_perfect_hash(testData88());

constexpr bool testCompileTime88() {
    constexpr auto data = testData88();
    for (size_t j = 0; j < 88; ++j) {
        if (table88.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime88(),
              "Perfect hash compile-time test failed for 88 keys.");

constexpr std::array<std::pair<std::string_view, int>, 89> testData89() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88},
    }};
}

static constexpr auto table89 = make_perfect_hash(testData89());

constexpr bool testCompileTime89() {
    constexpr auto data = testData89();
    for (size_t j = 0; j < 89; ++j) {
        if (table89.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime89(),
              "Perfect hash compile-time test failed for 89 keys.");

constexpr std::array<std::pair<std::string_view, int>, 90> testData90() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89},
    }};
}

static constexpr auto table90 = make_perfect_hash(testData90());

constexpr bool testCompileTime90() {
    constexpr auto data = testData90();
    for (size_t j = 0; j < 90; ++j) {
        if (table90.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime90(),
              "Perfect hash compile-time test failed for 90 keys.");

constexpr std::array<std::pair<std::string_view, int>, 91> testData91() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90},
    }};
}

static constexpr auto table91 = make_perfect_hash(testData91());

constexpr bool testCompileTime91() {
    constexpr auto data = testData91();
    for (size_t j = 0; j < 91; ++j) {
        if (table91.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime91(),
              "Perfect hash compile-time test failed for 91 keys.");

constexpr std::array<std::pair<std::string_view, int>, 92> testData92() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
    }};
}

static constexpr auto table92 = make_perfect_hash(testData92());

constexpr bool testCompileTime92() {
    constexpr auto data = testData92();
    for (size_t j = 0; j < 92; ++j) {
        if (table92.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime92(),
              "Perfect hash compile-time test failed for 92 keys.");

constexpr std::array<std::pair<std::string_view, int>, 93> testData93() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92},
    }};
}

static constexpr auto table93 = make_perfect_hash(testData93());

constexpr bool testCompileTime93() {
    constexpr auto data = testData93();
    for (size_t j = 0; j < 93; ++j) {
        if (table93.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime93(),
              "Perfect hash compile-time test failed for 93 keys.");

constexpr std::array<std::pair<std::string_view, int>, 94> testData94() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93},
    }};
}

static constexpr auto table94 = make_perfect_hash(testData94());

constexpr bool testCompileTime94() {
    constexpr auto data = testData94();
    for (size_t j = 0; j < 94; ++j) {
        if (table94.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime94(),
              "Perfect hash compile-time test failed for 94 keys.");

constexpr std::array<std::pair<std::string_view, int>, 95> testData95() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93}, {"key094", 94},
    }};
}

static constexpr auto table95 = make_perfect_hash(testData95());

constexpr bool testCompileTime95() {
    constexpr auto data = testData95();
    for (size_t j = 0; j < 95; ++j) {
        if (table95.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime95(),
              "Perfect hash compile-time test failed for 95 keys.");

constexpr std::array<std::pair<std::string_view, int>, 96> testData96() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93}, {"key094", 94}, {"key095", 95},
    }};
}

static constexpr auto table96 = make_perfect_hash(testData96());

constexpr bool testCompileTime96() {
    constexpr auto data = testData96();
    for (size_t j = 0; j < 96; ++j) {
        if (table96.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime96(),
              "Perfect hash compile-time test failed for 96 keys.");

constexpr std::array<std::pair<std::string_view, int>, 97> testData97() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93}, {"key094", 94}, {"key095", 95},
        {"key096", 96},
    }};
}

static constexpr auto table97 = make_perfect_hash(testData97());

constexpr bool testCompileTime97() {
    constexpr auto data = testData97();
    for (size_t j = 0; j < 97; ++j) {
        if (table97.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime97(),
              "Perfect hash compile-time test failed for 97 keys.");

constexpr std::array<std::pair<std::string_view, int>, 98> testData98() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93}, {"key094", 94}, {"key095", 95},
        {"key096", 96}, {"key097", 97},
    }};
}

static constexpr auto table98 = make_perfect_hash(testData98());

constexpr bool testCompileTime98() {
    constexpr auto data = testData98();
    for (size_t j = 0; j < 98; ++j) {
        if (table98.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime98(),
              "Perfect hash compile-time test failed for 98 keys.");

constexpr std::array<std::pair<std::string_view, int>, 99> testData99() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93}, {"key094", 94}, {"key095", 95},
        {"key096", 96}, {"key097", 97}, {"key098", 98},
    }};
}

static constexpr auto table99 = make_perfect_hash(testData99());

constexpr bool testCompileTime99() {
    constexpr auto data = testData99();
    for (size_t j = 0; j < 99; ++j) {
        if (table99.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime99(),
              "Perfect hash compile-time test failed for 99 keys.");

constexpr std::array<std::pair<std::string_view, int>, 100> testData100() {
    return {{
        {"key000", 0},  {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},  {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},  {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12}, {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16}, {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20}, {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24}, {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28}, {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32}, {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36}, {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40}, {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44}, {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48}, {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52}, {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56}, {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60}, {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64}, {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68}, {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72}, {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76}, {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80}, {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84}, {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88}, {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92}, {"key093", 93}, {"key094", 94}, {"key095", 95},
        {"key096", 96}, {"key097", 97}, {"key098", 98}, {"key099", 99},
    }};
}

static constexpr auto table100 = make_perfect_hash(testData100());

constexpr bool testCompileTime100() {
    constexpr auto data = testData100();
    for (size_t j = 0; j < 100; ++j) {
        if (table100.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime100(),
              "Perfect hash compile-time test failed for 100 keys.");

constexpr std::array<std::pair<std::string_view, int>, 101> testData101() {
    return {{
        {"key000", 0},   {"key001", 1},  {"key002", 2},  {"key003", 3},
        {"key004", 4},   {"key005", 5},  {"key006", 6},  {"key007", 7},
        {"key008", 8},   {"key009", 9},  {"key010", 10}, {"key011", 11},
        {"key012", 12},  {"key013", 13}, {"key014", 14}, {"key015", 15},
        {"key016", 16},  {"key017", 17}, {"key018", 18}, {"key019", 19},
        {"key020", 20},  {"key021", 21}, {"key022", 22}, {"key023", 23},
        {"key024", 24},  {"key025", 25}, {"key026", 26}, {"key027", 27},
        {"key028", 28},  {"key029", 29}, {"key030", 30}, {"key031", 31},
        {"key032", 32},  {"key033", 33}, {"key034", 34}, {"key035", 35},
        {"key036", 36},  {"key037", 37}, {"key038", 38}, {"key039", 39},
        {"key040", 40},  {"key041", 41}, {"key042", 42}, {"key043", 43},
        {"key044", 44},  {"key045", 45}, {"key046", 46}, {"key047", 47},
        {"key048", 48},  {"key049", 49}, {"key050", 50}, {"key051", 51},
        {"key052", 52},  {"key053", 53}, {"key054", 54}, {"key055", 55},
        {"key056", 56},  {"key057", 57}, {"key058", 58}, {"key059", 59},
        {"key060", 60},  {"key061", 61}, {"key062", 62}, {"key063", 63},
        {"key064", 64},  {"key065", 65}, {"key066", 66}, {"key067", 67},
        {"key068", 68},  {"key069", 69}, {"key070", 70}, {"key071", 71},
        {"key072", 72},  {"key073", 73}, {"key074", 74}, {"key075", 75},
        {"key076", 76},  {"key077", 77}, {"key078", 78}, {"key079", 79},
        {"key080", 80},  {"key081", 81}, {"key082", 82}, {"key083", 83},
        {"key084", 84},  {"key085", 85}, {"key086", 86}, {"key087", 87},
        {"key088", 88},  {"key089", 89}, {"key090", 90}, {"key091", 91},
        {"key092", 92},  {"key093", 93}, {"key094", 94}, {"key095", 95},
        {"key096", 96},  {"key097", 97}, {"key098", 98}, {"key099", 99},
        {"key100", 100},
    }};
}

static constexpr auto table101 = make_perfect_hash(testData101());

constexpr bool testCompileTime101() {
    constexpr auto data = testData101();
    for (size_t j = 0; j < 101; ++j) {
        if (table101.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime101(),
              "Perfect hash compile-time test failed for 101 keys.");

constexpr std::array<std::pair<std::string_view, int>, 102> testData102() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},  {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},  {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10}, {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14}, {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18}, {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22}, {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26}, {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30}, {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34}, {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38}, {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42}, {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46}, {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50}, {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54}, {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58}, {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62}, {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66}, {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70}, {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74}, {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78}, {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82}, {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86}, {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90}, {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94}, {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98}, {"key099", 99},
        {"key100", 100}, {"key101", 101},
    }};
}

static constexpr auto table102 = make_perfect_hash(testData102());

constexpr bool testCompileTime102() {
    constexpr auto data = testData102();
    for (size_t j = 0; j < 102; ++j) {
        if (table102.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime102(),
              "Perfect hash compile-time test failed for 102 keys.");

constexpr std::array<std::pair<std::string_view, int>, 103> testData103() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102},
    }};
}

static constexpr auto table103 = make_perfect_hash(testData103());

constexpr bool testCompileTime103() {
    constexpr auto data = testData103();
    for (size_t j = 0; j < 103; ++j) {
        if (table103.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime103(),
              "Perfect hash compile-time test failed for 103 keys.");

constexpr std::array<std::pair<std::string_view, int>, 104> testData104() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
    }};
}

static constexpr auto table104 = make_perfect_hash(testData104());

constexpr bool testCompileTime104() {
    constexpr auto data = testData104();
    for (size_t j = 0; j < 104; ++j) {
        if (table104.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime104(),
              "Perfect hash compile-time test failed for 104 keys.");

constexpr std::array<std::pair<std::string_view, int>, 105> testData105() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104},
    }};
}

static constexpr auto table105 = make_perfect_hash(testData105());

constexpr bool testCompileTime105() {
    constexpr auto data = testData105();
    for (size_t j = 0; j < 105; ++j) {
        if (table105.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime105(),
              "Perfect hash compile-time test failed for 105 keys.");

constexpr std::array<std::pair<std::string_view, int>, 106> testData106() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105},
    }};
}

static constexpr auto table106 = make_perfect_hash(testData106());

constexpr bool testCompileTime106() {
    constexpr auto data = testData106();
    for (size_t j = 0; j < 106; ++j) {
        if (table106.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime106(),
              "Perfect hash compile-time test failed for 106 keys.");

constexpr std::array<std::pair<std::string_view, int>, 107> testData107() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106},
    }};
}

static constexpr auto table107 = make_perfect_hash(testData107());

constexpr bool testCompileTime107() {
    constexpr auto data = testData107();
    for (size_t j = 0; j < 107; ++j) {
        if (table107.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime107(),
              "Perfect hash compile-time test failed for 107 keys.");

constexpr std::array<std::pair<std::string_view, int>, 108> testData108() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
    }};
}

static constexpr auto table108 = make_perfect_hash(testData108());

constexpr bool testCompileTime108() {
    constexpr auto data = testData108();
    for (size_t j = 0; j < 108; ++j) {
        if (table108.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime108(),
              "Perfect hash compile-time test failed for 108 keys.");

constexpr std::array<std::pair<std::string_view, int>, 109> testData109() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108},
    }};
}

static constexpr auto table109 = make_perfect_hash(testData109());

constexpr bool testCompileTime109() {
    constexpr auto data = testData109();
    for (size_t j = 0; j < 109; ++j) {
        if (table109.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime109(),
              "Perfect hash compile-time test failed for 109 keys.");

constexpr std::array<std::pair<std::string_view, int>, 110> testData110() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109},
    }};
}

static constexpr auto table110 = make_perfect_hash(testData110());

constexpr bool testCompileTime110() {
    constexpr auto data = testData110();
    for (size_t j = 0; j < 110; ++j) {
        if (table110.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime110(),
              "Perfect hash compile-time test failed for 110 keys.");

constexpr std::array<std::pair<std::string_view, int>, 111> testData111() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110},
    }};
}

static constexpr auto table111 = make_perfect_hash(testData111());

constexpr bool testCompileTime111() {
    constexpr auto data = testData111();
    for (size_t j = 0; j < 111; ++j) {
        if (table111.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime111(),
              "Perfect hash compile-time test failed for 111 keys.");

constexpr std::array<std::pair<std::string_view, int>, 112> testData112() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
    }};
}

static constexpr auto table112 = make_perfect_hash(testData112());

constexpr bool testCompileTime112() {
    constexpr auto data = testData112();
    for (size_t j = 0; j < 112; ++j) {
        if (table112.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime112(),
              "Perfect hash compile-time test failed for 112 keys.");

constexpr std::array<std::pair<std::string_view, int>, 113> testData113() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112},
    }};
}

static constexpr auto table113 = make_perfect_hash(testData113());

constexpr bool testCompileTime113() {
    constexpr auto data = testData113();
    for (size_t j = 0; j < 113; ++j) {
        if (table113.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime113(),
              "Perfect hash compile-time test failed for 113 keys.");

constexpr std::array<std::pair<std::string_view, int>, 114> testData114() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113},
    }};
}

static constexpr auto table114 = make_perfect_hash(testData114());

constexpr bool testCompileTime114() {
    constexpr auto data = testData114();
    for (size_t j = 0; j < 114; ++j) {
        if (table114.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime114(),
              "Perfect hash compile-time test failed for 114 keys.");

constexpr std::array<std::pair<std::string_view, int>, 115> testData115() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114},
    }};
}

static constexpr auto table115 = make_perfect_hash(testData115());

constexpr bool testCompileTime115() {
    constexpr auto data = testData115();
    for (size_t j = 0; j < 115; ++j) {
        if (table115.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime115(),
              "Perfect hash compile-time test failed for 115 keys.");

constexpr std::array<std::pair<std::string_view, int>, 116> testData116() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
    }};
}

static constexpr auto table116 = make_perfect_hash(testData116());

constexpr bool testCompileTime116() {
    constexpr auto data = testData116();
    for (size_t j = 0; j < 116; ++j) {
        if (table116.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime116(),
              "Perfect hash compile-time test failed for 116 keys.");

constexpr std::array<std::pair<std::string_view, int>, 117> testData117() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116},
    }};
}

static constexpr auto table117 = make_perfect_hash(testData117());

constexpr bool testCompileTime117() {
    constexpr auto data = testData117();
    for (size_t j = 0; j < 117; ++j) {
        if (table117.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime117(),
              "Perfect hash compile-time test failed for 117 keys.");

constexpr std::array<std::pair<std::string_view, int>, 118> testData118() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117},
    }};
}

static constexpr auto table118 = make_perfect_hash(testData118());

constexpr bool testCompileTime118() {
    constexpr auto data = testData118();
    for (size_t j = 0; j < 118; ++j) {
        if (table118.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime118(),
              "Perfect hash compile-time test failed for 118 keys.");

constexpr std::array<std::pair<std::string_view, int>, 119> testData119() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118},
    }};
}

static constexpr auto table119 = make_perfect_hash(testData119());

constexpr bool testCompileTime119() {
    constexpr auto data = testData119();
    for (size_t j = 0; j < 119; ++j) {
        if (table119.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime119(),
              "Perfect hash compile-time test failed for 119 keys.");

constexpr std::array<std::pair<std::string_view, int>, 120> testData120() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
    }};
}

static constexpr auto table120 = make_perfect_hash(testData120());

constexpr bool testCompileTime120() {
    constexpr auto data = testData120();
    for (size_t j = 0; j < 120; ++j) {
        if (table120.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime120(),
              "Perfect hash compile-time test failed for 120 keys.");

constexpr std::array<std::pair<std::string_view, int>, 121> testData121() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120},
    }};
}

static constexpr auto table121 = make_perfect_hash(testData121());

constexpr bool testCompileTime121() {
    constexpr auto data = testData121();
    for (size_t j = 0; j < 121; ++j) {
        if (table121.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime121(),
              "Perfect hash compile-time test failed for 121 keys.");

constexpr std::array<std::pair<std::string_view, int>, 122> testData122() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121},
    }};
}

static constexpr auto table122 = make_perfect_hash(testData122());

constexpr bool testCompileTime122() {
    constexpr auto data = testData122();
    for (size_t j = 0; j < 122; ++j) {
        if (table122.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime122(),
              "Perfect hash compile-time test failed for 122 keys.");

constexpr std::array<std::pair<std::string_view, int>, 123> testData123() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122},
    }};
}

static constexpr auto table123 = make_perfect_hash(testData123());

constexpr bool testCompileTime123() {
    constexpr auto data = testData123();
    for (size_t j = 0; j < 123; ++j) {
        if (table123.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime123(),
              "Perfect hash compile-time test failed for 123 keys.");

constexpr std::array<std::pair<std::string_view, int>, 124> testData124() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
    }};
}

static constexpr auto table124 = make_perfect_hash(testData124());

constexpr bool testCompileTime124() {
    constexpr auto data = testData124();
    for (size_t j = 0; j < 124; ++j) {
        if (table124.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime124(),
              "Perfect hash compile-time test failed for 124 keys.");

constexpr std::array<std::pair<std::string_view, int>, 125> testData125() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124},
    }};
}

static constexpr auto table125 = make_perfect_hash(testData125());

constexpr bool testCompileTime125() {
    constexpr auto data = testData125();
    for (size_t j = 0; j < 125; ++j) {
        if (table125.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime125(),
              "Perfect hash compile-time test failed for 125 keys.");

constexpr std::array<std::pair<std::string_view, int>, 126> testData126() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125},
    }};
}

static constexpr auto table126 = make_perfect_hash(testData126());

constexpr bool testCompileTime126() {
    constexpr auto data = testData126();
    for (size_t j = 0; j < 126; ++j) {
        if (table126.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime126(),
              "Perfect hash compile-time test failed for 126 keys.");

constexpr std::array<std::pair<std::string_view, int>, 127> testData127() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126},
    }};
}

static constexpr auto table127 = make_perfect_hash(testData127());

constexpr bool testCompileTime127() {
    constexpr auto data = testData127();
    for (size_t j = 0; j < 127; ++j) {
        if (table127.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime127(),
              "Perfect hash compile-time test failed for 127 keys.");

constexpr std::array<std::pair<std::string_view, int>, 128> testData128() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
    }};
}

static constexpr auto table128 = make_perfect_hash(testData128());

constexpr bool testCompileTime128() {
    constexpr auto data = testData128();
    for (size_t j = 0; j < 128; ++j) {
        if (table128.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime128(),
              "Perfect hash compile-time test failed for 128 keys.");

constexpr std::array<std::pair<std::string_view, int>, 129> testData129() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128},
    }};
}

static constexpr auto table129 = make_perfect_hash(testData129());

constexpr bool testCompileTime129() {
    constexpr auto data = testData129();
    for (size_t j = 0; j < 129; ++j) {
        if (table129.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime129(),
              "Perfect hash compile-time test failed for 129 keys.");

constexpr std::array<std::pair<std::string_view, int>, 130> testData130() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129},
    }};
}

static constexpr auto table130 = make_perfect_hash(testData130());

constexpr bool testCompileTime130() {
    constexpr auto data = testData130();
    for (size_t j = 0; j < 130; ++j) {
        if (table130.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime130(),
              "Perfect hash compile-time test failed for 130 keys.");

constexpr std::array<std::pair<std::string_view, int>, 131> testData131() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130},
    }};
}

static constexpr auto table131 = make_perfect_hash(testData131());

constexpr bool testCompileTime131() {
    constexpr auto data = testData131();
    for (size_t j = 0; j < 131; ++j) {
        if (table131.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime131(),
              "Perfect hash compile-time test failed for 131 keys.");

constexpr std::array<std::pair<std::string_view, int>, 132> testData132() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
    }};
}

static constexpr auto table132 = make_perfect_hash(testData132());

constexpr bool testCompileTime132() {
    constexpr auto data = testData132();
    for (size_t j = 0; j < 132; ++j) {
        if (table132.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime132(),
              "Perfect hash compile-time test failed for 132 keys.");

constexpr std::array<std::pair<std::string_view, int>, 133> testData133() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132},
    }};
}

static constexpr auto table133 = make_perfect_hash(testData133());

constexpr bool testCompileTime133() {
    constexpr auto data = testData133();
    for (size_t j = 0; j < 133; ++j) {
        if (table133.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime133(),
              "Perfect hash compile-time test failed for 133 keys.");

constexpr std::array<std::pair<std::string_view, int>, 134> testData134() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133},
    }};
}

static constexpr auto table134 = make_perfect_hash(testData134());

constexpr bool testCompileTime134() {
    constexpr auto data = testData134();
    for (size_t j = 0; j < 134; ++j) {
        if (table134.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime134(),
              "Perfect hash compile-time test failed for 134 keys.");

constexpr std::array<std::pair<std::string_view, int>, 135> testData135() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134},
    }};
}

static constexpr auto table135 = make_perfect_hash(testData135());

constexpr bool testCompileTime135() {
    constexpr auto data = testData135();
    for (size_t j = 0; j < 135; ++j) {
        if (table135.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime135(),
              "Perfect hash compile-time test failed for 135 keys.");

constexpr std::array<std::pair<std::string_view, int>, 136> testData136() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
    }};
}

static constexpr auto table136 = make_perfect_hash(testData136());

constexpr bool testCompileTime136() {
    constexpr auto data = testData136();
    for (size_t j = 0; j < 136; ++j) {
        if (table136.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime136(),
              "Perfect hash compile-time test failed for 136 keys.");

constexpr std::array<std::pair<std::string_view, int>, 137> testData137() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136},
    }};
}

static constexpr auto table137 = make_perfect_hash(testData137());

constexpr bool testCompileTime137() {
    constexpr auto data = testData137();
    for (size_t j = 0; j < 137; ++j) {
        if (table137.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime137(),
              "Perfect hash compile-time test failed for 137 keys.");

constexpr std::array<std::pair<std::string_view, int>, 138> testData138() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137},
    }};
}

static constexpr auto table138 = make_perfect_hash(testData138());

constexpr bool testCompileTime138() {
    constexpr auto data = testData138();
    for (size_t j = 0; j < 138; ++j) {
        if (table138.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime138(),
              "Perfect hash compile-time test failed for 138 keys.");

constexpr std::array<std::pair<std::string_view, int>, 139> testData139() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138},
    }};
}

static constexpr auto table139 = make_perfect_hash(testData139());

constexpr bool testCompileTime139() {
    constexpr auto data = testData139();
    for (size_t j = 0; j < 139; ++j) {
        if (table139.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime139(),
              "Perfect hash compile-time test failed for 139 keys.");

constexpr std::array<std::pair<std::string_view, int>, 140> testData140() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
    }};
}

static constexpr auto table140 = make_perfect_hash(testData140());

constexpr bool testCompileTime140() {
    constexpr auto data = testData140();
    for (size_t j = 0; j < 140; ++j) {
        if (table140.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime140(),
              "Perfect hash compile-time test failed for 140 keys.");

constexpr std::array<std::pair<std::string_view, int>, 141> testData141() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140},
    }};
}

static constexpr auto table141 = make_perfect_hash(testData141());

constexpr bool testCompileTime141() {
    constexpr auto data = testData141();
    for (size_t j = 0; j < 141; ++j) {
        if (table141.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime141(),
              "Perfect hash compile-time test failed for 141 keys.");

constexpr std::array<std::pair<std::string_view, int>, 142> testData142() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141},
    }};
}

static constexpr auto table142 = make_perfect_hash(testData142());

constexpr bool testCompileTime142() {
    constexpr auto data = testData142();
    for (size_t j = 0; j < 142; ++j) {
        if (table142.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime142(),
              "Perfect hash compile-time test failed for 142 keys.");

constexpr std::array<std::pair<std::string_view, int>, 143> testData143() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142},
    }};
}

static constexpr auto table143 = make_perfect_hash(testData143());

constexpr bool testCompileTime143() {
    constexpr auto data = testData143();
    for (size_t j = 0; j < 143; ++j) {
        if (table143.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime143(),
              "Perfect hash compile-time test failed for 143 keys.");

constexpr std::array<std::pair<std::string_view, int>, 144> testData144() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
    }};
}

static constexpr auto table144 = make_perfect_hash(testData144());

constexpr bool testCompileTime144() {
    constexpr auto data = testData144();
    for (size_t j = 0; j < 144; ++j) {
        if (table144.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime144(),
              "Perfect hash compile-time test failed for 144 keys.");

constexpr std::array<std::pair<std::string_view, int>, 145> testData145() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144},
    }};
}

static constexpr auto table145 = make_perfect_hash(testData145());

constexpr bool testCompileTime145() {
    constexpr auto data = testData145();
    for (size_t j = 0; j < 145; ++j) {
        if (table145.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime145(),
              "Perfect hash compile-time test failed for 145 keys.");

constexpr std::array<std::pair<std::string_view, int>, 146> testData146() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145},
    }};
}

static constexpr auto table146 = make_perfect_hash(testData146());

constexpr bool testCompileTime146() {
    constexpr auto data = testData146();
    for (size_t j = 0; j < 146; ++j) {
        if (table146.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime146(),
              "Perfect hash compile-time test failed for 146 keys.");

constexpr std::array<std::pair<std::string_view, int>, 147> testData147() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146},
    }};
}

static constexpr auto table147 = make_perfect_hash(testData147());

constexpr bool testCompileTime147() {
    constexpr auto data = testData147();
    for (size_t j = 0; j < 147; ++j) {
        if (table147.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime147(),
              "Perfect hash compile-time test failed for 147 keys.");

constexpr std::array<std::pair<std::string_view, int>, 148> testData148() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
    }};
}

static constexpr auto table148 = make_perfect_hash(testData148());

constexpr bool testCompileTime148() {
    constexpr auto data = testData148();
    for (size_t j = 0; j < 148; ++j) {
        if (table148.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime148(),
              "Perfect hash compile-time test failed for 148 keys.");

constexpr std::array<std::pair<std::string_view, int>, 149> testData149() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148},
    }};
}

static constexpr auto table149 = make_perfect_hash(testData149());

constexpr bool testCompileTime149() {
    constexpr auto data = testData149();
    for (size_t j = 0; j < 149; ++j) {
        if (table149.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime149(),
              "Perfect hash compile-time test failed for 149 keys.");

constexpr std::array<std::pair<std::string_view, int>, 150> testData150() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149},
    }};
}

static constexpr auto table150 = make_perfect_hash(testData150());

constexpr bool testCompileTime150() {
    constexpr auto data = testData150();
    for (size_t j = 0; j < 150; ++j) {
        if (table150.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime150(),
              "Perfect hash compile-time test failed for 150 keys.");

constexpr std::array<std::pair<std::string_view, int>, 151> testData151() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150},
    }};
}

static constexpr auto table151 = make_perfect_hash(testData151());

constexpr bool testCompileTime151() {
    constexpr auto data = testData151();
    for (size_t j = 0; j < 151; ++j) {
        if (table151.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime151(),
              "Perfect hash compile-time test failed for 151 keys.");

constexpr std::array<std::pair<std::string_view, int>, 152> testData152() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
    }};
}

static constexpr auto table152 = make_perfect_hash(testData152());

constexpr bool testCompileTime152() {
    constexpr auto data = testData152();
    for (size_t j = 0; j < 152; ++j) {
        if (table152.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime152(),
              "Perfect hash compile-time test failed for 152 keys.");

constexpr std::array<std::pair<std::string_view, int>, 153> testData153() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152},
    }};
}

static constexpr auto table153 = make_perfect_hash(testData153());

constexpr bool testCompileTime153() {
    constexpr auto data = testData153();
    for (size_t j = 0; j < 153; ++j) {
        if (table153.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime153(),
              "Perfect hash compile-time test failed for 153 keys.");

constexpr std::array<std::pair<std::string_view, int>, 154> testData154() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153},
    }};
}

static constexpr auto table154 = make_perfect_hash(testData154());

constexpr bool testCompileTime154() {
    constexpr auto data = testData154();
    for (size_t j = 0; j < 154; ++j) {
        if (table154.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime154(),
              "Perfect hash compile-time test failed for 154 keys.");

constexpr std::array<std::pair<std::string_view, int>, 155> testData155() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154},
    }};
}

static constexpr auto table155 = make_perfect_hash(testData155());

constexpr bool testCompileTime155() {
    constexpr auto data = testData155();
    for (size_t j = 0; j < 155; ++j) {
        if (table155.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime155(),
              "Perfect hash compile-time test failed for 155 keys.");

constexpr std::array<std::pair<std::string_view, int>, 156> testData156() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
    }};
}

static constexpr auto table156 = make_perfect_hash(testData156());

constexpr bool testCompileTime156() {
    constexpr auto data = testData156();
    for (size_t j = 0; j < 156; ++j) {
        if (table156.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime156(),
              "Perfect hash compile-time test failed for 156 keys.");

constexpr std::array<std::pair<std::string_view, int>, 157> testData157() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156},
    }};
}

static constexpr auto table157 = make_perfect_hash(testData157());

constexpr bool testCompileTime157() {
    constexpr auto data = testData157();
    for (size_t j = 0; j < 157; ++j) {
        if (table157.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime157(),
              "Perfect hash compile-time test failed for 157 keys.");

constexpr std::array<std::pair<std::string_view, int>, 158> testData158() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157},
    }};
}

static constexpr auto table158 = make_perfect_hash(testData158());

constexpr bool testCompileTime158() {
    constexpr auto data = testData158();
    for (size_t j = 0; j < 158; ++j) {
        if (table158.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime158(),
              "Perfect hash compile-time test failed for 158 keys.");

constexpr std::array<std::pair<std::string_view, int>, 159> testData159() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158},
    }};
}

static constexpr auto table159 = make_perfect_hash(testData159());

constexpr bool testCompileTime159() {
    constexpr auto data = testData159();
    for (size_t j = 0; j < 159; ++j) {
        if (table159.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime159(),
              "Perfect hash compile-time test failed for 159 keys.");

constexpr std::array<std::pair<std::string_view, int>, 160> testData160() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
    }};
}

static constexpr auto table160 = make_perfect_hash(testData160());

constexpr bool testCompileTime160() {
    constexpr auto data = testData160();
    for (size_t j = 0; j < 160; ++j) {
        if (table160.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime160(),
              "Perfect hash compile-time test failed for 160 keys.");

constexpr std::array<std::pair<std::string_view, int>, 161> testData161() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160},
    }};
}

static constexpr auto table161 = make_perfect_hash(testData161());

constexpr bool testCompileTime161() {
    constexpr auto data = testData161();
    for (size_t j = 0; j < 161; ++j) {
        if (table161.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime161(),
              "Perfect hash compile-time test failed for 161 keys.");

constexpr std::array<std::pair<std::string_view, int>, 162> testData162() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161},
    }};
}

static constexpr auto table162 = make_perfect_hash(testData162());

constexpr bool testCompileTime162() {
    constexpr auto data = testData162();
    for (size_t j = 0; j < 162; ++j) {
        if (table162.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime162(),
              "Perfect hash compile-time test failed for 162 keys.");

constexpr std::array<std::pair<std::string_view, int>, 163> testData163() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162},
    }};
}

static constexpr auto table163 = make_perfect_hash(testData163());

constexpr bool testCompileTime163() {
    constexpr auto data = testData163();
    for (size_t j = 0; j < 163; ++j) {
        if (table163.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime163(),
              "Perfect hash compile-time test failed for 163 keys.");

constexpr std::array<std::pair<std::string_view, int>, 164> testData164() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
    }};
}

static constexpr auto table164 = make_perfect_hash(testData164());

constexpr bool testCompileTime164() {
    constexpr auto data = testData164();
    for (size_t j = 0; j < 164; ++j) {
        if (table164.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime164(),
              "Perfect hash compile-time test failed for 164 keys.");

constexpr std::array<std::pair<std::string_view, int>, 165> testData165() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164},
    }};
}

static constexpr auto table165 = make_perfect_hash(testData165());

constexpr bool testCompileTime165() {
    constexpr auto data = testData165();
    for (size_t j = 0; j < 165; ++j) {
        if (table165.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime165(),
              "Perfect hash compile-time test failed for 165 keys.");

constexpr std::array<std::pair<std::string_view, int>, 166> testData166() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165},
    }};
}

static constexpr auto table166 = make_perfect_hash(testData166());

constexpr bool testCompileTime166() {
    constexpr auto data = testData166();
    for (size_t j = 0; j < 166; ++j) {
        if (table166.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime166(),
              "Perfect hash compile-time test failed for 166 keys.");

constexpr std::array<std::pair<std::string_view, int>, 167> testData167() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166},
    }};
}

static constexpr auto table167 = make_perfect_hash(testData167());

constexpr bool testCompileTime167() {
    constexpr auto data = testData167();
    for (size_t j = 0; j < 167; ++j) {
        if (table167.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime167(),
              "Perfect hash compile-time test failed for 167 keys.");

constexpr std::array<std::pair<std::string_view, int>, 168> testData168() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
    }};
}

static constexpr auto table168 = make_perfect_hash(testData168());

constexpr bool testCompileTime168() {
    constexpr auto data = testData168();
    for (size_t j = 0; j < 168; ++j) {
        if (table168.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime168(),
              "Perfect hash compile-time test failed for 168 keys.");

constexpr std::array<std::pair<std::string_view, int>, 169> testData169() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168},
    }};
}

static constexpr auto table169 = make_perfect_hash(testData169());

constexpr bool testCompileTime169() {
    constexpr auto data = testData169();
    for (size_t j = 0; j < 169; ++j) {
        if (table169.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime169(),
              "Perfect hash compile-time test failed for 169 keys.");

constexpr std::array<std::pair<std::string_view, int>, 170> testData170() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169},
    }};
}

static constexpr auto table170 = make_perfect_hash(testData170());

constexpr bool testCompileTime170() {
    constexpr auto data = testData170();
    for (size_t j = 0; j < 170; ++j) {
        if (table170.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime170(),
              "Perfect hash compile-time test failed for 170 keys.");

constexpr std::array<std::pair<std::string_view, int>, 171> testData171() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170},
    }};
}

static constexpr auto table171 = make_perfect_hash(testData171());

constexpr bool testCompileTime171() {
    constexpr auto data = testData171();
    for (size_t j = 0; j < 171; ++j) {
        if (table171.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime171(),
              "Perfect hash compile-time test failed for 171 keys.");

constexpr std::array<std::pair<std::string_view, int>, 172> testData172() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
    }};
}

static constexpr auto table172 = make_perfect_hash(testData172());

constexpr bool testCompileTime172() {
    constexpr auto data = testData172();
    for (size_t j = 0; j < 172; ++j) {
        if (table172.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime172(),
              "Perfect hash compile-time test failed for 172 keys.");

constexpr std::array<std::pair<std::string_view, int>, 173> testData173() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172},
    }};
}

static constexpr auto table173 = make_perfect_hash(testData173());

constexpr bool testCompileTime173() {
    constexpr auto data = testData173();
    for (size_t j = 0; j < 173; ++j) {
        if (table173.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime173(),
              "Perfect hash compile-time test failed for 173 keys.");

constexpr std::array<std::pair<std::string_view, int>, 174> testData174() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173},
    }};
}

static constexpr auto table174 = make_perfect_hash(testData174());

constexpr bool testCompileTime174() {
    constexpr auto data = testData174();
    for (size_t j = 0; j < 174; ++j) {
        if (table174.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime174(),
              "Perfect hash compile-time test failed for 174 keys.");

constexpr std::array<std::pair<std::string_view, int>, 175> testData175() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174},
    }};
}

static constexpr auto table175 = make_perfect_hash(testData175());

constexpr bool testCompileTime175() {
    constexpr auto data = testData175();
    for (size_t j = 0; j < 175; ++j) {
        if (table175.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime175(),
              "Perfect hash compile-time test failed for 175 keys.");

constexpr std::array<std::pair<std::string_view, int>, 176> testData176() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
    }};
}

static constexpr auto table176 = make_perfect_hash(testData176());

constexpr bool testCompileTime176() {
    constexpr auto data = testData176();
    for (size_t j = 0; j < 176; ++j) {
        if (table176.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime176(),
              "Perfect hash compile-time test failed for 176 keys.");

constexpr std::array<std::pair<std::string_view, int>, 177> testData177() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176},
    }};
}

static constexpr auto table177 = make_perfect_hash(testData177());

constexpr bool testCompileTime177() {
    constexpr auto data = testData177();
    for (size_t j = 0; j < 177; ++j) {
        if (table177.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime177(),
              "Perfect hash compile-time test failed for 177 keys.");

constexpr std::array<std::pair<std::string_view, int>, 178> testData178() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177},
    }};
}

static constexpr auto table178 = make_perfect_hash(testData178());

constexpr bool testCompileTime178() {
    constexpr auto data = testData178();
    for (size_t j = 0; j < 178; ++j) {
        if (table178.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime178(),
              "Perfect hash compile-time test failed for 178 keys.");

constexpr std::array<std::pair<std::string_view, int>, 179> testData179() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178},
    }};
}

static constexpr auto table179 = make_perfect_hash(testData179());

constexpr bool testCompileTime179() {
    constexpr auto data = testData179();
    for (size_t j = 0; j < 179; ++j) {
        if (table179.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime179(),
              "Perfect hash compile-time test failed for 179 keys.");

constexpr std::array<std::pair<std::string_view, int>, 180> testData180() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
    }};
}

static constexpr auto table180 = make_perfect_hash(testData180());

constexpr bool testCompileTime180() {
    constexpr auto data = testData180();
    for (size_t j = 0; j < 180; ++j) {
        if (table180.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime180(),
              "Perfect hash compile-time test failed for 180 keys.");

constexpr std::array<std::pair<std::string_view, int>, 181> testData181() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180},
    }};
}

static constexpr auto table181 = make_perfect_hash(testData181());

constexpr bool testCompileTime181() {
    constexpr auto data = testData181();
    for (size_t j = 0; j < 181; ++j) {
        if (table181.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime181(),
              "Perfect hash compile-time test failed for 181 keys.");

constexpr std::array<std::pair<std::string_view, int>, 182> testData182() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181},
    }};
}

static constexpr auto table182 = make_perfect_hash(testData182());

constexpr bool testCompileTime182() {
    constexpr auto data = testData182();
    for (size_t j = 0; j < 182; ++j) {
        if (table182.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime182(),
              "Perfect hash compile-time test failed for 182 keys.");

constexpr std::array<std::pair<std::string_view, int>, 183> testData183() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182},
    }};
}

static constexpr auto table183 = make_perfect_hash(testData183());

constexpr bool testCompileTime183() {
    constexpr auto data = testData183();
    for (size_t j = 0; j < 183; ++j) {
        if (table183.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime183(),
              "Perfect hash compile-time test failed for 183 keys.");

constexpr std::array<std::pair<std::string_view, int>, 184> testData184() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
    }};
}

static constexpr auto table184 = make_perfect_hash(testData184());

constexpr bool testCompileTime184() {
    constexpr auto data = testData184();
    for (size_t j = 0; j < 184; ++j) {
        if (table184.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime184(),
              "Perfect hash compile-time test failed for 184 keys.");

constexpr std::array<std::pair<std::string_view, int>, 185> testData185() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184},
    }};
}

static constexpr auto table185 = make_perfect_hash(testData185());

constexpr bool testCompileTime185() {
    constexpr auto data = testData185();
    for (size_t j = 0; j < 185; ++j) {
        if (table185.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime185(),
              "Perfect hash compile-time test failed for 185 keys.");

constexpr std::array<std::pair<std::string_view, int>, 186> testData186() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185},
    }};
}

static constexpr auto table186 = make_perfect_hash(testData186());

constexpr bool testCompileTime186() {
    constexpr auto data = testData186();
    for (size_t j = 0; j < 186; ++j) {
        if (table186.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime186(),
              "Perfect hash compile-time test failed for 186 keys.");

constexpr std::array<std::pair<std::string_view, int>, 187> testData187() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186},
    }};
}

static constexpr auto table187 = make_perfect_hash(testData187());

constexpr bool testCompileTime187() {
    constexpr auto data = testData187();
    for (size_t j = 0; j < 187; ++j) {
        if (table187.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime187(),
              "Perfect hash compile-time test failed for 187 keys.");

constexpr std::array<std::pair<std::string_view, int>, 188> testData188() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
    }};
}

static constexpr auto table188 = make_perfect_hash(testData188());

constexpr bool testCompileTime188() {
    constexpr auto data = testData188();
    for (size_t j = 0; j < 188; ++j) {
        if (table188.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime188(),
              "Perfect hash compile-time test failed for 188 keys.");

constexpr std::array<std::pair<std::string_view, int>, 189> testData189() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188},
    }};
}

static constexpr auto table189 = make_perfect_hash(testData189());

constexpr bool testCompileTime189() {
    constexpr auto data = testData189();
    for (size_t j = 0; j < 189; ++j) {
        if (table189.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime189(),
              "Perfect hash compile-time test failed for 189 keys.");

constexpr std::array<std::pair<std::string_view, int>, 190> testData190() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189},
    }};
}

static constexpr auto table190 = make_perfect_hash(testData190());

constexpr bool testCompileTime190() {
    constexpr auto data = testData190();
    for (size_t j = 0; j < 190; ++j) {
        if (table190.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime190(),
              "Perfect hash compile-time test failed for 190 keys.");

constexpr std::array<std::pair<std::string_view, int>, 191> testData191() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190},
    }};
}

static constexpr auto table191 = make_perfect_hash(testData191());

constexpr bool testCompileTime191() {
    constexpr auto data = testData191();
    for (size_t j = 0; j < 191; ++j) {
        if (table191.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime191(),
              "Perfect hash compile-time test failed for 191 keys.");

constexpr std::array<std::pair<std::string_view, int>, 192> testData192() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
    }};
}

static constexpr auto table192 = make_perfect_hash(testData192());

constexpr bool testCompileTime192() {
    constexpr auto data = testData192();
    for (size_t j = 0; j < 192; ++j) {
        if (table192.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime192(),
              "Perfect hash compile-time test failed for 192 keys.");

constexpr std::array<std::pair<std::string_view, int>, 193> testData193() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192},
    }};
}

static constexpr auto table193 = make_perfect_hash(testData193());

constexpr bool testCompileTime193() {
    constexpr auto data = testData193();
    for (size_t j = 0; j < 193; ++j) {
        if (table193.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime193(),
              "Perfect hash compile-time test failed for 193 keys.");

constexpr std::array<std::pair<std::string_view, int>, 194> testData194() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193},
    }};
}

static constexpr auto table194 = make_perfect_hash(testData194());

constexpr bool testCompileTime194() {
    constexpr auto data = testData194();
    for (size_t j = 0; j < 194; ++j) {
        if (table194.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime194(),
              "Perfect hash compile-time test failed for 194 keys.");

constexpr std::array<std::pair<std::string_view, int>, 195> testData195() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194},
    }};
}

static constexpr auto table195 = make_perfect_hash(testData195());

constexpr bool testCompileTime195() {
    constexpr auto data = testData195();
    for (size_t j = 0; j < 195; ++j) {
        if (table195.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime195(),
              "Perfect hash compile-time test failed for 195 keys.");

constexpr std::array<std::pair<std::string_view, int>, 196> testData196() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
    }};
}

static constexpr auto table196 = make_perfect_hash(testData196());

constexpr bool testCompileTime196() {
    constexpr auto data = testData196();
    for (size_t j = 0; j < 196; ++j) {
        if (table196.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime196(),
              "Perfect hash compile-time test failed for 196 keys.");

constexpr std::array<std::pair<std::string_view, int>, 197> testData197() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196},
    }};
}

static constexpr auto table197 = make_perfect_hash(testData197());

constexpr bool testCompileTime197() {
    constexpr auto data = testData197();
    for (size_t j = 0; j < 197; ++j) {
        if (table197.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime197(),
              "Perfect hash compile-time test failed for 197 keys.");

constexpr std::array<std::pair<std::string_view, int>, 198> testData198() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197},
    }};
}

static constexpr auto table198 = make_perfect_hash(testData198());

constexpr bool testCompileTime198() {
    constexpr auto data = testData198();
    for (size_t j = 0; j < 198; ++j) {
        if (table198.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime198(),
              "Perfect hash compile-time test failed for 198 keys.");

constexpr std::array<std::pair<std::string_view, int>, 199> testData199() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198},
    }};
}

static constexpr auto table199 = make_perfect_hash(testData199());

constexpr bool testCompileTime199() {
    constexpr auto data = testData199();
    for (size_t j = 0; j < 199; ++j) {
        if (table199.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime199(),
              "Perfect hash compile-time test failed for 199 keys.");

constexpr std::array<std::pair<std::string_view, int>, 200> testData200() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
    }};
}

static constexpr auto table200 = make_perfect_hash(testData200());

constexpr bool testCompileTime200() {
    constexpr auto data = testData200();
    for (size_t j = 0; j < 200; ++j) {
        if (table200.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime200(),
              "Perfect hash compile-time test failed for 200 keys.");

constexpr std::array<std::pair<std::string_view, int>, 201> testData201() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200},
    }};
}

static constexpr auto table201 = make_perfect_hash(testData201());

constexpr bool testCompileTime201() {
    constexpr auto data = testData201();
    for (size_t j = 0; j < 201; ++j) {
        if (table201.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime201(),
              "Perfect hash compile-time test failed for 201 keys.");

constexpr std::array<std::pair<std::string_view, int>, 202> testData202() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201},
    }};
}

static constexpr auto table202 = make_perfect_hash(testData202());

constexpr bool testCompileTime202() {
    constexpr auto data = testData202();
    for (size_t j = 0; j < 202; ++j) {
        if (table202.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime202(),
              "Perfect hash compile-time test failed for 202 keys.");

constexpr std::array<std::pair<std::string_view, int>, 203> testData203() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202},
    }};
}

static constexpr auto table203 = make_perfect_hash(testData203());

constexpr bool testCompileTime203() {
    constexpr auto data = testData203();
    for (size_t j = 0; j < 203; ++j) {
        if (table203.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime203(),
              "Perfect hash compile-time test failed for 203 keys.");

constexpr std::array<std::pair<std::string_view, int>, 204> testData204() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
    }};
}

static constexpr auto table204 = make_perfect_hash(testData204());

constexpr bool testCompileTime204() {
    constexpr auto data = testData204();
    for (size_t j = 0; j < 204; ++j) {
        if (table204.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime204(),
              "Perfect hash compile-time test failed for 204 keys.");

constexpr std::array<std::pair<std::string_view, int>, 205> testData205() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204},
    }};
}

static constexpr auto table205 = make_perfect_hash(testData205());

constexpr bool testCompileTime205() {
    constexpr auto data = testData205();
    for (size_t j = 0; j < 205; ++j) {
        if (table205.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime205(),
              "Perfect hash compile-time test failed for 205 keys.");

constexpr std::array<std::pair<std::string_view, int>, 206> testData206() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205},
    }};
}

static constexpr auto table206 = make_perfect_hash(testData206());

constexpr bool testCompileTime206() {
    constexpr auto data = testData206();
    for (size_t j = 0; j < 206; ++j) {
        if (table206.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime206(),
              "Perfect hash compile-time test failed for 206 keys.");

constexpr std::array<std::pair<std::string_view, int>, 207> testData207() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206},
    }};
}

static constexpr auto table207 = make_perfect_hash(testData207());

constexpr bool testCompileTime207() {
    constexpr auto data = testData207();
    for (size_t j = 0; j < 207; ++j) {
        if (table207.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime207(),
              "Perfect hash compile-time test failed for 207 keys.");

constexpr std::array<std::pair<std::string_view, int>, 208> testData208() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
    }};
}

static constexpr auto table208 = make_perfect_hash(testData208());

constexpr bool testCompileTime208() {
    constexpr auto data = testData208();
    for (size_t j = 0; j < 208; ++j) {
        if (table208.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime208(),
              "Perfect hash compile-time test failed for 208 keys.");

constexpr std::array<std::pair<std::string_view, int>, 209> testData209() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208},
    }};
}

static constexpr auto table209 = make_perfect_hash(testData209());

constexpr bool testCompileTime209() {
    constexpr auto data = testData209();
    for (size_t j = 0; j < 209; ++j) {
        if (table209.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime209(),
              "Perfect hash compile-time test failed for 209 keys.");

constexpr std::array<std::pair<std::string_view, int>, 210> testData210() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209},
    }};
}

static constexpr auto table210 = make_perfect_hash(testData210());

constexpr bool testCompileTime210() {
    constexpr auto data = testData210();
    for (size_t j = 0; j < 210; ++j) {
        if (table210.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime210(),
              "Perfect hash compile-time test failed for 210 keys.");

constexpr std::array<std::pair<std::string_view, int>, 211> testData211() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210},
    }};
}

static constexpr auto table211 = make_perfect_hash(testData211());

constexpr bool testCompileTime211() {
    constexpr auto data = testData211();
    for (size_t j = 0; j < 211; ++j) {
        if (table211.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime211(),
              "Perfect hash compile-time test failed for 211 keys.");

constexpr std::array<std::pair<std::string_view, int>, 212> testData212() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
    }};
}

static constexpr auto table212 = make_perfect_hash(testData212());

constexpr bool testCompileTime212() {
    constexpr auto data = testData212();
    for (size_t j = 0; j < 212; ++j) {
        if (table212.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime212(),
              "Perfect hash compile-time test failed for 212 keys.");

constexpr std::array<std::pair<std::string_view, int>, 213> testData213() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212},
    }};
}

static constexpr auto table213 = make_perfect_hash(testData213());

constexpr bool testCompileTime213() {
    constexpr auto data = testData213();
    for (size_t j = 0; j < 213; ++j) {
        if (table213.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime213(),
              "Perfect hash compile-time test failed for 213 keys.");

constexpr std::array<std::pair<std::string_view, int>, 214> testData214() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213},
    }};
}

static constexpr auto table214 = make_perfect_hash(testData214());

constexpr bool testCompileTime214() {
    constexpr auto data = testData214();
    for (size_t j = 0; j < 214; ++j) {
        if (table214.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime214(),
              "Perfect hash compile-time test failed for 214 keys.");

constexpr std::array<std::pair<std::string_view, int>, 215> testData215() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214},
    }};
}

static constexpr auto table215 = make_perfect_hash(testData215());

constexpr bool testCompileTime215() {
    constexpr auto data = testData215();
    for (size_t j = 0; j < 215; ++j) {
        if (table215.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime215(),
              "Perfect hash compile-time test failed for 215 keys.");

constexpr std::array<std::pair<std::string_view, int>, 216> testData216() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
    }};
}

static constexpr auto table216 = make_perfect_hash(testData216());

constexpr bool testCompileTime216() {
    constexpr auto data = testData216();
    for (size_t j = 0; j < 216; ++j) {
        if (table216.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime216(),
              "Perfect hash compile-time test failed for 216 keys.");

constexpr std::array<std::pair<std::string_view, int>, 217> testData217() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216},
    }};
}

static constexpr auto table217 = make_perfect_hash(testData217());

constexpr bool testCompileTime217() {
    constexpr auto data = testData217();
    for (size_t j = 0; j < 217; ++j) {
        if (table217.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime217(),
              "Perfect hash compile-time test failed for 217 keys.");

constexpr std::array<std::pair<std::string_view, int>, 218> testData218() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217},
    }};
}

static constexpr auto table218 = make_perfect_hash(testData218());

constexpr bool testCompileTime218() {
    constexpr auto data = testData218();
    for (size_t j = 0; j < 218; ++j) {
        if (table218.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime218(),
              "Perfect hash compile-time test failed for 218 keys.");

constexpr std::array<std::pair<std::string_view, int>, 219> testData219() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218},
    }};
}

static constexpr auto table219 = make_perfect_hash(testData219());

constexpr bool testCompileTime219() {
    constexpr auto data = testData219();
    for (size_t j = 0; j < 219; ++j) {
        if (table219.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime219(),
              "Perfect hash compile-time test failed for 219 keys.");

constexpr std::array<std::pair<std::string_view, int>, 220> testData220() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
    }};
}

static constexpr auto table220 = make_perfect_hash(testData220());

constexpr bool testCompileTime220() {
    constexpr auto data = testData220();
    for (size_t j = 0; j < 220; ++j) {
        if (table220.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime220(),
              "Perfect hash compile-time test failed for 220 keys.");

constexpr std::array<std::pair<std::string_view, int>, 221> testData221() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220},
    }};
}

static constexpr auto table221 = make_perfect_hash(testData221());

constexpr bool testCompileTime221() {
    constexpr auto data = testData221();
    for (size_t j = 0; j < 221; ++j) {
        if (table221.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime221(),
              "Perfect hash compile-time test failed for 221 keys.");

constexpr std::array<std::pair<std::string_view, int>, 222> testData222() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221},
    }};
}

static constexpr auto table222 = make_perfect_hash(testData222());

constexpr bool testCompileTime222() {
    constexpr auto data = testData222();
    for (size_t j = 0; j < 222; ++j) {
        if (table222.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime222(),
              "Perfect hash compile-time test failed for 222 keys.");

constexpr std::array<std::pair<std::string_view, int>, 223> testData223() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222},
    }};
}

static constexpr auto table223 = make_perfect_hash(testData223());

constexpr bool testCompileTime223() {
    constexpr auto data = testData223();
    for (size_t j = 0; j < 223; ++j) {
        if (table223.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime223(),
              "Perfect hash compile-time test failed for 223 keys.");

constexpr std::array<std::pair<std::string_view, int>, 224> testData224() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
    }};
}

static constexpr auto table224 = make_perfect_hash(testData224());

constexpr bool testCompileTime224() {
    constexpr auto data = testData224();
    for (size_t j = 0; j < 224; ++j) {
        if (table224.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime224(),
              "Perfect hash compile-time test failed for 224 keys.");

constexpr std::array<std::pair<std::string_view, int>, 225> testData225() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224},
    }};
}

static constexpr auto table225 = make_perfect_hash(testData225());

constexpr bool testCompileTime225() {
    constexpr auto data = testData225();
    for (size_t j = 0; j < 225; ++j) {
        if (table225.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime225(),
              "Perfect hash compile-time test failed for 225 keys.");

constexpr std::array<std::pair<std::string_view, int>, 226> testData226() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225},
    }};
}

static constexpr auto table226 = make_perfect_hash(testData226());

constexpr bool testCompileTime226() {
    constexpr auto data = testData226();
    for (size_t j = 0; j < 226; ++j) {
        if (table226.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime226(),
              "Perfect hash compile-time test failed for 226 keys.");

constexpr std::array<std::pair<std::string_view, int>, 227> testData227() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226},
    }};
}

static constexpr auto table227 = make_perfect_hash(testData227());

constexpr bool testCompileTime227() {
    constexpr auto data = testData227();
    for (size_t j = 0; j < 227; ++j) {
        if (table227.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime227(),
              "Perfect hash compile-time test failed for 227 keys.");

constexpr std::array<std::pair<std::string_view, int>, 228> testData228() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
    }};
}

static constexpr auto table228 = make_perfect_hash(testData228());

constexpr bool testCompileTime228() {
    constexpr auto data = testData228();
    for (size_t j = 0; j < 228; ++j) {
        if (table228.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime228(),
              "Perfect hash compile-time test failed for 228 keys.");

constexpr std::array<std::pair<std::string_view, int>, 229> testData229() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228},
    }};
}

static constexpr auto table229 = make_perfect_hash(testData229());

constexpr bool testCompileTime229() {
    constexpr auto data = testData229();
    for (size_t j = 0; j < 229; ++j) {
        if (table229.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime229(),
              "Perfect hash compile-time test failed for 229 keys.");

constexpr std::array<std::pair<std::string_view, int>, 230> testData230() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229},
    }};
}

static constexpr auto table230 = make_perfect_hash(testData230());

constexpr bool testCompileTime230() {
    constexpr auto data = testData230();
    for (size_t j = 0; j < 230; ++j) {
        if (table230.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime230(),
              "Perfect hash compile-time test failed for 230 keys.");

constexpr std::array<std::pair<std::string_view, int>, 231> testData231() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230},
    }};
}

static constexpr auto table231 = make_perfect_hash(testData231());

constexpr bool testCompileTime231() {
    constexpr auto data = testData231();
    for (size_t j = 0; j < 231; ++j) {
        if (table231.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime231(),
              "Perfect hash compile-time test failed for 231 keys.");

constexpr std::array<std::pair<std::string_view, int>, 232> testData232() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
    }};
}

static constexpr auto table232 = make_perfect_hash(testData232());

constexpr bool testCompileTime232() {
    constexpr auto data = testData232();
    for (size_t j = 0; j < 232; ++j) {
        if (table232.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime232(),
              "Perfect hash compile-time test failed for 232 keys.");

constexpr std::array<std::pair<std::string_view, int>, 233> testData233() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232},
    }};
}

static constexpr auto table233 = make_perfect_hash(testData233());

constexpr bool testCompileTime233() {
    constexpr auto data = testData233();
    for (size_t j = 0; j < 233; ++j) {
        if (table233.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime233(),
              "Perfect hash compile-time test failed for 233 keys.");

constexpr std::array<std::pair<std::string_view, int>, 234> testData234() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233},
    }};
}

static constexpr auto table234 = make_perfect_hash(testData234());

constexpr bool testCompileTime234() {
    constexpr auto data = testData234();
    for (size_t j = 0; j < 234; ++j) {
        if (table234.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime234(),
              "Perfect hash compile-time test failed for 234 keys.");

constexpr std::array<std::pair<std::string_view, int>, 235> testData235() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234},
    }};
}

static constexpr auto table235 = make_perfect_hash(testData235());

constexpr bool testCompileTime235() {
    constexpr auto data = testData235();
    for (size_t j = 0; j < 235; ++j) {
        if (table235.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime235(),
              "Perfect hash compile-time test failed for 235 keys.");

constexpr std::array<std::pair<std::string_view, int>, 236> testData236() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
    }};
}

static constexpr auto table236 = make_perfect_hash(testData236());

constexpr bool testCompileTime236() {
    constexpr auto data = testData236();
    for (size_t j = 0; j < 236; ++j) {
        if (table236.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime236(),
              "Perfect hash compile-time test failed for 236 keys.");

constexpr std::array<std::pair<std::string_view, int>, 237> testData237() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236},
    }};
}

static constexpr auto table237 = make_perfect_hash(testData237());

constexpr bool testCompileTime237() {
    constexpr auto data = testData237();
    for (size_t j = 0; j < 237; ++j) {
        if (table237.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime237(),
              "Perfect hash compile-time test failed for 237 keys.");

constexpr std::array<std::pair<std::string_view, int>, 238> testData238() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237},
    }};
}

static constexpr auto table238 = make_perfect_hash(testData238());

constexpr bool testCompileTime238() {
    constexpr auto data = testData238();
    for (size_t j = 0; j < 238; ++j) {
        if (table238.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime238(),
              "Perfect hash compile-time test failed for 238 keys.");

constexpr std::array<std::pair<std::string_view, int>, 239> testData239() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238},
    }};
}

static constexpr auto table239 = make_perfect_hash(testData239());

constexpr bool testCompileTime239() {
    constexpr auto data = testData239();
    for (size_t j = 0; j < 239; ++j) {
        if (table239.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime239(),
              "Perfect hash compile-time test failed for 239 keys.");

constexpr std::array<std::pair<std::string_view, int>, 240> testData240() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
    }};
}

static constexpr auto table240 = make_perfect_hash(testData240());

constexpr bool testCompileTime240() {
    constexpr auto data = testData240();
    for (size_t j = 0; j < 240; ++j) {
        if (table240.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime240(),
              "Perfect hash compile-time test failed for 240 keys.");

constexpr std::array<std::pair<std::string_view, int>, 241> testData241() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240},
    }};
}

static constexpr auto table241 = make_perfect_hash(testData241());

constexpr bool testCompileTime241() {
    constexpr auto data = testData241();
    for (size_t j = 0; j < 241; ++j) {
        if (table241.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime241(),
              "Perfect hash compile-time test failed for 241 keys.");

constexpr std::array<std::pair<std::string_view, int>, 242> testData242() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241},
    }};
}

static constexpr auto table242 = make_perfect_hash(testData242());

constexpr bool testCompileTime242() {
    constexpr auto data = testData242();
    for (size_t j = 0; j < 242; ++j) {
        if (table242.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime242(),
              "Perfect hash compile-time test failed for 242 keys.");

constexpr std::array<std::pair<std::string_view, int>, 243> testData243() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242},
    }};
}

static constexpr auto table243 = make_perfect_hash(testData243());

constexpr bool testCompileTime243() {
    constexpr auto data = testData243();
    for (size_t j = 0; j < 243; ++j) {
        if (table243.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime243(),
              "Perfect hash compile-time test failed for 243 keys.");

constexpr std::array<std::pair<std::string_view, int>, 244> testData244() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
    }};
}

static constexpr auto table244 = make_perfect_hash(testData244());

constexpr bool testCompileTime244() {
    constexpr auto data = testData244();
    for (size_t j = 0; j < 244; ++j) {
        if (table244.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime244(),
              "Perfect hash compile-time test failed for 244 keys.");

constexpr std::array<std::pair<std::string_view, int>, 245> testData245() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244},
    }};
}

static constexpr auto table245 = make_perfect_hash(testData245());

constexpr bool testCompileTime245() {
    constexpr auto data = testData245();
    for (size_t j = 0; j < 245; ++j) {
        if (table245.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime245(),
              "Perfect hash compile-time test failed for 245 keys.");

constexpr std::array<std::pair<std::string_view, int>, 246> testData246() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245},
    }};
}

static constexpr auto table246 = make_perfect_hash(testData246());

constexpr bool testCompileTime246() {
    constexpr auto data = testData246();
    for (size_t j = 0; j < 246; ++j) {
        if (table246.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime246(),
              "Perfect hash compile-time test failed for 246 keys.");

constexpr std::array<std::pair<std::string_view, int>, 247> testData247() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246},
    }};
}

static constexpr auto table247 = make_perfect_hash(testData247());

constexpr bool testCompileTime247() {
    constexpr auto data = testData247();
    for (size_t j = 0; j < 247; ++j) {
        if (table247.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime247(),
              "Perfect hash compile-time test failed for 247 keys.");

constexpr std::array<std::pair<std::string_view, int>, 248> testData248() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
    }};
}

static constexpr auto table248 = make_perfect_hash(testData248());

constexpr bool testCompileTime248() {
    constexpr auto data = testData248();
    for (size_t j = 0; j < 248; ++j) {
        if (table248.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime248(),
              "Perfect hash compile-time test failed for 248 keys.");

constexpr std::array<std::pair<std::string_view, int>, 249> testData249() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248},
    }};
}

static constexpr auto table249 = make_perfect_hash(testData249());

constexpr bool testCompileTime249() {
    constexpr auto data = testData249();
    for (size_t j = 0; j < 249; ++j) {
        if (table249.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime249(),
              "Perfect hash compile-time test failed for 249 keys.");

constexpr std::array<std::pair<std::string_view, int>, 250> testData250() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248}, {"key249", 249},
    }};
}

static constexpr auto table250 = make_perfect_hash(testData250());

constexpr bool testCompileTime250() {
    constexpr auto data = testData250();
    for (size_t j = 0; j < 250; ++j) {
        if (table250.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime250(),
              "Perfect hash compile-time test failed for 250 keys.");

constexpr std::array<std::pair<std::string_view, int>, 251> testData251() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248}, {"key249", 249}, {"key250", 250},
    }};
}

static constexpr auto table251 = make_perfect_hash(testData251());

constexpr bool testCompileTime251() {
    constexpr auto data = testData251();
    for (size_t j = 0; j < 251; ++j) {
        if (table251.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime251(),
              "Perfect hash compile-time test failed for 251 keys.");

constexpr std::array<std::pair<std::string_view, int>, 252> testData252() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248}, {"key249", 249}, {"key250", 250}, {"key251", 251},
    }};
}

static constexpr auto table252 = make_perfect_hash(testData252());

constexpr bool testCompileTime252() {
    constexpr auto data = testData252();
    for (size_t j = 0; j < 252; ++j) {
        if (table252.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime252(),
              "Perfect hash compile-time test failed for 252 keys.");

constexpr std::array<std::pair<std::string_view, int>, 253> testData253() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248}, {"key249", 249}, {"key250", 250}, {"key251", 251},
        {"key252", 252},
    }};
}

static constexpr auto table253 = make_perfect_hash(testData253());

constexpr bool testCompileTime253() {
    constexpr auto data = testData253();
    for (size_t j = 0; j < 253; ++j) {
        if (table253.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime253(),
              "Perfect hash compile-time test failed for 253 keys.");

constexpr std::array<std::pair<std::string_view, int>, 254> testData254() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248}, {"key249", 249}, {"key250", 250}, {"key251", 251},
        {"key252", 252}, {"key253", 253},
    }};
}

static constexpr auto table254 = make_perfect_hash(testData254());

constexpr bool testCompileTime254() {
    constexpr auto data = testData254();
    for (size_t j = 0; j < 254; ++j) {
        if (table254.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime254(),
              "Perfect hash compile-time test failed for 254 keys.");

constexpr std::array<std::pair<std::string_view, int>, 255> testData255() {
    return {{
        {"key000", 0},   {"key001", 1},   {"key002", 2},   {"key003", 3},
        {"key004", 4},   {"key005", 5},   {"key006", 6},   {"key007", 7},
        {"key008", 8},   {"key009", 9},   {"key010", 10},  {"key011", 11},
        {"key012", 12},  {"key013", 13},  {"key014", 14},  {"key015", 15},
        {"key016", 16},  {"key017", 17},  {"key018", 18},  {"key019", 19},
        {"key020", 20},  {"key021", 21},  {"key022", 22},  {"key023", 23},
        {"key024", 24},  {"key025", 25},  {"key026", 26},  {"key027", 27},
        {"key028", 28},  {"key029", 29},  {"key030", 30},  {"key031", 31},
        {"key032", 32},  {"key033", 33},  {"key034", 34},  {"key035", 35},
        {"key036", 36},  {"key037", 37},  {"key038", 38},  {"key039", 39},
        {"key040", 40},  {"key041", 41},  {"key042", 42},  {"key043", 43},
        {"key044", 44},  {"key045", 45},  {"key046", 46},  {"key047", 47},
        {"key048", 48},  {"key049", 49},  {"key050", 50},  {"key051", 51},
        {"key052", 52},  {"key053", 53},  {"key054", 54},  {"key055", 55},
        {"key056", 56},  {"key057", 57},  {"key058", 58},  {"key059", 59},
        {"key060", 60},  {"key061", 61},  {"key062", 62},  {"key063", 63},
        {"key064", 64},  {"key065", 65},  {"key066", 66},  {"key067", 67},
        {"key068", 68},  {"key069", 69},  {"key070", 70},  {"key071", 71},
        {"key072", 72},  {"key073", 73},  {"key074", 74},  {"key075", 75},
        {"key076", 76},  {"key077", 77},  {"key078", 78},  {"key079", 79},
        {"key080", 80},  {"key081", 81},  {"key082", 82},  {"key083", 83},
        {"key084", 84},  {"key085", 85},  {"key086", 86},  {"key087", 87},
        {"key088", 88},  {"key089", 89},  {"key090", 90},  {"key091", 91},
        {"key092", 92},  {"key093", 93},  {"key094", 94},  {"key095", 95},
        {"key096", 96},  {"key097", 97},  {"key098", 98},  {"key099", 99},
        {"key100", 100}, {"key101", 101}, {"key102", 102}, {"key103", 103},
        {"key104", 104}, {"key105", 105}, {"key106", 106}, {"key107", 107},
        {"key108", 108}, {"key109", 109}, {"key110", 110}, {"key111", 111},
        {"key112", 112}, {"key113", 113}, {"key114", 114}, {"key115", 115},
        {"key116", 116}, {"key117", 117}, {"key118", 118}, {"key119", 119},
        {"key120", 120}, {"key121", 121}, {"key122", 122}, {"key123", 123},
        {"key124", 124}, {"key125", 125}, {"key126", 126}, {"key127", 127},
        {"key128", 128}, {"key129", 129}, {"key130", 130}, {"key131", 131},
        {"key132", 132}, {"key133", 133}, {"key134", 134}, {"key135", 135},
        {"key136", 136}, {"key137", 137}, {"key138", 138}, {"key139", 139},
        {"key140", 140}, {"key141", 141}, {"key142", 142}, {"key143", 143},
        {"key144", 144}, {"key145", 145}, {"key146", 146}, {"key147", 147},
        {"key148", 148}, {"key149", 149}, {"key150", 150}, {"key151", 151},
        {"key152", 152}, {"key153", 153}, {"key154", 154}, {"key155", 155},
        {"key156", 156}, {"key157", 157}, {"key158", 158}, {"key159", 159},
        {"key160", 160}, {"key161", 161}, {"key162", 162}, {"key163", 163},
        {"key164", 164}, {"key165", 165}, {"key166", 166}, {"key167", 167},
        {"key168", 168}, {"key169", 169}, {"key170", 170}, {"key171", 171},
        {"key172", 172}, {"key173", 173}, {"key174", 174}, {"key175", 175},
        {"key176", 176}, {"key177", 177}, {"key178", 178}, {"key179", 179},
        {"key180", 180}, {"key181", 181}, {"key182", 182}, {"key183", 183},
        {"key184", 184}, {"key185", 185}, {"key186", 186}, {"key187", 187},
        {"key188", 188}, {"key189", 189}, {"key190", 190}, {"key191", 191},
        {"key192", 192}, {"key193", 193}, {"key194", 194}, {"key195", 195},
        {"key196", 196}, {"key197", 197}, {"key198", 198}, {"key199", 199},
        {"key200", 200}, {"key201", 201}, {"key202", 202}, {"key203", 203},
        {"key204", 204}, {"key205", 205}, {"key206", 206}, {"key207", 207},
        {"key208", 208}, {"key209", 209}, {"key210", 210}, {"key211", 211},
        {"key212", 212}, {"key213", 213}, {"key214", 214}, {"key215", 215},
        {"key216", 216}, {"key217", 217}, {"key218", 218}, {"key219", 219},
        {"key220", 220}, {"key221", 221}, {"key222", 222}, {"key223", 223},
        {"key224", 224}, {"key225", 225}, {"key226", 226}, {"key227", 227},
        {"key228", 228}, {"key229", 229}, {"key230", 230}, {"key231", 231},
        {"key232", 232}, {"key233", 233}, {"key234", 234}, {"key235", 235},
        {"key236", 236}, {"key237", 237}, {"key238", 238}, {"key239", 239},
        {"key240", 240}, {"key241", 241}, {"key242", 242}, {"key243", 243},
        {"key244", 244}, {"key245", 245}, {"key246", 246}, {"key247", 247},
        {"key248", 248}, {"key249", 249}, {"key250", 250}, {"key251", 251},
        {"key252", 252}, {"key253", 253}, {"key254", 254},
    }};
}

static constexpr auto table255 = make_perfect_hash(testData255());

constexpr bool testCompileTime255() {
    constexpr auto data = testData255();
    for (size_t j = 0; j < 255; ++j) {
        if (table255.get(data[j].first) != data[j].second)
            return false;
    }
    return true;
}

static_assert(testCompileTime255(),
              "Perfect hash compile-time test failed for 255 keys.");
