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
#include <cstdlib>

#include <boost/program_options.hpp>

namespace obs {

    namespace po = boost::program_options;

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
        template <typename TYPE> struct param_traits {
            using type = TYPE;
            using po_type = TYPE;

            constexpr static const string_literal extra_xml =
                "    <required/>\n";

            template <string_literal NAME>
            static const type &vm_get(const po::variables_map &vm)
            {
                if (!vm.count(NAME.value))
                    throw po::required_option(NAME.value);
                return vm[NAME.value].template as<type>();
            }
        };

        template <typename TYPE> struct param_traits<std::optional<TYPE>> {
            using type = std::optional<TYPE>;
            using po_type = TYPE;

            constexpr static const string_literal extra_xml = "";

            template <string_literal NAME>
            static type vm_get(const po::variables_map &vm) {
                if (vm.count(NAME.value))
                    return vm[NAME.value].template as<po_type>();
                else
                    return {};
            }
        };

        template <typename TYPE> struct param_traits<std::vector<TYPE>> {
            using type = std::vector<TYPE>;
            using po_type = type;

            constexpr static const string_literal extra_xml =
                "    <allowmultiple/>\n";

            template <string_literal NAME>
            static type vm_get(const po::variables_map &vm) {
                if (vm.count(NAME.value))
                    return vm[NAME.value].template as<type>();
                else
                    return {};
            }
        };

        template <string_literal NAME> struct name_tag {
            constexpr static const char *const value = NAME.value;
        };

        template <typename... PARAMS> struct params;

        template<> struct params<> {
            constexpr static const string_literal xml_literal = "";

            static void add_options(po::options_description &) {}

            params(const po::variables_map &) {}
        };

        template <typename TYPE,
                  string_literal NAME,
                  string_literal DESCR,
                  typename... PARAMS>
        struct params<param<TYPE, NAME, DESCR>, PARAMS...>: params<PARAMS...> {
            constexpr static const string_literal xml_literal =
                string_literal("  <parameter name=\"") + NAME +
                string_literal("\">\n") +
                string_literal("    <description>") + DESCR +
                string_literal("</description>\n") +
                param_traits<TYPE>::extra_xml +
                string_literal("  </parameter>\n") +
                params<PARAMS...>::xml_literal;

            using type = TYPE;
            type value;

            constexpr const type &get_impl(name_tag<NAME>) const {return value;}

            template <string_literal N>
            constexpr const auto &get_impl(name_tag<N>) const {
                return this->params<PARAMS...>::get_impl(name_tag<N>());
            }

            static void add_options(po::options_description &desc) {
                using po_type = typename param_traits<type>::po_type;
                desc.add_options()(NAME.value,
                                   po::value<po_type>(),
                                   DESCR.value);
                params<PARAMS...>::add_options(desc);
            }

            params(const po::variables_map &vm)
            : params<PARAMS...>{vm},
            value{param_traits<type>::template vm_get<NAME>(vm)} {}
        };
    }

    template <string_literal NAME,
              string_literal SUMMARY,
              string_literal DESCR,
              typename... PARAMS>
    class service: public detail::params<PARAMS...> {
        constexpr static const string_literal xml_literal =
            string_literal("<service name=\"") + NAME +
            string_literal("\">\n") +
            string_literal("  <summary>") + SUMMARY +
            string_literal("</summary>\n") +
            string_literal("  <description>") + DESCR +
            string_literal("</description>\n") +
            detail::params<PARAMS...>::xml_literal +
            string_literal("</service>\n");

            static po::variables_map parse_program_args(int argc,
                                                        const char *const *argv)
            {
                po::options_description desc {options_description()};
                po::variables_map vm;
                po::store(po::parse_command_line(argc, argv, desc), vm);
                po::notify(vm);
                if (vm.count("help")) {
                    std::cout << desc << "\n";
                    std::exit(0);
                } else if (vm.count("xml")) {
                    std::cout << service::xml;
                    std::exit(0);
                }
                return vm;
            }

    public:
        constexpr static const char *const name = NAME.value;
        constexpr static const char *const summary = SUMMARY.value;
        constexpr static const char *const description = DESCR.value;
        constexpr static const char *const xml = xml_literal.value;

        static po::options_description options_description() {
            po::options_description desc("Allowed options");
            desc.add_options()
            ("help", "produce help message")
            ("xml", "print OBS service XML description")
            ;
            detail::params<PARAMS...>::add_options(desc);
            return desc;
        }

        service(const po::variables_map &vm)
        : detail::params<PARAMS...>{vm} {}

        service(int argc, const char *const *argv)
        : detail::params<PARAMS...>{parse_program_args(argc, argv)} {}

        template <string_literal N> const auto &get() const {
            return this->get_impl(detail::name_tag<N>());
        }

    };
}

#endif

