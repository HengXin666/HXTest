#include <iostream>
#include <string>
#include <tbb/task_group.h>

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
    tbb::task_group tg;
    tg.run([]{
        download("loli.zip");
    });
    tg.run([]{
        interact();
    });
    // interact();
    tg.wait();
    return 0;
}
