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
#include <filesystem>
#include <cstdlib>

#include <boost/program_options.hpp>

namespace obs {

    namespace po = boost::program_options;

    //! @brief String literal for template parameters
    //! @tparam N length of the string (including zero byte at the end)
    //! @details
    //! This is simple char array wrapper that can be used for
    //! passing string literals as template arguments.
    //! You can define a template to store compile-time strings like this:
    //! @code
    //! template <obs::string_literal NAME> struct tag {
    //!     constexpr static const char *const value = NAME.value;
    //! };
    //! @endcode
    //! and then use it like this:
    //! @code
    //! std::cout << tag<"Hello">::value << "\n";
    //! @endcode
    template<size_t N> struct string_literal {
        constexpr string_literal() {}
        constexpr string_literal(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        char value[N]; //!< value of the string literal
    };

    //! @brief Concatenate two string literals
    //! @param s1 first string literal to concatenate
    //! @param s2 second string literal to concatenate
    //! @return constructed string literal with concatenated s1 and s2
    template <size_t N1, size_t N2>
    constexpr string_literal<N1 + N2 - 1> operator+(const string_literal<N1> s1,
                                                    const string_literal<N2> s2)
    {
        string_literal<N1 + N2 - 1> result;
        std::copy_n(s1.value, N1 - 1, result.value);
        std::copy_n(s2.value, N2, result.value + N1 - 1);
        return result;
    }

    //! @brief OBS service parameter definition
    //! @tparam TYPE parameter type
    //! @tparam NAME a string literal containing parameter name
    //! @tparam DESCR a string literal containing parameter description
    //! @details
    //! Usage example:
    //! @code
    //! obs::param<std::string, "p1", "String parameter [required]">
    //! @endcode
    template <typename TYPE, string_literal NAME, string_literal DESCR>
    struct param {
        //! OBS parameter type
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
                constexpr static const string_literal po_name =
                    string_literal("--") + NAME;
                if (!vm.count(NAME.value))
                    throw po::required_option(po_name.value);
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

        template <> struct param_traits<bool> {
            using type = bool;
            using po_type = bool;

            constexpr static const string_literal extra_xml =
                "    <allowedvalue>true</allowedvalue>\n"
                "    <allowedvalue>yes</allowedvalue>\n"
                "    <allowedvalue>on</allowedvalue>\n"
                "    <allowedvalue>1</allowedvalue>\n"
                "    <allowedvalue>false</allowedvalue>\n"
                "    <allowedvalue>no</allowedvalue>\n"
                "    <allowedvalue>off</allowedvalue>\n"
                "    <allowedvalue>0</allowedvalue>\n";

            template <string_literal NAME>
            static type vm_get(const po::variables_map &vm) {
                if (vm.count(NAME.value))
                    return vm[NAME.value].template as<type>();
                else
                    return false;
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

    //! @brief OBS source service
    //! @tparam NAME string literal containing service name
    //! @tparam SUMMARY string literal containing short summary
    //! @tparam DESCR string literal containing long description
    //! @tparam PARAMS list of obs::param templates with parameter definitions
    //! @details
    //! Define an object of this class with template parameters
    //! describing OBS service name, short summary, long description,
    //! as well as all service parameters.
    //!
    //! Construct the object using `argc` and `argv` argumants passed
    //! to the `main()` function to store values of its parameters
    //! usimg command line options according to OBS service conventions.
    //! You can access them using `get<"name">()` member function template
    //! (where `"name"` is the string literal with parameter name).
    //!
    //! Use rest of the `main()` function to implement whatever
    //! your service should do.
    //!
    //! Example:
    //! @code
    //! #include <iostream>
    //! #include <string>
    //! #include <optional>
    //! #include <exception>
    //! #include <obs/service.hpp>
    //!
    //! using service =
    //!     obs::service<"example", "Example service",
    //!                  "An example service that prints its parameters.",
    //!                  obs::param<std::string, "p1",
    //!                            "String parameter [required]">,
    //!                  obs::param<std::optional<unsigned int>, "p2",
    //!                             "Integer parameter [optional]">>;
    //!
    //! int main(int argc, const char *const *argv) try {
    //!     service srv{argc, argv};
    //!     std::cout << "outdir = " << srv.outdir() << "\n"
    //!     std::cout << "p1 = " << srv.get<"p1">() << "\n";
    //!     const auto &p2 = srv.get<"p2">();
    //!     if (p2)
    //!         std::cout << "p2 = " << *p2 << std::endl;
    //!     else
    //!         std::cout << "p2 is absent\n";
    //!     return 0;
    //! } catch (const std::exception &e) {
    //!     std::cerr << argv[0] << ": Error: " << e.what() << "\n";
    //!     return 1;
    //! }
    //! @endcode
    template <string_literal NAME,
              string_literal SUMMARY,
              string_literal DESCR,
              typename... PARAMS>
    class service: public detail::params<PARAMS...> {
    public:
        //! Output directory path type
        using path_type = std::filesystem::path;

    private:
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

        path_type outdir_;

    public:
        //! OBS service name
        constexpr static const char *const name = NAME.value;

        //! Short summary
        constexpr static const char *const summary = SUMMARY.value;

        //! Long description
        constexpr static const char *const description = DESCR.value;

        //! OBS service descriptionn in XML format that can be stored
        //! in NAME.service file
        constexpr static const char *const xml = xml_literal.value;

        //! Generates program options description for the service
        //! @return options description to parse all program options necessary
        static po::options_description options_description() {
            po::options_description desc("Allowed options");
            desc.add_options()
            ("outdir", po::value<std::filesystem::path>(), "output directory")
            ("help", "produce help message")
            ("xml", "print OBS service XML description")
            ;
            detail::params<PARAMS...>::add_options(desc);
            return desc;
        }

        //! @brief Construct OBS source service from existing variables_map
        //! @param vm variables map obtained from command line arguments
        service(const po::variables_map &vm)
        : detail::params<PARAMS...>{vm},
          outdir_{detail::param_traits<path_type>::template vm_get<"outdir">(vm)}
        {}

        //! @brief Construct OBS source service from command line arguments
        //! @param argc argument counter, first argument of `main()` function
        //! @param argv argument vector, second argument of `main()` function
        //! @details
        //! Create an object of OBS source service
        //! using `argc` and `argv` arguments of `main()` function
        //! to initialise it.
        //!
        //! The object's constructor takes care ot parsing
        //! all necessary command line options
        //! according to OBS service conventions.
        //!
        //! Also, it provides special handling for
        //! the following command line options:
        //! - `--help`: prints command line options description and exits;
        //! - `--xml`: prints OBS service XML description and exits;
        //!   this description can be written to `NAME.service` file
        //!   (where `NAME` is the name of the OBS service).
        service(int argc, const char *const *argv)
        : service{parse_program_args(argc, argv)} {}

        //! @brief Value of service parameter
        //! @tparam N string literal containing the name of parameter
        //! @return reference to stored parameter value
        template <string_literal N> const auto &get() const {
            return this->get_impl(detail::name_tag<N>());
        }

        //! @brief Output directory path
        //! @return path to the output directory
        const path_type &outdir() const {return outdir_;}
    };
}

#endif

