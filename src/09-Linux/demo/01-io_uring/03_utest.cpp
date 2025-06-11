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
    awa.reset();
    awa.emplace<std::string>("Woc Nb!");

    constexpr auto idx2
        = UninitializedNonVoidVariantIndexVal<Awa, UninitializedNonVoidVariant<int, Awa>>;

    print::println("idx2 = ", idx2);

    HX::print::println("get: ", get<2>(awa));

    get<std::string>(awa) = "awa";

    HX::print::println("get: ", get<2>(awa));
    return 0;
}