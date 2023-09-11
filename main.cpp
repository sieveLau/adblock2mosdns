#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_set>
#include "cxxopts.hpp"
#include <thread>

#include "downloader.hpp"
#include "nlohmann/json.hpp"

using std::cout;
using std::endl;
using std::cerr;
using std::clog;
using std::ofstream;
using std::ostringstream;
using json = nlohmann::json;

using std::filesystem::exists;
using std::filesystem::create_directories;

void parse(const std::filesystem::path& input_file, std::unordered_set<std::string>& domains, std::unordered_set<std::string>& hosts, std::unordered_set<std::string>& excepts) {
    std::ifstream in(input_file);
    std::string line;
    const std::regex reg_hosts(R"(^[0-9a-zA-Z\.\:]{1,}(?:\t| ){1,}(\S*))");
    const std::regex reg_excepts(R"(^@{2}\|{2}([0-9a-zA-Z\-_\.]*)(?:\^?|\/)\S*)");
    const std::regex reg_domains(R"(^\|{2}([0-9a-zA-Z\-_\.]*)\^$)");
    std::cmatch match;
    while ((std::getline(in, line))) {
        auto line_cstr = line.c_str();
        if (std::regex_match(line_cstr, match, reg_hosts)) {
            hosts.insert(match[1]);
        } else
        if (std::regex_match(line_cstr, match, reg_domains)) {
            domains.insert(match[1]);
        } else
        if (std::regex_match(line_cstr, match, reg_excepts)) {
            excepts.insert(match[1]);
        }
    }
}

void write_to_file(std::filesystem::path output, const std::unordered_set<std::string>* data, const std::regex* keys_to_remove, const size_t size, std::string prefix){
    ofstream out(output);
    for (auto && rec : *data){
        for (int i = 0;i<size;++i){
            if (std::regex_match(rec.c_str(),keys_to_remove[i])) {
                goto skipwrite;
            }
        }
        out<<prefix<<rec<<'\n';
    skipwrite:
        ;
    }
    out.close();
}

int main(int argc, char** argv) {

    cxxopts::Options options("adblock2mosdns", "Translate adblock and hosts to mosdns format.");
    options.add_options()
        ("skip-download", "Skip download and use cache")
        ("h,help", "Print usage")
        ;
    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    std::filesystem::path myself(argv[0]);
    std::filesystem::path parent = myself.parent_path();
    std::filesystem::path path_link_file = parent / "links.txt";
    std::filesystem::path path_keys_file = parent / "keys_to_remove.txt";
    std::filesystem::path cache_dir = parent / "cache";
    std::filesystem::path output_file = parent / "output.txt";
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
    if (!exists(cache_dir)){
        cerr<<"Cache directory not exists, exiting."<<endl;
        exit(2);
    }
//    return 0;


    std::unordered_set<std::string> hosts;
    std::unordered_set<std::string> domains;
    std::unordered_set<std::string> keys_to_remove;


    std::ifstream in;
    if(!result.count("skip-download")) {
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
        std::clog<< "Download skipped." << endl;
    }

//    auto str = download(path_link_file);

    for (auto const& dir_entry : std::filesystem::directory_iterator{cache_dir}) {
        parse(dir_entry.path(),domains,hosts,keys_to_remove);
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

    char host_buffer[1024]{'\0'};
    const char* host_patter = R"(([0-9a-zA-Z\-_\.]*\.)?%s)";
    ofstream output(output_file);

    auto *to_remove_regex_list=new std::regex[keys_to_remove.size()];
    for (auto&& to_remove : keys_to_remove){
        host_buffer[0]='\0';
        snprintf(host_buffer,1024,host_patter,to_remove.c_str());
        to_remove_regex_list[count++] = std::string(host_buffer);
    }

    // todo: split into more threads
    // todo: google re2
    // todo: escape domain chars
    // todo: combine regex into one
    std::thread th1(write_to_file, parent / "ad_domains.txt",&domains,to_remove_regex_list,count,"domain:");
    std::thread th2(write_to_file, parent / "ad_hosts.txt",&hosts,to_remove_regex_list,count,"full:");



    th1.join();
    th2.join();


    return 0;
}