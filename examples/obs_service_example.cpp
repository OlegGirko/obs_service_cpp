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
#include <exception>
#include <obs/service.hpp>

using service =
    obs::service<"example", "Example service",
                 "An example service that prints its parameters.",
                 obs::param<std::string, "p1", "String parameter [required]">,
                 obs::param<std::optional<unsigned int>, "p2",
                            "Integer parameter [optional]">,
                 obs::param<std::vector<std::string>, "p3",
                            "Another string parameter [multiple]">,
                 obs::param<bool, "p4", "Boolean parameter">>;

int main(int argc, const char *const *argv) try {
    service srv{argc, argv};
    std::cout << "p1 = " << srv.get<"p1">() << "\n";
    const auto &p2 = srv.get<"p2">();
    if (p2)
        std::cout << "p2 = " << *p2 << std::endl;
    else
        std::cout << "p2 is absent\n";
    std::cout << "p3:\n";
    for (const auto &p: srv.get<"p3">())
        std::cout << p << std::endl;
    std::cout << "p4 = " << (srv.get<"p4">() ? "true" : "false") << "\n";
    return 0;
} catch (const std::exception &e) {
    std::cerr << argv[0] << ": Error: " << e.what() << "\n";
    return 1;
} catch (...) {
    std::cerr << "Unknown error!\n";
    return 1;
}
