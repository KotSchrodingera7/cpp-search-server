#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;



// const int MAX_RESULT_DOCUMENT_COUNT = 5;

// string ReadLine() {
//     string s;
//     getline(cin, s);
//     return s;
// }

// int ReadLineWithNumber() {
//     int result;
//     cin >> result;
//     ReadLine();
//     return result;
// }

// vector<string> SplitIntoWords(const string& text) {
//     vector<string> words;
//     string word;
//     for (const char c : text) {
//         if (c == ' ') {
//             if (!word.empty()) {
//                 words.push_back(word);
//                 word.clear();
//             }
//         } else {
//             word += c;
//         }
//     }
//     if (!word.empty()) {
//         words.push_back(word);
//     }

//     return words;
// }

// struct Document {
//     int id;
//     double relevance;
//     int rating;
// };

// enum class DocumentStatus {
//     ACTUAL,
//     IRRELEVANT,
//     BANNED,
//     REMOVED,
// };



// class SearchServer {
// public:
//     void SetStopWords(const string& text) {
//         for (const string& word : SplitIntoWords(text)) {
//             stop_words_.insert(word);
//         }
//     }

//     void AddDocument(int document_id, const string& document, DocumentStatus status,
//                      const vector<int>& ratings) {
//         const vector<string> words = SplitIntoWordsNoStop(document);
//         const double inv_word_count = 1.0 / words.size();
//         for (const string& word : words) {
//             word_to_document_freqs_[word][document_id] += inv_word_count;
//         }
//         documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
//     }

//     vector<Document> FindTopDocuments(const string& raw_query) const {
//         return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
//     }
//     vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status_) const {
//         return FindTopDocuments(raw_query, [&status_](int document_id, DocumentStatus status, int rating) { return status == status_; });
//     }
// template<typename Foo>
//     vector<Document> FindTopDocuments(const string& raw_query, Foo foo) const {
//         const Query query = ParseQuery(raw_query);
//         auto matched_documents = FindAllDocuments(query, foo);

//         sort(matched_documents.begin(), matched_documents.end(),
//              [](const Document& lhs, const Document& rhs) {
//                  if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
//                      return lhs.rating > rhs.rating;
//                  } else {
//                      return lhs.relevance > rhs.relevance;
//                  }
//              });
//         if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
//             matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
//         }
//         return matched_documents;
//     }

//     int GetDocumentCount() const {
//         return documents_.size();
//     }

//     tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
//                                                         int document_id) const {
//         const Query query = ParseQuery(raw_query);
//         vector<string> matched_words;
//         for (const string& word : query.plus_words) {
//             if (word_to_document_freqs_.count(word) == 0) {
//                 continue;
//             }
//             if (word_to_document_freqs_.at(word).count(document_id)) {
//                 matched_words.push_back(word);
//             }
//         }
//         for (const string& word : query.minus_words) {
//             if (word_to_document_freqs_.count(word) == 0) {
//                 continue;
//             }
//             if (word_to_document_freqs_.at(word).count(document_id)) {
//                 matched_words.clear();
//                 break;
//             }
//         }
//         return {matched_words, documents_.at(document_id).status};
//     }

// private:
//     struct DocumentData {
//         int rating;
//         DocumentStatus status;
//     };

//     set<string> stop_words_;
//     map<string, map<int, double>> word_to_document_freqs_;
//     map<int, DocumentData> documents_;

//     bool IsStopWord(const string& word) const {
//         return stop_words_.count(word) > 0;
//     }

//     vector<string> SplitIntoWordsNoStop(const string& text) const {
//         vector<string> words;
//         for (const string& word : SplitIntoWords(text)) {
//             if (!IsStopWord(word)) {
//                 words.push_back(word);
//             }
//         }
//         return words;
//     }

//     static int ComputeAverageRating(const vector<int>& ratings) {
//         if (ratings.empty()) {
//             return 0;
//         }
//         int rating_sum = 0;
//         for (const int rating : ratings) {
//             rating_sum += rating;
//         }
//         return rating_sum / static_cast<int>(ratings.size());
//     }

