#include <iostream>
#include <map>
#include <optional>
#include <chrono>

using namespace std::chrono;

auto makeTime = [](std::chrono::system_clock::duration d) {
    return std::chrono::system_clock::now() + d;
};

namespace print {

template <typename... Ts>
void println(Ts&&... ts) {
    ((std::cout << ts), ...);
    std::cout << '\n';
}

} // namespace print

struct MD {
    using MdTree = std::multimap<std::chrono::system_clock::time_point, int>;
    MD& operator=(MD&&) = delete;
    
    MdTree::iterator addTimer(
        std::chrono::system_clock::time_point expireTime,
        int v
    ) {
        print::println("insert: ", v);
        return _mdTree.insert({expireTime, v});
    }

    struct A {
        A(MD* md)
            : _md{md}
            , _expireTime{}
            , _it{}
        {
            print::println("make this: ", this);
        }
    
        A(A const&) = delete;
        A& operator=(A const&) noexcept = delete;

#if 0
        A(A&&) = default;
        A& operator=(A&&) noexcept = default;
#else
        A(A&& that)
            : _md{that._md}
            , _expireTime{std::move(that._expireTime)}
            , _it{std::move(that._it)}
        {
            // 如果缺少下面的, 就会抛异常
            // print::println("that: ", this, " [A&&] ref: ", _md);
            print::println("this: ", this, " [A&&] ref: ", _md);
        }

        A& operator=(A&& that) noexcept {
            _md = that._md;
            _expireTime = std::move(that._expireTime);
            _it = std::move(that._it);
            // print::println("that: ", this, " [operator=&&] ref: ", _timerLoop);
            print::println("this: ", this, " [operator=&&] ref: ", _md);
            return *this;
        }
#endif
    
        void await_suspend() const noexcept {
            print::println("this: ", this);
            _it = _md->addTimer(_expireTime, 1);
        }
    
        void await_resume() const noexcept {
            print::println("this: ", this);
            _it.reset();
        }
    
        A&& setExpireTime(std::chrono::system_clock::time_point expireTime) && noexcept {
            _expireTime = expireTime;
            return std::move(*this);
        }
    
        ~A() noexcept {
            print::println("~this: ", this);
            if (_it) {
                print::println("the ", this, " del!");
                _md->_mdTree.erase(*_it);
            }
        }
    
    private:
        MD* _md;
        std::chrono::system_clock::time_point _expireTime;
        mutable std::optional<MdTree::iterator> _it;
    };
private:  
    struct BuildA {
        BuildA(MD* md) 
            : _md{md}
        {}
    
        BuildA& operator=(BuildA&&) noexcept = delete;
    
        A&& build(std::chrono::system_clock::duration t) && {
            return std::move(A{_md}.setExpireTime(makeTime(t)));
        }

        MD* _md;
    };
public:
    static BuildA makeBuild(MD& md) {
        return BuildA{&md};
    }

private:
    MdTree _mdTree;
};


static void linkTimeout(MD::A timeout) { // 如果修改为 A&&, insert后, 会卡死在STL的一个while中
    timeout.await_suspend();
    timeout.await_resume();
}

void test02() {
    MD fk;
    linkTimeout(MD::makeBuild(fk).build(1s));
}

void test01() {
    // std::optional<It> it;
    // it = mp.insert({makeTime(1s), 1});
    // auto jt = std::move(it);
    // auto [k, v] = **jt;
    // (void)k;
    // print::println("v: ", v);
}

int main() {
    test02();
    return 0;
}