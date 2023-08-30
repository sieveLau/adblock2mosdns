#include "downloader.hpp"

using curl::curl_easy;
using curl::curl_easy_exception;
using curl::curl_ios;

void add_host_to_set(const std::string& src, std::unordered_set<std::string>& dest) {
    std::istringstream in(src);
    std::string line;
    const std::regex reg_hosts("\\S* (\\S*)");
    std::cmatch match;
    while ((std::getline(in, line))) {
        auto line_cstr = line.c_str();
        if (std::regex_match(line_cstr, match, reg_hosts)) {
            dest.insert(match[1]);
        }
    }
}


std::string download(const std::filesystem::path& links_file){
    std::ostringstream str;
    curl_ios<std::ostringstream> writer(str);
    std::ifstream in(links_file);
    if(in.is_open()){
        std::string a_url;
        while (std::getline(in, a_url)) {
            curl_easy easy(writer);
            easy.add<CURLOPT_URL>(a_url.c_str());
            easy.add<CURLOPT_FOLLOWLOCATION>(1L);
            try {
                easy.perform();
            } catch (curl_easy_exception& exception) {
                std::cerr << exception.what() << std::endl;
                exception.print_traceback();
            }
            easy.reset();
        }
        in.close();
    }
    return str.str();
}