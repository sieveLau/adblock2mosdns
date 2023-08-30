#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_set>
#include "downloader.hpp"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostringstream;

int main(int argc, char** argv) {
    std::filesystem::path myself(argv[0]);
    std::filesystem::path parent = myself.parent_path();
    std::filesystem::path path_link_file = parent / "links.txt";
    std::filesystem::path path_keys_file = parent / "keys_to_remove.txt";
    if (!std::filesystem::exists(path_link_file)) {
        std::cerr << "links.txt not found!" << endl;
        std::cerr << "Please create one at " << path_link_file << endl;
        exit(2);
    }
    if (!std::filesystem::exists(path_keys_file)) {
        std::cerr << "keys_to_remove.txt not found!" << endl;
        std::cerr << "Please create one at " << path_keys_file << endl;
        exit(2);
    }

    std::unordered_set<std::string> set;
    std::unordered_set<std::string> keys_to_remove;

    auto str = download(path_link_file);

    std::ifstream in(path_keys_file);
    if (in.is_open()) {
        std::string a_key;
        while (std::getline(in, a_key)) {
            keys_to_remove.insert(a_key);
        }
        in.close();
    }

    add_host_to_set(str, set);
    for (const auto& key : keys_to_remove) {
        set.erase(key);
    }

    for (const auto& i : set) {
        cout<< i <<endl;
    }
    std::clog << "number of hosts: " << set.size() << endl;
    return 0;
}