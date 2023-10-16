//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎
#include "search_server.h"
#include <cmath>
#include <numeric>
#include <execution>
using namespace std::literals;


void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status, 
                                const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }

    storage.push_back(std::string{document});
    const auto words = SplitIntoWordsNoStop(storage.back());

    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_id_freq_word[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}   

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq,
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}


const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static std::map<std::string_view, double> result;
    if( document_id_freq_word.count(document_id) ) {
        result = document_id_freq_word.at(document_id);
    }
    return result;
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}


std::set<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}
std::set<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view raw_query,
                                                    int document_id) const {
    
    // if ((document_id < 0) || (documents_.count(document_id) == 0)) {
    //     throw std::out_of_range("Invalid document_id"s);
    // }
    return MatchDocument(std::execution::seq, raw_query, document_id);
   
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, 
                                                    const std::string_view raw_query, 
                                                    int document_id) const {
    if ((document_id < 0) || (documents_.count(document_id) == 0)) {
        throw std::out_of_range("Invalid document_id"s);
    }
    
    const auto query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return {std::vector<std::string_view>{}, documents_.at(document_id).status};
        }
    }
    
    for (const auto word: query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    return {matched_words, documents_.at(document_id).status};
}
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&,
                                                    const std::string_view raw_query,
                                                    int document_id) const {
    
     if ((document_id < 0) || (documents_.count(document_id) == 0)) {
        throw std::out_of_range("Invalid document_id"s);
    }
    
    const auto query = ParseQuery(raw_query, false);

    

    bool status_ = std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(), [&](const auto word){
                                        if (word_to_document_freqs_.count(word) == 0) {
                                            return false;
                                        }
                                        if (word_to_document_freqs_.at(word).count(document_id)) {
                                            return true;
                                        }

                                        return false;
                                });

    if( status_ ) {
        return {std::vector<std::string_view>{}, documents_.at(document_id).status};
    }



    std::vector<std::string_view> matched_words(query.plus_words.size());

    auto it = std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
                                [&](const auto word) {
                                        if (word_to_document_freqs_.count(word) == 0) {
                                            return false;
                                        }
                                        if (word_to_document_freqs_.at(word).count(document_id)) {
                                            return true;
                                        }

                                        return false;
                                });


    matched_words.resize(std::distance(matched_words.begin(), it));

    std::sort(std::execution::par,matched_words.begin(),matched_words.end());
    auto last = std::unique(matched_words.begin(),matched_words.end());
    matched_words.erase(last,matched_words.end());

    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const std::string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const {
    std::vector<std::string_view> words;
    for (const std::string_view word : SplitIntoWordsView(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word.data() + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}
SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string_view word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + text.data() + " is invalid");
    }

    return {word, is_minus, IsStopWord(word)};
}

void SearchServer::DeleteDuplicate(std::vector<std::string> &data)
{
    std::sort(data.begin(), data.end());

    auto last = std::unique(data.begin(), data.end());

    data.erase(last, data.end());
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view text, bool policy) const {
    Query result;
    for (const std::string_view word : SplitIntoWordsView(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            } else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }
    
     if( policy ) {

        std::sort(result.minus_words.begin(), result.minus_words.end());
        auto last = std::unique(result.minus_words.begin(), result.minus_words.end());
        result.minus_words.resize(std::distance(result.minus_words.begin(), last));

        std::sort(result.plus_words.begin(), result.plus_words.end());
        last = std::unique(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.resize(std::distance(result.plus_words.begin(), last));
        
    }
    

    return result;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
