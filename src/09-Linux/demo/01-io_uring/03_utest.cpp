#include <HXprint/print.h>

#include <tools/UninitializedNonVoidVariant.hpp>

struct Awa {
    Awa() = default;

    ~Awa() noexcept {
        HX::print::println("~A");
    }
};

int main() {
    using namespace HX;
    UninitializedNonVoidVariant<int, Awa> awa;
    auto& res = get<0>(awa);
    res = 0x7F'FF'FF'FF;
    (void)awa;
    get<1>(awa) = {};
    // HX::print::println("get: ", (unsigned char)get<0>(awa));
    return 0;
}