#pragma once
//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎


#include <vector>
#include <string>
#include <deque>
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : server_(search_server) {
        // напишите реализацию
    }
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;

private:
    void AddRequest(const std::vector<Document> &data, const std::string &query);
    struct QueryResult {
        int no_result;
        // определите, что должно быть в структуре
    };
    
    std::deque<QueryResult> requests_{};
    const static int min_in_day_ = 1440;
    uint64_t current_time{0};
    int no_result_count{0};
    const SearchServer& server_;
    // возможно, здесь вам понадобится что-то ещё
}; 

 template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    // напишите реализацию
    auto data_ = server_.FindTopDocuments(raw_query, document_predicate);
    AddRequest(data_, raw_query);
    return data_;
}