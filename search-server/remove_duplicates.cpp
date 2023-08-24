// в качестве заготовки кода используйте последнюю версию своей поисковой системы
#include "remove_duplicates.h"

#include <set>

void RemoveDuplicates(SearchServer& search_server) {

    std::set<int> duplicates_;

    std::set<std::vector<std::string>> words;

    for( const int id : search_server ) {
        auto words_server = search_server.GetWordFrequencies(id);
        
        std::vector<std::string> data_;
        for( const auto &[word, rel] : words_server ) {
            data_.push_back(word);
        }

        if( words.count(data_) ) {
            duplicates_.insert(id);
            continue;
        }

        words.insert(data_);
    }

    for( const int id : duplicates_ ) {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}
  