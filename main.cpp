#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <unordered_set>

#include "cxxopts.hpp"
#include "downloader.hpp"
#include "fmt/core.h"
#include "nlohmann/json.hpp"
#include "re2/re2.h"

using std::cerr;
using std::clog;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostringstream;
using json = nlohmann::json;

using std::filesystem::create_directories;
using std::filesystem::exists;
using std::filesystem::path;

void parse(const path& input_file, std::unordered_set<std::string>& domains, std::unordered_set<std::string>& hosts,
           std::unordered_set<std::string>& excepts) {
    std::ifstream in(input_file);
    std::string line;

    RE2 reg_hosts(R"(^[0-9a-zA-Z\.\:]{1,}(?:\t| ){1,}(\S*))");
    RE2 reg_excepts(R"(^@{2}\|{2}([0-9a-zA-Z\-_\.]*)(?:\^?|\/)\S*)");
    RE2 reg_domains(R"(^\|{2}([0-9a-zA-Z\-_\.]*)\^$)");
    std::string match;
    while ((std::getline(in, line))) {
        if (RE2::FullMatch(line, reg_hosts, &match)) {
            hosts.insert(match);
            continue;
        }
        if (RE2::FullMatch(line, reg_domains, &match)) {
            domains.insert(match);
            continue;
        }
        if (RE2::FullMatch(line, reg_excepts, &match)) {
            excepts.insert(match);
        }
    }
}

void write_to_file(path output, const std::unordered_set<std::string>* data, const std::vector<RE2*>* regex_list,
                   std::string prefix) {
    ofstream out(output);
    for (auto&& rec : *data) {
        for (const RE2* re : *regex_list) {
            if (RE2::FullMatch(rec, *re)) {
                goto skipwrite;
            }
        }
        out << prefix << rec << '\n';
    skipwrite:;
    }
    out.close();
}

int main(int argc, char** argv) {
    cxxopts::Options options("adblock2mosdns", "Translate adblock and hosts to mosdns format.");
    options.add_options()("skip-download", "Skip download and use cache")(
        "only-download", "Just download and skip parsing")("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    path myself(argv[0]);
    path parent = myself.parent_path();
    path path_link_file = parent / "links.txt";
    path path_keys_file = parent / "keys_to_remove.txt";
    path cache_dir = parent / "cache";
    path output_file = parent / "output.txt";
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
    create_directories(cache_dir);
    if (!exists(cache_dir)) {
        cerr << "Cache directory not exists, exiting." << endl;
        exit(2);
    }

    std::unordered_set<std::string> hosts;
    std::unordered_set<std::string> domains;
    std::unordered_set<std::string> keys_to_remove;

    std::ifstream in;
    if (!result.count("skip-download")) {
        in.open(path_link_file);
        if (in.is_open()) {
            std::string line;
            char file_name_buffer[14]{'\0'};
            const char* file_name_patter = "file-%d.txt";
            int count = 0;
            while (std::getline(in, line)) {
                if (line.starts_with('#')) continue;
                snprintf(file_name_buffer, 14, file_name_patter, count++);
                if (download(line, cache_dir / file_name_buffer)) {
                    cout << "Successfully downloaded: " << line << endl;
                } else {
                    cerr << "Failed to download: " << line << endl;
                }
                file_name_buffer[0] = '\0';
            }
        }
        in.close();
    } else {
        std::clog << "Download skipped." << endl;
    }
    if (result.count("download")) {
        return 0;
    }

    for (auto const& dir_entry : std::filesystem::directory_iterator{cache_dir}) {
        parse(dir_entry.path(), domains, hosts, keys_to_remove);
    }

    in.open(path_keys_file);
    if (in.is_open()) {
        std::string a_key;
        while (std::getline(in, a_key)) {
            keys_to_remove.insert(a_key);
        }
        in.close();
    }

    long count = 0;
    cout << domains.size() << endl;
    cout << hosts.size() << endl;
    cout << keys_to_remove.size() << endl;

    std::string host_pattern = R"(([0-9a-zA-Z\-_\.]*\.)?{})";
    std::vector<RE2*> regex_list;
    ofstream output(output_file);

    std::regex re(R"([-[\]{}()*+?.,\^$|#\s])");

    std::string src;

    RE2::Options option;
    option.set_never_capture(true);
    for (auto&& i : keys_to_remove) {
        src = std::regex_replace(i, re, R"(\$&)");
        regex_list.emplace_back(new RE2(fmt::vformat(host_pattern, fmt::make_format_args(src)), option));
    }
    clog << host_pattern << '\n';

    // todo: split into more threads
    std::thread th1(write_to_file, parent / "ad_domains.txt", &domains, &regex_list, "domain:");
    std::thread th2(write_to_file, parent / "ad_hosts.txt", &hosts, &regex_list, "full:");

    th1.join();
    th2.join();
    for (auto i = regex_list.begin(); i != regex_list.end(); ++i) {
        delete *i;
    }

    return 0;
}