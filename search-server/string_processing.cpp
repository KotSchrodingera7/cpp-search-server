//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎
#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string_view& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
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
}

std::vector<std::string_view> SplitIntoWordsView(std::string_view str) {
    std::vector<std::string_view> result;
    
    str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
    
    while (!str.empty()) {
        int64_t space = str.find(' ');
        result.push_back(str.substr(0, space));
        if( space == -1) {
            str.remove_prefix(str.size());
            continue;    
        }
        str.remove_prefix(space);
        str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
    }

    return result;
}