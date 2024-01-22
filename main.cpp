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

class SearchServer {
public:
    void SetStopWords(const string& text) {

        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }

    }

    void AddDocument(int document_id, const string& document) {

        const vector<string> words_no_stop = SplitIntoWordsNoStop(document);
        int size = words_no_stop.size();
        const double tf = 1./size; 
        for(const string& word: words_no_stop) {
            word_to_document_freqs_[word][document_id] += tf; 
        }
        
        ++document_count_;
    }


    vector<Document> FindTopDocuments(const string& raw_query) const {

        const Query query_words = ParseQuery(raw_query);
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
     struct Query{
        set<string> words_plus;
        set<string> words_minus;
     };
  
    //сопоставление слова с id_doc -> TF
    map<string, map<int, double>> word_to_document_freqs_;
    int document_count_ = 0;
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

    Query ParseQuery(const string& text) const {

        Query query_words;
        for(const string& word: SplitIntoWords(text)){
            if(word[0] == '-'){
                if(!IsStopWord(word.substr(1))){
                    query_words.words_minus.insert(word.substr(1));
                }
            } 
            else{
                if(!IsStopWord(word)){
                    query_words.words_plus.insert(word);
                } 
            } 
        }
        return query_words;
    }
    
    double GetIDFByWord(const string& word) const{
        if(word.empty()) return 0;
        return log(static_cast<double>(document_count_)/ word_to_document_freqs_.at(word).size());
    }
    
    bool IsDocHevMinusQuery(const pair<int, double>& doc, const set<string>& words_minus) const{
        for(const string& wor : words_minus){
            if(word_to_document_freqs_.count(wor) == 0){
                continue;
            } 
            for(const auto& [id , rel]: word_to_document_freqs_.at(wor)){
                if(id != doc.first){
                   continue; 
                } 
                return true;
            }
        }
        return false;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        
        vector<Document> matched_documents;
        
        if(query_words.words_plus.size() == 0){
            return  matched_documents;
        }
        
        map<int, double> id_rel_doc;
        
        for(const string& word_plus : query_words.words_plus){
            if(word_to_document_freqs_.count(word_plus) == 0) continue;
            const double idf = GetIDFByWord(word_plus);
            for(const auto& [id , rel]: word_to_document_freqs_.at(word_plus)){
                id_rel_doc[id] += rel * idf;
            }
        }        
        
        for(const auto& doc : id_rel_doc){
            if(IsDocHevMinusQuery(doc, query_words.words_minus)) continue;
            matched_documents.push_back({doc.first, doc.second});
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