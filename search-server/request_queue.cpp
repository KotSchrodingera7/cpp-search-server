//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎

#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    // напишите реализацию
    auto data_ = server_.FindTopDocuments(std::execution::seq, raw_query, status);
    AddRequest(data_, raw_query);
    return data_;
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    // напишите реализацию
    auto data_ = server_.FindTopDocuments(raw_query);
    AddRequest(data_, raw_query);
    return data_;
}
int RequestQueue::GetNoResultRequests() const {
    // напишите реализацию
    return no_result_count;
}
void RequestQueue::AddRequest(const std::vector<Document> &data, const std::string &query) {
    ++current_time;
    QueryResult result{0};

    if( data.empty() ) {
        result.no_result = 1;
        ++no_result_count;
    }
    requests_.push_back(result);

    if( current_time > min_in_day_ ) {
        if( requests_.front().no_result ) {
            --no_result_count;
            requests_.pop_front();
        }
    }

    requests_.push_back(result);
}