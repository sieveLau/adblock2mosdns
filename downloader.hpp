#pragma once
#include <string>
// #include <iostream>
#include <unordered_set>
#include <regex>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "curl_easy.h"
#include "curl_exception.h"
#include "curl_form.h"
#include "curl_ios.h"

void add_host_to_set(const std::string& src, std::unordered_set<std::string>& dest);

std::string download(const std::filesystem::path& links_file);
std::string download(const std::string& url);
bool download(const std::string& url, const std::filesystem::path& output_file);
