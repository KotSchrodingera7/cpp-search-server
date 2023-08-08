#pragma once
//Вставьте сюда своё решение из урока «‎Очередь запросов».‎


#include <vector>
#include "document.h"

template<typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator begin, Iterator end) :
                    begin_(begin), end_(end) {}
    
    Iterator begin() const {
        return begin_;
    }

    Iterator end() const {
        return end_;
    }
private:
    Iterator begin_;
    Iterator end_;
};

template<typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, int size) {
        auto range = distance(begin, end);
        auto start_it = begin;
        while( range > size ) {
            range_.push_back({start_it, start_it + size});
            range -= size;
            start_it += size;   
        }
        range_.push_back({start_it, end});
    }
    auto begin() const {
        return range_.begin();
    }

    auto end() const {
        return range_.end();
    }
private:
    std::vector<IteratorRange<Iterator>> range_;
    int size_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template<typename Iterator>
std::ostream& operator<<(std::ostream &out, const IteratorRange<Iterator> &document) {
    for(auto it = document.begin(); it != document.end(); ++it) {
        out << *it;
    }
    return out;
}
