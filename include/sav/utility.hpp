
#ifndef SAVVY_SAV_UTILITY_HPP
#define SAVVY_SAV_UTILITY_HPP

#include "savvy/region.hpp"

#include <string>
#include <vector>
#include <set>

savvy::region string_to_region(const std::string& s);
std::vector<std::string> split_string_to_vector(const char* in, char delim);
std::set<std::string> split_string_to_set(const char* in, char delim);
std::set<std::string> split_file_to_set(const char* in);

#endif //SAVVY_SAV_UTILITY_HPP