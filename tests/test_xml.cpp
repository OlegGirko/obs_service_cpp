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

#include <iostream>
#include <string>
#include <optional>
#include <vector>
#include <obs/service.hpp>

#include <boost/program_options.hpp>

using service =
    obs::service<"example", "Example service", "Example service.",
                 obs::param<std::string, "p1", "Param 1">,
                 obs::param<std::optional<std::string>, "p2", "Param 2">,
                 obs::param<std::vector<std::string>, "p3", "Param 3">>;

int main(int argc, const char *const *argv) {
    namespace po = boost::program_options;
    po::options_description desc {service::options_description()};
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << "\n";
    } else if (vm.count("xml")) {
        std::cout << service::xml;
    } else {
        service srv{vm};
        std::cout << "p1 = " << srv.get(obs::name_tag<"p1">()) << "\n";
        const auto &p2 = srv.get(obs::name_tag<"p2">());
        if (p2)
            std::cout << "p2 = " << *p2 << std::endl;
        else
            std::cout << "p2 is absent\n";
        std::cout << "p3:\n";
        for (const auto &p: srv.get(obs::name_tag<"p3">()))
            std::cout << p << std::endl;
    }
    return 0;
}
