#pragma once
//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎


#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <execution>
#include <deque>
#include <future>

#include "document.h"
#include "string_processing.h"
#include "concurrent_map.h"


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const int NUMBER_OF_MAPS = 100;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
        if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            using namespace std::literals;
            throw std::invalid_argument("Some of stop words are invalid"s);
        }
    }

    explicit SearchServer(const std::string& stop_words_text)
        : SearchServer(
            SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
    {
    }

    explicit SearchServer(std::string_view &stop_words_text) : 
            SearchServer(SplitIntoWords(std::string(stop_words_text)))
    {

    }

    template <class Execution, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const Execution &policy, const std::string_view raw_query, DocumentPredicate document_predicate) const;

    template <class Execution>
    void RemoveDocument(const Execution &policy, int document_id);

    // void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    void AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings);
    void RemoveDocument(int document_id);

    template <class Execution>
    std::vector<Document> FindTopDocuments (const Execution &policy, const std::string_view raw_query) const;

    template <class Execution>
    std::vector<Document> FindTopDocuments(const Execution &policy ,const std::string_view raw_query, DocumentStatus status) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const;
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const;
    int GetDocumentCount() const;
    std::set<int>::iterator begin();
    std::set<int>::iterator end();

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, const std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id) const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
     struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };
    
    std::deque<std::string> storage;
    const std::set<std::string, std::less<>> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_id_freq_word;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    bool IsStopWord(const std::string_view word) const;

    static bool IsValidWord(const std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);
    QueryWord ParseQueryWord(const std::string_view text) const;
    Query ParseQuery(const std::string_view text, bool policy = true) const;
    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    void DeleteDuplicate(std::vector<std::string> &data);

    template <class Execution, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Execution &policy, const Query& query, DocumentPredicate document_predicate) const;
};

template <class Execution>
void SearchServer::RemoveDocument(const Execution &policy, int document_id) {

    if( document_ids_.count(document_id) ) {
        documents_.erase(document_id);
        document_ids_.erase(document_id);

        const auto& words = document_id_freq_word.at(document_id);
        
        std::vector<const std::string_view*> word_of_delete(words.size());
        std::transform(policy, words.begin(), words.end(), word_of_delete.begin(), [](const auto& item){return &item.first;});
        std::for_each(policy, word_of_delete.begin(), word_of_delete.end(), [=](auto word){
            this->word_to_document_freqs_.at(*word).erase(document_id);
        });
        
        document_id_freq_word.erase(document_id);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <class Execution>
std::vector<Document> SearchServer::FindTopDocuments (const Execution &policy, const std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <class Execution>
std::vector<Document> SearchServer::FindTopDocuments(const Execution &policy ,const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy,
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

template <class Execution, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments (const Execution &policy, const std::string_view raw_query,
                                    DocumentPredicate document_predicate) const {


    const auto query = ParseQuery(raw_query);//, std::is_same_v<std::decay_t<Execution>, std::execution::sequenced_policy>);

    auto matched_documents = FindAllDocuments(policy, query, document_predicate);

    std::sort(policy, matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (std::abs(lhs.relevance - rhs.relevance) < std::numeric_limits<double>::epsilon()) {
                    return lhs.rating > rhs.rating;
                } else {
                    return lhs.relevance > rhs.relevance;
                }
            });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <class Execution, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Execution &policy, const Query& query,
                                    DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance(NUMBER_OF_MAPS);
    

    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(), [&](const auto word){
         if (word_to_document_freqs_.count(word) == 0) {
            return;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
            }
        }
    });


    std::for_each(policy, query.minus_words.begin(), query.minus_words.end(), [&](const auto word){
        if (word_to_document_freqs_.count(word) == 0) {
            return;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }

    });

    auto document_to_relevance_map = document_to_relevance.BuildOrdinaryMap();
    std::vector<Document> matched_documents(document_to_relevance_map.size());

    std::transform(policy, document_to_relevance_map.begin(), document_to_relevance_map.end(), matched_documents.begin(), [&](const auto& item) {
                return Document(item.first, item.second, documents_.at(item.first).rating);
            });

    return matched_documents;
    //   std::map<int, double> document_to_relevance;
    // for (const std::string& word : query.plus_words) {
    //     if (word_to_document_freqs_.count(word) == 0) {
    //         continue;
    //     }
    //     const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
    //     for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
    //         const auto& document_data = documents_.at(document_id);
    //         if (document_predicate(document_id, document_data.status, document_data.rating)) {
    //             document_to_relevance[document_id] += term_freq * inverse_document_freq;
    //         }
    //     }
    // }

    // for (const std::string& word : query.minus_words) {
    //     if (word_to_document_freqs_.count(word) == 0) {
    //         continue;
    //     }
    //     for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
    //         document_to_relevance.erase(document_id);
    //     }
    // }

    // std::vector<Document> matched_documents;
    // for (const auto [document_id, relevance] : document_to_relevance) {
    //     matched_documents.push_back(
    //         {document_id, relevance, documents_.at(document_id).rating});
    // }
    // return matched_documents;

    // std::vector<Document> matched_documents(document_to_relevance.size());

    // std::transform(policy, document_to_relevance.begin(), document_to_relevance.end(), matched_documents.begin(), [&](const auto& item) {
    //             return Document(item.first, item.second, documents_.at(item.first).rating);
    //         });
}
