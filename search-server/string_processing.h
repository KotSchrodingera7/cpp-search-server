#pragma once
//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎


#include <set>
#include <string>
#include <vector>

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

std::vector<std::string> SplitIntoWords(const std::string_view& text);
std::vector<std::string_view> SplitIntoWordsView(std::string_view str);