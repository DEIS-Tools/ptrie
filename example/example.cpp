#include <ptrie/ptrie.h>
#include <ptrie/ptrie_map.h>

#include <iostream>

std::ostream& operator<<(std::ostream& os, const ptrie::set<int>& set)
{
    const auto end = set.end();
    for (auto begin = set.begin(); begin != end; ++begin) {
        int value;
        begin.unpack(&value);
        os << " " << value;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ptrie::map<int, std::string>& map)
{
    const auto end = map.end();
    for (auto begin = map.begin(); begin != end; ++begin) {
        int key;
        begin.unpack(&key);
        os << key << " -> " << *begin << std::endl;
    }
    return os;
}

int main()
{
    auto foo = ptrie::set<int>{};  // divisible by 3 but not by 5
    auto bar = ptrie::set<int>{};  // divisible by 5 but not by 3
    auto baz = ptrie::set<int>{};  // divisible by both 3 and 5
    auto map = ptrie::map<int, std::string>{};
    for (auto i = -32; i < 31; ++i) {
        if (i % 3 == 0) {
            if (i % 5 == 0) {
                baz.insert(i);
                map[i] = "baz";
            } else {
                foo.insert(i);
                map[i] = "foo";
            }
        } else if (i % 5 == 0) {
            bar.insert(i);
            map[i] = "bar";
        }
    }
    std::cout << "foo:" << foo << std::endl;
    std::cout << "bar:" << bar << std::endl;
    std::cout << "baz:" << baz << std::endl;
    std::cout << "map:\n" << map << std::endl;
}
