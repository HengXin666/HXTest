#include <HXprint/print.h>

#include <tools/UninitializedNonVoidVariant.hpp>

using namespace HX;

struct Awa {
    Awa() {
        // _ptr = new int[24];
        static int id = 0;
        _id = ++id;
    }

    // Awa(Awa const&) = default;
    // Awa(Awa&&) = default;

    // Awa& operator=(Awa const&) = default;
    // Awa& operator=(Awa&& that) {
    //     delete  _ptr;
    //     _ptr = that._ptr;
    //     that._ptr = nullptr;
    //     return *this;
    // }

    ~Awa() noexcept {
        HX::print::println("~A: ", _id);
        // delete _ptr;
        // _ptr = nullptr;
    }

    int _id;
    // int* _ptr;
};

#include <variant>
#include <vector>

static void stdTest() {
    try {
        std::variant<int, Awa, std::string> awa{Awa{}}, qwq;
        std::variant<Awa, std::string> orz; 
        awa.emplace<std::string>("qwq");
        print::println("idx = ", awa.index());
        // qwq = awa;
        qwq = "awa";
        // orz = qwq;
        // qwq = std::vector<int>{};
        print::println("get<int>: ", get<2>(awa));
        print::println("get<int>: ", get<int>(awa));
    } catch (...) {
        print::println("Error! STL");
    }
}

static void hxTest() {
    try {
        UninitializedNonVoidVariant<int, Awa, std::string> awa{}, qwq;
        Awa _{};
        awa = std::move(_);
        awa.emplace<std::string>("qwq");
        print::println("idx = ", awa.index());
        qwq = awa;

        // qwq = "awa";
        // qwq = std::vector<int>{};
        
        visit(awa, [](auto&& v) -> void {
            print::println("v: ", v);
            return {};
        });

        print::println("get<int>: ", get<2>(awa));
        print::println("get<int>: ", get<int>(awa));
    } catch (...) {
        print::println("Error! STL");
    }
}

#if 0

// 等价于 c++11 std::conditional

template <bool val, typename T, typename U>
struct EnableIfTT {
    using Type = T;
};

template <typename T, typename U>
struct EnableIfTT<false, T, U> {
    using Type = U;
};

#endif

int main() {
    stdTest();
    print::println("===");
    hxTest();

    return 0;

#if 0
    UninitializedNonVoidVariant<int, Awa, std::string> awa, qwq;
    awa.emplace<Awa>();
    awa.reset();
    awa.emplace<std::string>("Woc Nb!");

    constexpr auto idx2
        = UninitializedNonVoidVariantIndexVal<Awa, UninitializedNonVoidVariant<int, Awa>>;

    print::println("idx2 = ", idx2);

    HX::print::println("get: ", get<2>(awa));

    get<std::string>(awa) = "awa";

    HX::print::println("get: ", get<2>(awa));
    // qwq = std::move(awa);
    return 0;
#endif
}