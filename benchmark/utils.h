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
#include <string_view>

#include <cstdio>
#include <chrono>

template <typename T>
void read_arg(const char* data, T& dest, const char* error, const char* type)
{
    if (sscanf(data, type, &dest) != 1) {
        std::cerr << error << std::endl;
        exit(-1);
    }
}

inline void print_settings(std::string_view type, size_t elements, size_t seed, size_t bytes, double deletes,
                           double read_rate, size_t mv)
{
    std::cout << "Using " << type << ", inserting " << elements << " items of " << bytes << " bytes produced via seed "
              << seed << ". Of those " << (deletes * 100.0)
              << "% will be deleted at random, and for each insert, on average " << read_rate
              << " extra reads will occur"
              << ". All bytes in rand data are mod " << mv << std::endl;
}

struct Timer
{
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;
    Timer() = default;
    Duration elapsed() const { return {Clock::now() - start}; }
    ~Timer() { std::cout << "Completed in " << elapsed().count() << std::endl; }
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

private:
    TimePoint start = Clock::now();
};

#endif  // PTRIE_UTILS_H
