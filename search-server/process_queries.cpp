

#include <execution>
#include "process_queries.h"
//using namespace std::literals;
using namespace std;


std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    
    std::vector<std::vector<Document>> result(queries.size());
    transform(execution::par, queries.begin(), 
                    queries.end(), result.begin(), 
                    [&search_server] (const std::string &querie) {
                        return search_server.FindTopDocuments(querie);
                    });

    return result;
}


std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    
    // std::vector<std::vector<Document>> result = ProcessQueries(search_server, queries);
    std::vector<Document> result_return;

    std::vector<std::vector<Document>> result(queries.size());
    transform(execution::par, queries.begin(), 
                    queries.end(), result.begin(), 
                    [&search_server, &result_return] (const std::string &querie) {
                        auto find_ = search_server.FindTopDocuments(querie);
                        // result_return.insert(result_return.begin(), find_.begin(), find_.end());
                        return find_;
                    });

    for(const auto &data : result) {
        for( const auto &doc : data) {
            result_return.push_back(std::move(doc));
        }
    }


    return result_return;
}
