// This file is part of obs_service_cpp.
//
// obs_service_cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Blooto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with obs_service_cpp.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _OBS_SERVICE_HPP
#define _OBS_SERVICE_HPP

#include <algorithm>
#include <optional>
#include <vector>

namespace obs {

    template<size_t N> struct string_literal {
        constexpr string_literal() {}
        constexpr string_literal(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        char value[N];
    };

    template <size_t N1, size_t N2>
    constexpr string_literal<N1 + N2 - 1> operator+(const string_literal<N1> s1,
                                                    const string_literal<N2> s2)
    {
        string_literal<N1 + N2 - 1> result;
        std::copy_n(s1.value, N1 - 1, result.value);
        std::copy_n(s2.value, N2, result.value + N1 - 1);
        return result;
    }

    template <typename TYPE, string_literal NAME, string_literal DESCR>
    struct param {
        using type = TYPE;
    };

    namespace detail {
        template <typename TYPE> struct param_extra {
            constexpr static const string_literal xml = "    <required/>\n";
        };

        template <typename TYPE> struct param_extra<std::optional<TYPE>> {
            constexpr static const string_literal xml = "";
        };

        template <typename TYPE> struct param_extra<std::vector<TYPE>> {
            constexpr static const string_literal xml =
                "    <allowmultiple/>\n";
        };

        template <typename... PARAMS> struct params;

        template<> struct params<> {
            constexpr static const string_literal xml = "";
        };

        template <typename TYPE,
                  string_literal NAME,
                  string_literal DESCR,
                  typename... PARAMS>
        struct params<param<TYPE, NAME, DESCR>, PARAMS...>: params<PARAMS...> {
            constexpr static const string_literal xml =
                string_literal("  <parameter name=\"") + NAME +
                string_literal("\">\n") +
                string_literal("    <description>") + DESCR +
                string_literal("</description>\n") +
                param_extra<TYPE>::xml +
                string_literal("  </parameter>\n") +
                params<PARAMS...>::xml;
        };
    }

    template <string_literal NAME,
              string_literal SUMMARY,
              string_literal DESCR,
              typename... PARAMS>
    struct service: detail::params<PARAMS...> {
        constexpr static const string_literal name = NAME;
        constexpr static const string_literal summary = SUMMARY;
        constexpr static const string_literal description = DESCR;
        constexpr static const string_literal xml =
            string_literal("<service name=\"") + name +
            string_literal("\">\n") +
            string_literal("  <summary>") + summary +
            string_literal("</summary>\n") +
            string_literal("  <description>") + description +
            string_literal("</description>\n") +
            detail::params<PARAMS...>::xml +
            string_literal("</service>\n");
    };
}

#endif

