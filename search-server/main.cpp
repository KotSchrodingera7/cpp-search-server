#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};
struct PlusMinus
{
    set<string> minus_word;
    set<string> plus_word;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        for(const auto &word : words)
        {
            documents_[word][document_id] += 1./words.size();
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const PlusMinus query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
 
    int document_count_ = 0;
    //vector<DocumentContent> documents_;
    map<string, map<int, double>> documents_;

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    PlusMinus ParseQuery(const string& text) const {
        PlusMinus query_words;
        for (const string &word : SplitIntoWordsNoStop(text)) {
            if( word[0] == '-')
            {
                query_words.minus_word.insert(word.substr(1));
                continue;
            }
            query_words.plus_word.insert(word);
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const PlusMinus& query_words) const {
        map<int, double> document_to_relevance;
        vector<Document> matched_documents;
        
        for(const auto &word : query_words.plus_word)
        {
            double idf = 0.0;
            auto p = documents_.find(word);
            if( p != documents_.end() )
            {
                idf = log(static_cast<double>((double)document_count_/(double)p->second.size()));
                for(const auto [id, relevance] : p->second)
                {
                    document_to_relevance[id] += static_cast<double>(relevance*idf);
                }
            }
        }
        
        for(auto &word: query_words.minus_word)
        {
            auto p = documents_.find(word);
            if( p != documents_.end() )
            {
                for(const auto [id, relevance] : p->second)
                {
                    document_to_relevance.erase(id);
                }
            }
        }
        
        for(const auto [id, relevance] : document_to_relevance)
        {
            matched_documents.push_back({id, relevance});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}