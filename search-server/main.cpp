// в качестве заготовки кода используйте последнюю версию своей поисковой системы
//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎

// #include "process_queries.h"
// #include "search_server.h"

// #include <execution>
// #include <iostream>
// #include <string>
// #include <vector>

// using namespace std;

// const std::vector<int> ratings1 = {1, 2, 3, 4, 5};
// const std::vector<int> ratings2 = {-1, -2, 30, -3, 44, 5};
// const std::vector<int> ratings3 = {12, -20, 80, 0, 8, 0, 0, 9, 67};

// void PrintMatchDocumentResultUTest(int document_id, const std::vector<string_view>& words,
//                                    DocumentStatus status) {
//     std::cout << "{ "
//               << "document_id = " << document_id << ", "
//               << "status = " << static_cast<int>(status) << ", "
//               << "words =";
//     for (string_view word : words) {
//         std::cout << ' ' << word;
//     }
//     std::cout << "}" << std::endl;
// }

// void PrintDocumentUTest(const Document& document) {
//     std::cout << "{ "
//               << "document_id = " << document.id << ", "
//               << "relevance = " << document.relevance << ", "
//               << "rating = " << document.rating << " }" << std::endl;
// }


// void Test() {
//     std::string stop_words = "и в на";
//     SearchServer search_server(stop_words);

//     search_server.AddDocument(0, "белый кот и модный ошейник", DocumentStatus::ACTUAL, ratings1);
//     search_server.AddDocument(1, "пушистый кот пушистый хвост", DocumentStatus::ACTUAL, ratings2);
//     search_server.AddDocument(2, "ухоженный пёс выразительные глаза", DocumentStatus::ACTUAL,
//                               ratings3);
//     search_server.AddDocument(3, "белый модный кот", DocumentStatus::IRRELEVANT, ratings1);
//     search_server.AddDocument(4, "пушистый кот пёс", DocumentStatus::IRRELEVANT, ratings2);
//     search_server.AddDocument(5, "ухоженный ошейник выразительные глаза",
//                               DocumentStatus::IRRELEVANT, ratings3);
//     search_server.AddDocument(6, "кот и ошейник", DocumentStatus::BANNED, ratings1);
//     search_server.AddDocument(7, "пёс и хвост", DocumentStatus::BANNED, ratings2);
//     search_server.AddDocument(8, "модный пёс пушистый хвост", DocumentStatus::BANNED, ratings3);
//     search_server.AddDocument(9, "кот пушистый ошейник", DocumentStatus::REMOVED, ratings1);
//     search_server.AddDocument(10, "ухоженный кот и пёс", DocumentStatus::REMOVED, ratings2);
//     search_server.AddDocument(11, "хвост и выразительные глаза", DocumentStatus::REMOVED, ratings3);

//     const std::string query = "пушистый ухоженный кот -ошейник";
//     const auto documents = search_server.FindTopDocuments(query);

//     std::cout << "Top documents for query:" << std::endl;
//     for (const Document& document : documents) {
//         PrintDocumentUTest(document);
//     }

//     std::cout << "Documents' statuses:" << std::endl;
//     const int document_count = search_server.GetDocumentCount();
//     for (int document_id = 0; document_id < document_count; ++document_id) {
//         const auto [words, status] = search_server.MatchDocument(query, document_id);
//         PrintMatchDocumentResultUTest(document_id, words, status);
//     }
// }

// int main() {
//     Test();
    

//     return 0;
// }

#include "search_server.h"
#include "log_duration.h"
#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;
string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}
vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}
string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}
vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}
template <typename ExecutionPolicy>
void Test(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}
#define TEST(policy) Test(#policy, search_server, queries, execution::policy)
int main() {
    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 1000, 10);
    const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }
    const auto queries = GenerateQueries(generator, dictionary, 100, 70);
    TEST(seq);
    TEST(par);
} 




