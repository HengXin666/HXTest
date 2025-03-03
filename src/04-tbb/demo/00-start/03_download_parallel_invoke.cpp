#include <iostream>
#include <string>
#include <tbb/parallel_invoke.h>

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
    // tbb::parallel_invoke([]{
    //     download("loli.zip");
    // }, []{
    //     interact();
    // });

    std::string str{"Hello Loli~"};
    char c = 'L';
    tbb::parallel_invoke([&]{
        for (uint32_t i = 0; i < str.size() / 2; ++i)
            if (c == str[i])
                std::cout << "find!" << '\n';
    }, [&]{
        for (uint32_t i = str.size() / 2; i < str.size(); ++i)
            if (c == str[i])
                std::cout << "find!" << '\n';
    });
    return 0;
}
