#include <HXTest.hpp>
#include <vector>

template <typename T = void, typename = std::enable_if_t<std::is_same_v<T, void>>>
class Message; // 前向声明

template <typename T = void>
class User {
public:
    std::string name;
    std::vector<Message<>> messages; // 收到的消息列表

    User(const std::string& n) 
        : name(n)
    {}

    std::string getName() const {
        return name;
    }

    void receiveMessage(Message<T> msg) {
        messages.push_back(std::move(msg));
    }
};

template <typename T, typename>
class Message {
public:
    User<> sender;   // 发送者
    User<> receiver; // 接收者
    std::string content;

    void print() {
        log::hxLog.info(
            "From:", sender.getName(),
            "To:", receiver.getName(),
            "Say:", content
        );
    }
};

HX_NO_WARNINGS_BEGIN
int main() {
    Message msg {
        {"张三"},
        {"李四"},
        "Hello"
    };
    msg.print();

    User uesr{"老六"};
    return 0;
}
HX_NO_WARNINGS_END