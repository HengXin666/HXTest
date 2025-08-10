#include <HXTest.hpp>
#include <vector>

// 全局 前向声明
#define GLOBAL_STATEMENT(__NAME__) \
template <typename _AT_GT = void, typename = std::enable_if_t<std::is_same_v<_AT_GT, void>>> \
__NAME__

// 前向声明后, 实现处声明
#define AT_GLOBAL \
template <typename _AT_GT, typename>

// 在类或者函数内定义时候的声明
#define AT_D(__TYPE__) __TYPE__<_AT_GT>

////////////////////////////////////////////

GLOBAL_STATEMENT(class Message); // 前向声明
GLOBAL_STATEMENT(class User);    // 前向声明

AT_GLOBAL
class User {
public:
    std::string name;
    std::vector<AT_D(Message)> messages; // 收到的消息列表

    User(const std::string& n) 
        : name(n)
    {}

    std::string getName() const {
        return name;
    }

    void receiveMessage(AT_D(Message) msg) {
        messages.push_back(std::move(msg));
    }
};

AT_GLOBAL
class Message {
public:
    AT_D(User) sender;   // 发送者
    AT_D(User) receiver; // 接收者
    std::string content;

    void print() {
        log::hxLog.info(
            "From:", sender.getName(),
            "To:", receiver.getName(),
            "Say:", content
        );
    }
};

////////////////////////////////////////////

GLOBAL_STATEMENT(struct A);
GLOBAL_STATEMENT(struct B);

AT_GLOBAL
struct A {
    explicit A(AT_D(B)& b)
        : _b(b)
    {}

    void fun() {
        _b.todo(*this); // 可以使用 _b !
    }

    AT_D(B)& _b;
};

AT_GLOBAL
struct B {
    void todo(AT_D(A)&) {
        log::hxLog.warning("This B");
    }
};

////////////////////////////////////////////

HX_NO_WARNINGS_BEGIN
int main() {
    Message msg {
        {"张三"},
        {"李四"},
        "Hello"
    };
    msg.print();

    User uesr{"老六"};

////////////////////////////////////////////
    B b{};
    A a{b};
    a.fun();
    return 0;
}
HX_NO_WARNINGS_END