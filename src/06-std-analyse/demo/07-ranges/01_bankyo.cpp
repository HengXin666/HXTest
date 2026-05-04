#include <ranges>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

#include <HXTest.hpp>

namespace ranges = std::ranges;
namespace views = std::views;

struct Order {
    int id;
    int userId;
    double price;
    bool valid;
    std::string tag;
};

struct NoRangesTest {
    std::vector<int> process(const std::vector<Order>& orders) {
        std::vector<int> result;

        // step1: 收集有效订单
        std::vector<Order> validOrders;
        for (size_t i = 0; i < orders.size(); ++i) {
            if (orders[i].valid == true) {
                validOrders.push_back(orders[i]);
            }
        }

        // step2: 过滤 price > 100 且 tag != "test"
        std::vector<Order> filtered;
        for (size_t i = 0; i < validOrders.size(); ++i) {
            if (validOrders[i].price > 100.0) {
                if (validOrders[i].tag != "test") {
                    filtered.push_back(validOrders[i]);
                }
            }
        }

        // step3: 按 price 排序(降序)
        std::sort(
            filtered.begin(), filtered.end(),
            [](const Order& a, const Order& b) { return a.price > b.price; });

        // step4: 去重 userId(保留第一次出现的)
        std::unordered_set<int> seen;
        std::vector<Order> dedup;
        for (size_t i = 0; i < filtered.size(); ++i) {
            if (seen.find(filtered[i].userId) == seen.end()) {
                seen.insert(filtered[i].userId);
                dedup.push_back(filtered[i]);
            }
        }

        // step5: 取前 10 个
        std::vector<Order> top;
        for (size_t i = 0; i < dedup.size(); ++i) {
            if (i < 10) {
                top.push_back(dedup[i]);
            }
        }

        // step6: 提取 userId
        for (size_t i = 0; i < top.size(); ++i) {
            result.push_back(top[i].userId);
        }

        return result;
    }
};

struct RangesTest {
    auto process(std::vector<Order> const& orders) {
        auto filtered = orders | views::filter([](Order const& o) {
            // step1: 收集有效订单
            return o.valid
            // step2: 过滤 price > 100 且 tag != "test"
            && o.price > 100.0
            && o.tag != "test";
        });
        std::vector<Order> arr{filtered.begin(), filtered.end()};
        // step3: 按 price 排序(降序)
        ranges::sort(arr, std::greater<>{}, &Order::price);
        std::unordered_set<int> st;
        auto res = arr
        // step4: 去重 userId(保留第一次出现的)
            | views::filter([&](Order const& o) {
                return st.insert(o.userId).second;
            })
        // step5: 取前 10 个
            | views::take(10)
        // step6: 提取 userId
            | views::transform([](Order const& o) { return o.userId; });
        return res;
    }
};

int main() {
}