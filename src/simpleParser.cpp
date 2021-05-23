
// simplerParser.cpp
//
// initial (minimal) RcppTomlPlusPlus example
//
// taken from upstream file examples/simple_parser.cpp and Rcpp-ified for R use

// This file is a part of toml++ and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/tomlplusplus/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

#include "examples.h"

#define TOML_UNRELEASED_FEATURES 1
#include <toml++/toml.h>

using namespace std::string_view_literals;

#define STRICT_R_HEADERS
#include <Rcpp.h>

// [[Rcpp::export]]
int simpleParser(const std::string & filename) {
    examples::init();

    const auto path = std::string_view{ filename };
    try {
        const auto table = toml::parse_file(path);
        Rcpp::Rcout << table << "\n";
    }
    catch (const toml::parse_error& err) {
        std::cerr << err << "\n";
        return 1;
    }
    return 0;
}
