#include <iostream>
#include <string>
#include <thread>

void download(std::string file) {
    for (int i = 0; i < 10; ++i) {
        std::cout << "Downloading " << file
                  << " (" << i * 10 << "%)..." << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    std::cout << "Download complete: " << file << '\n';
}

void interact() {
    std::string name;
    std::cin >> name;
    std::cout << "Hi, " << name << '\n';
}

int main() {
    std::thread t([&]{
        download("loli.zip");
    });
    interact();
    t.join();
    return 0;
}
