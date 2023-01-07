//  RcppTomlPlusPlus -- Rcpp bindings to TOML via cpptomlplusplus
//                      (based on earlier work in RcppTOML using cpptoml)
//
//  Copyright (C) 2015 - 2023  Dirk Eddelbuettel
//
//  This file is part of RcppTomlPlusPlus
//
//  RcppTomlPlusPlus is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  RcppTomlPlusPlus is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with RcppTomlPlusPlus.  If not, see <http://www.gnu.org/licenses/>.

#include <Rcpp/Lightest>
#include <toml++/toml.h>
#include "parse.h"

void visitArray(const toml::array& arr, const std::string& ind = "") {
    arr.for_each([ind](auto&& elem) {
        if (elem.is_table()) {
            Rcpp::Rcout << "is table\n";
            visitTable(*elem.as_table(), ind + std::string("  "));
        } else if (elem.is_array()) {
            Rcpp::Rcout << "is array\n";
            visitArray(*elem.as_array(), ind + std::string("  "));
        } else if (elem.is_value()) {
            Rcpp::Rcout << ind << elem << "\n";
        } else {
            Rcpp::Rcout << ind << "unknown type: " << elem.type() << "\n";
        }
    });
}

void visitTable(const toml::table& tbl, const std::string& ind = "") {
    tbl.for_each([ind](const toml::key& key, auto&& val) {
        Rcpp::Rcout << ind << key << " ";
        if (val.is_array_of_tables()) {
            Rcpp::Rcout << "is array of tables\n";
        } else if (val.is_table()) {
            Rcpp::Rcout << "is table\n";
            visitTable(*val.as_table(), ind + std::string("  "));
        } else if (val.is_array()) {
            Rcpp::Rcout << "is array\n";
            visitArray(*val.as_array(), ind + std::string("  "));
        } else if (val.is_value()) {
            Rcpp::Rcout << "is value: " << val << "\n";
        } else {
            Rcpp::Rcout << ind << "unknown type: " << val.type() << "\n";
        }
    });
}
