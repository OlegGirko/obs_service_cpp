# obs_service_cpp

A C++ library to simplify writing source services for
[OBS](https://openbuildservice.org/).

It provides a simple domain-specific language embedded in C++ program
to define an OBS source service and its parameters.

## Prerequisites

You need:

* [CMake](https://cmake.org/) version 3.27 or later;
* a C++23 compiler (like [GCC](https://gcc.gnu.org/) or
  [Clang](https://clang.llvm.org/)) with a standard C++23 library;
* [Boost.Program_options](https://www.boost.org/library/latest/program_options/)
  C++ library.

## License

This project is licensed under the
[**GNU General Public License v3.0**](https://www.gnu.org/licenses/gpl-3.0).
See the `LICENSE` file for full details.

## Usage

1. Add `#include <obs/service.hpp>`.
2. Use `obs::service<>` template to define the service with the following
   template parameters:
   * service name (string literal);
   * short summary (string literal);
   * long description (string literal);
   * one or more `obs::param<>` templates
     with the following template parameters:
     - parameter type,
     - parameter name,
     - parameter description.
3. Create an object of `obs::service<>` template mentioned above
   in the `main()` function,
   using `argc` and `argv` arguments of `main()` function to initialise it.
   The object's constructor takes care ot parsing all necessary command line
   options according to OBS service conventions.
   Also, it provides special handling for the following command line options:
   * `--help`: prints command line options description and exits;
   * `--xml`: prints OBS service XML description and exits;
     this description can be written to
     `NAME.service` file (where `NAME` is the name of the OBS service).
4. The `obs::service<>` object will store values of all service parameters.
   You can access them using `get<"name">()` member function template
   (where `"name"` is the string literal with parameter name).
   Use rest of the `main()` function to implement whatever your service
   should do.
5. Don't forget to handle exceptions to print error message
   if something goes wrong.

You can use any parameter type (first argument os `obs::param<>` template)
as long as there is an input operator (`<<`) for this type.
If you want to use a type that has no input operator, you have to define it.

By default, parameter is required, and there is an exception thrown
if one is not provided by command line arguments.

You can use `std::optional<TYPE>` template as its type
to make your parameter optional.
It will be stored as `std::optional<TYPE>` in the service,
so you can correctly check whether the value is missing.

If your parameter can have multiple values,
use `std::vector<TYPE>` template as its type.

Parameters with `bool` type can have the following values:

- `true`, `yes`, `on` or `1` for `true`;
- `false`, `no`, `off`, `0` or missing parameter for `false`.

## Example

This example just prints values of its parameters.

```c++
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
    std::cout << "outdir = " << srv.outdir() << "\n";
    std::cout << "p1 = " << srv.get<"p1">() << "\n";
    const service::param_value_type<"p2"> &p2 = srv.get<"p2">();
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
}
```

If you compile this program as `obs_service_example`,
you can invoke it like this:

```sh
obs_service_example --outdir /tmp --p1 aaa --p2 42 --p3 a1 --p3 a2 --p4 yes
```

to get the following output:

```
outdir = "/tmp"
p1 = aaa
p2 = 42
p3:
a1
a2
p4 = true
```