// #include "process_queries.h"
// #include "search_server.h"
// #include <execution>
// #include <iostream>
// #include <string>
// #include <vector>
// using namespace std;
// void PrintDocument(const Document& document) {
//     cout << "{ "s
//          << "document_id = "s << document.id << ", "s
//          << "relevance = "s << document.relevance << ", "s
//          << "rating = "s << document.rating << " }"s << endl;
// }
// int main() {
//     SearchServer search_server("and with"s);
//     int id = 0;
//     for (
//         const string& text : {
//             "white cat and yellow hat"s,
//             "curly cat curly tail"s,
//             "nasty dog with big eyes"s,
//             "nasty pigeon john"s,
//         }
//     ) {
//         search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
//     }
//     cout << "ACTUAL by default:"s << endl;
//     // последовательная версия
//     for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
//         PrintDocument(document);
//     }
//     cout << "BANNED:"s << endl;
//     // последовательная версия
//     for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
//         PrintDocument(document);
//     }
//     cout << "Even ids:"s << endl;
//     // параллельная версия
//     for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
//         PrintDocument(document);
//     }
//     return 0;
// } 



// #include "search_server.h"

// #include <iostream>
// #include <string>
// #include <vector>

// using namespace std;

// int main() {
//     SearchServer search_server("and with"s);

//     int id = 0;
//     for (
//         const string& text : {
//             "funny pet and nasty rat"s,
//             "funny pet with curly hair"s,
//             "funny pet and not very nasty rat"s,
//             "pet with rat and rat and rat"s,
//             "nasty rat with curly hair"s,
//         }
//     ) {
//         search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
//     }

//     const string query = "curly and funny -not"s;

//     {
//         const auto [words, status] = search_server.MatchDocument(query, 1);
//         cout << words.size() << " words for document 1"s << endl;
//         // 1 words for document 1
//     }

//     {
//         const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
//         cout << words.size() << " words for document 2"s << endl;
//         // 2 words for document 2
//     }

//     {
//         const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
//         cout << words.size() << " words for document 3"s << endl;
//         // 0 words for document 3
//     }

//     return 0;
// }


// #include "search_server.h"

// #include <execution>
// #include <iostream>
// #include <random>
// #include <string>
// #include <vector>

// #include "log_duration.h"

// using namespace std;

// string GenerateWord(mt19937& generator, int max_length) {
//     const int length = uniform_int_distribution(1, max_length)(generator);
//     string word;
//     word.reserve(length);
//     for (int i = 0; i < length; ++i) {
//         word.push_back(uniform_int_distribution('a', 'z')(generator));
//     }
//     return word;
// }

// vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
//     vector<string> words;
//     words.reserve(word_count);
//     for (int i = 0; i < word_count; ++i) {
//         words.push_back(GenerateWord(generator, max_length));
//     }
//     sort(words.begin(), words.end());
//     words.erase(unique(words.begin(), words.end()), words.end());
//     return words;
// }

// string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
//     const int word_count = uniform_int_distribution(1, max_word_count)(generator);
//     string query;
//     for (int i = 0; i < word_count; ++i) {
//         if (!query.empty()) {
//             query.push_back(' ');
//         }
//         query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
//     }
//     return query;
// }

// vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
//     vector<string> queries;
//     queries.reserve(query_count);
//     for (int i = 0; i < query_count; ++i) {
//         queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
//     }
//     return queries;
// }

// template <typename ExecutionPolicy>
// void Test(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
//     LOG_DURATION(mark);
//     const int document_count = search_server.GetDocumentCount();
//     for (int id = 0; id < document_count; ++id) {
//         search_server.RemoveDocument(policy, id);
//     }
//     cout << search_server.GetDocumentCount() << endl;
// }

// #define TEST(mode) Test(#mode, search_server, execution::mode)

// int main() {
//     mt19937 generator;

//     const auto dictionary = GenerateDictionary(generator, 10'000, 25);
//     const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);

//     {
//         SearchServer search_server(dictionary[0]);
//         for (size_t i = 0; i < documents.size(); ++i) {
//             search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
//         }

//         TEST(seq);
//     }
//     {
//         SearchServer search_server(dictionary[0]);
//         for (size_t i = 0; i < documents.size(); ++i) {
//             search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
//         }

//         TEST(par);
//     }
// }






// #include "search_server.h"

// #include <iostream>
// #include <string>
// #include <vector>
// #include <execution>

// using namespace std;

// int main() {
//     SearchServer search_server("and with"s);

//     int id = 0;
//     for (
//         const string& text : {
//             "funny pet and nasty rat"s,
//             "funny pet with curly hair"s,
//             "funny pet and not very nasty rat"s,
//             "pet with rat and rat and rat"s,
//             "nasty rat with curly hair"s,
//         }
//     ) {
//         search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
//     }

