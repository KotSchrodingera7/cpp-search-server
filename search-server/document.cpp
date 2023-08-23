//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎

#include "document.h"

std::ostream& operator<<(std::ostream &out, Document document) {
    using namespace std::literals;
    out << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s;
    return out;
}