//     struct QueryWord {
//         string data;
//         bool is_minus;
//         bool is_stop;
//     };

//     QueryWord ParseQueryWord(string text) const {
//         bool is_minus = false;
//         // Word shouldn't be empty
//         if (text[0] == '-') {
//             is_minus = true;
//             text = text.substr(1);
//         }
//         return {text, is_minus, IsStopWord(text)};
//     }

//     struct Query {
//         set<string> plus_words;
//         set<string> minus_words;
//     };

//     Query ParseQuery(const string& text) const {
//         Query query;
//         for (const string& word : SplitIntoWords(text)) {
//             const QueryWord query_word = ParseQueryWord(word);
//             if (!query_word.is_stop) {
//                 if (query_word.is_minus) {
//                     query.minus_words.insert(query_word.data);
//                 } else {
//                     query.plus_words.insert(query_word.data);
//                 }
//             }
//         }
//         return query;
//     }

//     // Existence required
//     double ComputeWordInverseDocumentFreq(const string& word) const {
//         return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
//     }
// template<typename Foo>
//     vector<Document> FindAllDocuments(const Query& query, Foo foo) const {
//         map<int, double> document_to_relevance;
//         for (const string& word : query.plus_words) {
//             if (word_to_document_freqs_.count(word) == 0) {
//                 continue;
//             }
//             const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
//             for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
//                 auto &data = documents_.at(document_id);
//                 if ( foo(document_id, data.status, data.rating) ) {
//                     document_to_relevance[document_id] += term_freq * inverse_document_freq;
//                 }
//             }
//         }

//         for (const string& word : query.minus_words) {
//             if (word_to_document_freqs_.count(word) == 0) {
//                 continue;
//             }
//             for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
//                 document_to_relevance.erase(document_id);
//             }
//         }

//         vector<Document> matched_documents;
//         for (const auto [document_id, relevance] : document_to_relevance) {
//             matched_documents.push_back(
//                 {document_id, relevance, documents_.at(document_id).rating});
//         }
//         return matched_documents;
//     }
// };

// ==================== для примера =========================



ostream& operator<<(ostream& os, const DocumentStatus &r) {
    if( r == DocumentStatus::ACTUAL )
    {
        os << "ACTUAL";
    } else if( r == DocumentStatus::IRRELEVANT )
    {
        os << "IRRELEVANT";
    } else if( r == DocumentStatus::BANNED )
    {
        os << "BANNED";
    } else if( r == DocumentStatus::REMOVED )
    {
        os << "REMOVED";
    }
    
    return os;
}


// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.size());
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
        // assert(doc0.id == doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}
/*
Разместите код остальных тестов здесь
*/


void TestExcludeMinusWordsFromAddedDocumentContent()
{
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

        const auto [match_doc, status]  = server.MatchDocument("cat dog people", 42);
        ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
        ASSERT(match_doc.size());
        ASSERT_EQUAL(match_doc[0], "cat");
        // assert(status == DocumentStatus::ACTUAL);
        // assert(match_doc.size() == 1);
        // assert(match_doc[0] == "cat");

        const auto [match_doc_minus, status_minus]  = server.MatchDocument("-cat dog people", 42);
        ASSERT(match_doc_minus.empty());
        // assert(match_doc_minus.empty());
    }
}

void TestMatchedWithMinusWord()
{
    const int doc_id = 42;
    const string content = "-cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("cat"s).empty());
        // assert(server.FindTopDocuments("cat"s).empty());
    }
}

