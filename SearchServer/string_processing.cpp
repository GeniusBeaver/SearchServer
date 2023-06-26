#include "string_processing.h"
using namespace std;


std::vector<std::string_view> SplitIntoWords(std::string_view text) {
    std::vector<std::string_view> result;
    const int64_t pos_end = text.npos;
    text.remove_prefix(std::min(text.find_first_not_of(" "), text.size()));
    while (!text.empty()) {
        int64_t space = text.find(' ');

        if (space == pos_end) {
            result.push_back(text.substr(0, text.size()));
        }
        else {
           result.push_back(text.substr(0, space));
         }
       text.remove_prefix(std::min(text.size(), text.find_first_not_of(' ', space)));
    }
    return result;
}

/*std::vector<std::string_view> SplitIntoWords(const std::string& text) {
    std::vector<std::string_view> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                std::string_view temp_str(word);
                words.push_back(temp_str);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}*/
