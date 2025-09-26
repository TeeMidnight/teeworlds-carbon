/*
MIT License

Copyright (c) 2025 Bamcane

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __SUGARFORMAT_HPP__
#define __SUGARFORMAT_HPP__

#include <cstddef>

#include <fmt/format.h>

namespace sugarformat
{
    template <typename... Args>
    std::string format(fmt::string_view fmt, Args &&...args)
    {
        return fmt::vformat(fmt, fmt::make_format_args(args...));
    }

    template <typename... Args>
    size_t format_to(char *buffer, size_t buffer_size, fmt::string_view fmt, Args &&...args)
    {
        if (!buffer || buffer_size == 0)
        {
            try
            {
                return fmt::formatted_size(fmt::runtime(fmt), std::forward<Args>(args)...);
            }
            catch (...)
            {
                return 0;
            }
        }

        if (buffer_size == 1)
        {
            buffer[0] = '\0';
            try
            {
                return fmt::formatted_size(fmt::runtime(fmt), std::forward<Args>(args)...);
            }
            catch (...)
            {
                return 1;
            }
        }

        try
        {
            auto result = fmt::format_to_n(buffer, buffer_size - 1, fmt::runtime(fmt), std::forward<Args>(args)...);
            buffer[result.size] = '\0';
            return fmt::formatted_size(fmt::runtime(fmt), std::forward<Args>(args)...);
        }
        catch (...)
        {
            buffer[0] = '\0';
            return 0;
        }
    }
}

#endif // __SUGARFORMAT_HPP__
