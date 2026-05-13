/*
 * Copyright Peter G. Jensen <root@petergjoel.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PTRIE_UTILS_H
#define PTRIE_UTILS_H

#include <iostream>
#include <sstream>
#include <string_view>

#include <chrono>

template <typename T>
T read_arg(T default_value, const char* arg, const char* name)
{
    if (std::istringstream{arg} >> default_value)
        return default_value;
    throw std::logic_error{std::string{"Error parsing "} + name};
}

struct Settings
{
    std::string type;
    size_t elements{1024};
    size_t seed{0};
    size_t bytes{16};
    double deletes{0};
    double read_rate{2};
    size_t maxval{256};

private:
    friend std::ostream& operator<<(std::ostream& os, const Settings& s)
    {
        os << "Using " << s.type << "\n\tinserting " << s.elements << " items of " << s.bytes << " bytes"
           << "\n\tproduced via seed " << s.seed << "\n\tOf those " << (s.deletes * 100.0) << "% are deleted at random,"
           << "\n\tand for each insert, on average " << s.read_rate << " extra reads will occur."
           << "\n\tAll bytes in rand data are mod " << s.maxval << '\n';
        return os;
    }
};

inline Settings cli_settings(int argc, const char* argv[])
{
    auto res = Settings{};
    res.type = std::string_view{argv[1]};
    res.elements = read_arg(size_t{1024}, argv[2], "<number of elements>");
    if (argc > 3)
        res.seed = read_arg(res.seed, argv[3], "<seed>");
    if (argc > 4)
        res.bytes = read_arg(res.bytes, argv[4], "<bytes>");
    if (argc > 5)
        res.deletes = read_arg(res.deletes, argv[5], "<delete ratio>");
    if (argc > 6) {
        res.read_rate = read_arg(res.read_rate, argv[6], "<read rate>");
        if (res.read_rate <= 0)
            throw std::invalid_argument{"<read rate> must be greater than 0"};
    }
    if (argc > 7)
        res.maxval = read_arg(res.maxval, argv[7], "<max byte val>");
    return res;
}

struct Timer
{
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;
    Timer() = default;
    Duration elapsed() const { return {Clock::now() - start}; }
    ~Timer() { std::cout << "Completed in " << elapsed().count() << '\n'; }
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;

private:
    TimePoint start = Clock::now();
};

#endif  // PTRIE_UTILS_H
