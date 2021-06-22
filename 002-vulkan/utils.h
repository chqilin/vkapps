#include <vector>
#include <iostream>
#include <fstream>

const std::string hr = "------------------------------------------------------------\n";

const std::string tab(int n) {
    std::string str;
    for (int i = 0; i < n; i++) {
        str += "  ";
    }
    return str;
}

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t size = (size_t)file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);

    file.close();

    return buffer;
}
