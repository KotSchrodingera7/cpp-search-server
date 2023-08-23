// в качестве заготовки кода используйте последнюю версию своей поисковой системы
#include "remove_duplicates.h"

#include <set>
void RemoveDuplicates(SearchServer& search_server) {

    std::set<int> duplicates_;

    for( auto it = search_server.begin(); it != search_server.end(); ++it ) {
        auto words_first = search_server.GetWordFrequencies(*it);

        for( auto next_it = std::next(it); next_it != search_server.end(); ++next_it ) {
            auto words_second = search_server.GetWordFrequencies(*next_it);

            if( words_first.size() != words_second.size() ) {
                continue;
            }

            auto it_word_second = words_second.begin();

            for( auto it_word_first = words_first.begin(); it_word_first != words_first.end(); ++it_word_first) {

                if( (*it_word_first).first != (*it_word_second).first ) {
                    break;
                }
                ++it_word_second;
            }

            if( it_word_second == words_second.end() ) {
                duplicates_.insert(*next_it);
            }
        }
    }

    for(auto const &id : duplicates_) {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}
  