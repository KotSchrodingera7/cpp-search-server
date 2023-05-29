// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:

#include <cstring>

int how_much_three(void)
{
    int count = 0;
    std::string str;

    for(int i = 1; i < 1000; ++i)
    {
        for( const auto &i : std::to_string(i) )
        {
            if( i == '3')
            {
                ++count;
                break;
            }
        }
    }

    return count;
}
// Закомитьте изменения и отправьте их в свой репозиторий.