void TestRelevanceTopDocument()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3, -4, 4, 8};

    const int doc_id2 = 43;
    const string content2 = "cat dog people"s;
    const vector<int> ratings2 = {1, 2, -3, 12, 10};

    const int doc_id3 = 44;
    const string content3 = "cat yellow bug"s;
    const vector<int> ratings3 = {1, -3, 5, 6, 9};
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);

        const auto found_docs = server.FindTopDocuments("cat the home"s);

        ASSERT(found_docs.begin()->relevance > found_docs.end()->relevance);
        // assert(found_docs.begin()->relevance > found_docs.end()->relevance);
    }
}

void TestRatingDocuments()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat dog template"s;
    const vector<int> ratings2 = {-1, 2, 0, 3, 10};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.begin()->rating, 2);
        // assert(found_docs.begin()->rating == 2);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.begin()->rating, 2);
        // assert(found_docs.begin()->rating == 2);
    }
}

void TestPredicate()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat dog template"s;
    const vector<int> ratings2 = {-1, 2, 0, 3, 10};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);

        const auto found_docs = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
        ASSERT_EQUAL(found_docs.size(), 1);
        ASSERT_EQUAL(found_docs.begin()->id, 42);
        // assert(found_docs.size() == 1);
        // assert(found_docs.begin()->id == 42);
    }
}

void TestStatusDocument()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat dog template"s;
    const vector<int> ratings2 = {-1, 2, 0, 3, 10};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, ratings2);

        {
            const auto found_docs = server.FindTopDocuments("cat"s, DocumentStatus::ACTUAL);
            ASSERT_EQUAL(found_docs.size(), 1);
            ASSERT_EQUAL(found_docs.begin()->id, 42);
            // assert(found_docs.size() == 1);
            // assert(found_docs.begin()->id == 42);
        }


        {
            const auto found_docs = server.FindTopDocuments("cat"s, DocumentStatus::BANNED);
            ASSERT_EQUAL(found_docs.size(), 1);
            ASSERT_EQUAL(found_docs.begin()->id, 43);
            // assert(found_docs.size() == 1);
            // assert(found_docs.begin()->id == 43);
        }
    }
}

void TestRelevance()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat dog template"s;
    const vector<int> ratings2 = {-1, 2, 0, 3, 10};

    const int doc_id3 = 44;
    const string content3 = "dog template home"s;
    const vector<int> ratings3 = {-1, 2, 10};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("cat"s);
        double relevance1 = static_cast<double>(log(3.0/2.0)) * (1.0/4.0);
        double relevance2 = static_cast<double>(log(3.0/2.0)) * (1.0/3.0);
        double relevance3 = static_cast<double>(log(3.0/2.0)) * (0.0/3.0);

        // assert(found_docs.size() == 2);
        ASSERT_EQUAL(found_docs.size(), 2);
        // assert(found_docs[0].relevance == relevance2);
        ASSERT(found_docs[0].relevance == relevance2);
        // assert(found_docs[1].relevance == relevance1);
        ASSERT(found_docs[1].relevance == relevance1);
    }
}
// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestMatchedWithMinusWord();
    TestExcludeMinusWordsFromAddedDocumentContent();
    TestReleanceTopDocument();
    TestRatingDdpcuments();
    TestPredicat();
    TestStatusDocument();
    TestRelevance();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

// int main() {
//     TestSearchServer();
//     // Если вы видите эту строку, значит все тесты прошли успешно
//     cout << "Search server testing finished"s << endl;
// }


// void PrintDocument(const Document& document) {
//     cout << "{ "s
//          << "document_id = "s << document.id << ", "s
//          << "relevance = "s << document.relevance << ", "s
//          << "rating = "s << document.rating << " }"s << endl;
// }

// int main() {
//     SearchServer search_server;
//     search_server.SetStopWords("и в на"s);

//     search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
//     search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
//     search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
//     search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
//     cout << "ACTUAL by default:"s << endl;
//     for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
//         PrintDocument(document);
//     }
//     cout << "ACTUAL:"s << endl;
//     for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; })) {
//         PrintDocument(document);
//     }
//     cout << "Even ids:"s << endl;
//     for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
//         PrintDocument(document);
//     }
//     return 0;
// }