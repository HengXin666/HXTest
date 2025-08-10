#include <HXTest.hpp>

#include <HXAv.hpp>
HX_AV_INFO(__HX_AV_INFO__)

#include <memory>
#include <vector>

class Message; // 前向声明

class User {
public:
    std::string name;
    std::vector<std::unique_ptr<Message>> messages; // 收到的消息列表

    User(const std::string& n) 
        : name(n)
    {}

    std::string getName() const {
        return name;
    }

    void receiveMessage(std::unique_ptr<Message> msg) {
        messages.push_back(std::move(msg));
    }
};

class Message {
public:
    std::unique_ptr<User> sender;   // 发送者
    std::unique_ptr<User> receiver; // 接收者
    std::string content;

    void print() {
        log::hxLog.info(
            "From:", sender->getName(),
            "To:", receiver->getName(),
            "Say:", content
        );
    }
};

HX_NO_WARNINGS_BEGIN
int main() {
    Message msg {
        {std::make_unique<User>("张三")},
        {std::make_unique<User>("李四")},
        "Hello"
    };
    msg.print();
    return 0;
}
HX_NO_WARNINGS_END