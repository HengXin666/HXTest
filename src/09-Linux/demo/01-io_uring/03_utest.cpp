#include <HXprint/print.h>

#include <tools/UninitializedNonVoidVariant.hpp>

using namespace HX;

struct Awa {
    ~Awa() noexcept {
        HX::print::println("~A");
    }
};



int main() {
    UninitializedNonVoidVariant<int, Awa, std::string> awa;
    awa.emplace<Awa>();
    // auto& res = get<0>(awa);
    // res = 0x7F'FF'FF'FF;
    (void)awa;
    // get<1>(awa) = {};


    constexpr auto idx2
        = UninitializedNonVoidVariantIndexVal<Awa, UninitializedNonVoidVariant<int, Awa>>;

    print::println("idx2 = ", idx2);

    HX::print::println("get: ", get<1>(awa));
    return 0;
}