//     const string query = "curly and funny"s;

//     auto report = [&search_server, &query] {
//         cout << search_server.GetDocumentCount() << " documents total, "s
//             << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << endl;
//     };

//     report();
//     // однопоточная версия
//     search_server.RemoveDocument(5);
//     report();
//     // однопоточная версия
//     search_server.RemoveDocument(execution::seq, 1);
//     report();
//     // многопоточная версия
//     search_server.RemoveDocument(execution::par, 2);
//     report();

//     return 0;
// }



















// #include "process_queries.h"
// #include "search_server.h"
// #include <iostream>
// #include <string>
// #include <vector>
// using namespace std;
// int main() {
//     SearchServer search_server("and with"s);
//     int id = 0;
//     for (
//         const string& text : {
//             "funny pet and nasty rat"s,
//             "funny pet with curly hair"s,
//             "funny pet and not very nasty rat"s,
//             "pet with rat and rat and rat"s,
//             "nasty rat with curly hair"s,
//         }
//     ) {
//         search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
//     }
//     const vector<string> queries = {
//         "nasty rat -not"s,
//         "not very funny nasty pet"s,
//         "curly hair"s
//     };
//     for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
//         cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
//     }

// `
//     // for (const auto &document : ProcessQueries(search_server, queries)) {

//     //     for (const Document& document : document) {
//     //         cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
//     //     }
//     // }
//     return 0;
// }










// #include "search_server.h"
// #include <iostream>
// #include <random>
// #include <string>
// #include <vector>
// #include "log_duration.h"
// #include "process_queries.h"
// using namespace std;
// string GenerateWord(mt19937& generator, int max_length) {
//     const int length = uniform_int_distribution(1, max_length)(generator);
//     string word;
//     word.reserve(length);
//     for (int i = 0; i < length; ++i) {
//         word.push_back(uniform_int_distribution('a', 'z')(generator));
//     }
//     return word;
// }
// vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
//     vector<string> words;
//     words.reserve(word_count);
//     for (int i = 0; i < word_count; ++i) {
//         words.push_back(GenerateWord(generator, max_length));
//     }
//     sort(words.begin(), words.end());
//     words.erase(unique(words.begin(), words.end()), words.end());
//     return words;
// }
// string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
//     const int word_count = uniform_int_distribution(1, max_word_count)(generator);
//     string query;
//     for (int i = 0; i < word_count; ++i) {
//         if (!query.empty()) {
//             query.push_back(' ');
//         }
//         query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
//     }
//     return query;
// }
// vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
//     vector<string> queries;
//     queries.reserve(query_count);
//     for (int i = 0; i < query_count; ++i) {
//         queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
//     }
//     return queries;
// }
// template <typename QueriesProcessor>
// void Test(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
//     LOG_DURATION(mark);
//     const auto documents_lists = processor(search_server, queries);
// }
// #define TEST(processor) Test(#processor, processor, search_server, queries)
// int main() {
//     mt19937 generator;
//     const auto dictionary = GenerateDictionary(generator, 2'000, 25);
//     const auto documents = GenerateQueries(generator, dictionary, 20'000, 10);
//     SearchServer search_server(dictionary[0]);
//     for (size_t i = 0; i < documents.size(); ++i) {
//         search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
//     }
//     const auto queries = GenerateQueries(generator, dictionary, 2'000, 7);
//     TEST(ProcessQueriesJoined);
// }


























// #include "request_queue.h"
// #include "paginator.h"
// #include "process_queries.h"
// #include "remove_duplicates.h"

// #include <iostream>
// #include <vector>


// //using namespace std::literals;
// using namespace std;

// int main() {
//     SearchServer search_server("and with"s);
//     int id = 0;
//     for (
//         const string& text : {
//             "funny pet and nasty rat"s,
//             "funny pet with curly hair"s,
//             "funny pet and not very nasty rat"s,
//             "pet with rat and rat and rat"s,
//             "nasty rat with curly hair"s,
//         }
//     ) {
//         search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
//     }
//     const vector<string> queries = {
//         "nasty rat -not"s,
//         "not very funny nasty pet"s,
//         "curly hair"s
//     };
//     id = 0;
//     for (
//         const auto& documents : ProcessQueries(search_server, queries)
//     ) {
//         cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
//     }
//     return 0;
// } 