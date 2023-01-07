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

SEXP getValue(const toml::node& nod, bool escape=true) {
    toml::node_type nodetype = nod.type();
    //Rcpp::Rcout << "Type of node is " << nodetype << std::endl;
    if (nodetype == toml::node_type::string) {
        std::string val{*nod.as_string()};
        if (escape) {
            val = escapeString(val);
        }
        Rcpp::String se(val, CE_UTF8);
        return Rcpp::wrap(se);
    }
    else if (nodetype == toml::node_type::integer) {
        int64_t val{*nod.as_integer()};
        int32_t ival = static_cast<int32_t>(val); // known lossy but R has really only int32
        return Rcpp::wrap(ival);
    }
    else if (nodetype == toml::node_type::floating_point) {
        double val{*nod.as_floating_point()};
        return Rcpp::wrap(val);
    }
    else if (nodetype == toml::node_type::boolean) {
        bool val{*nod.as_boolean()};
        return Rcpp::wrap(val);
    }
    else if (nodetype == toml::node_type::date) {
        const toml::date val{*nod.as_date()};
        Rcpp::Date d(val.year, val.month, val.day);
        return Rcpp::wrap(d);
    }
    else if (nodetype == toml::node_type::date_time) {
        const toml::date_time val{*nod.as_date_time()};
        const toml::date date = val.date;
        const toml::time time = val.time;
        const std::optional<toml::time_offset> offset = val.offset;

        std::tm tm{};
        tm.tm_year = date.year - 1900;
        tm.tm_mon = date.month - 1;
        tm.tm_mday = date.day;
        tm.tm_hour = time.hour;
        tm.tm_min = time.minute;
        tm.tm_sec = time.second;
        //tm.tm_isdst = 1; // not filled
        time_t tt = local_timegm(&tm); // helper also used earlier in RcppTOML
        if (!val.is_local()) {
            tt = tt - offset->minutes*60;
        }
        Rcpp::DatetimeVector dt(1, "UTC"); // we always set UTC as RcppTOML did
        dt[0] =  tt + time.nanosecond * 1.0e-9;
        return Rcpp::wrap(dt);
    }
    else if (nodetype == toml::node_type::time) {
        const toml::time val{*nod.as_time()};
        std::stringstream ss;   		// print the time to string as there is no
        ss << val;               		// base R time type (we could pull in hms
        return Rcpp::wrap(ss.str());    // but rather not have the dependency
    }
    std::stringstream ss;   // because we have no Datetime ctor from components :-/
    ss << nodetype;
    Rcpp::warning("Unknown type: %s", ss.str());
    return R_NilValue;
}

SEXP getTable(const toml::table& tbl, bool escape = true) {
    Rcpp::StretchyList sl;
    //tbl.for_each([ind,sl](const toml::key& key, auto&& val) {
    for (auto it = tbl.cbegin(); it != tbl.cend(); it++) {
        const toml::key& key = it->first;
        const toml::node& val = it->second;
        if (val.is_array_of_tables()) {
            //Rcpp::Rcout << "is array of tables\n";
            //Rcpp::Rcout << key << " is array of tables\n";
            Rcpp::StretchyList l;
            const toml::array& arr = *tbl.get_as<toml::array>(key);
            for (auto ait = arr.cbegin(); ait != arr.cend(); ait++) {
                l.push_back(getTable(*ait->as_table(), escape));
            }
            sl.push_back(Rcpp::Named(key.data()) = Rcpp::as<Rcpp::List>(l));
        } else if (val.is_table()) {
            //Rcpp::Rcout << "is table\n";
            sl.push_back(Rcpp::Named(key.data()) = getTable(*val.as_table(), escape));
        } else if (val.is_array()) {
            //Rcpp::Rcout << "is array\n";
            //visitArray(*val.as_array(), ind + std::string("  "));
            //sl.push_back(getArray(*val.as_array()));
            sl.push_back(Rcpp::Named(key.data()) = getArray(*val.as_array(), escape));
         } else if (val.is_value()) {
            //Rcpp::Rcout << "is value of type: " << val.type() << "\n";
            sl.push_back(Rcpp::Named(key.data()) = getValue(val, escape));
        } else {
            Rcpp::Rcout << "unknown type in table: " << val.type() << "\n";
        }
    }
    return Rcpp::as<Rcpp::List>(sl);
}

SEXP getArray(const toml::array& arr, bool escape) {
    Rcpp::StretchyList sl;
    bool nonested = true;       // ie no embedded array
    //tbl.for_each([ind,sl](const toml::key& key, auto&& val) {
    for (auto it = arr.cbegin(); it != arr.cend(); it++) {
        const toml::node& val = *it;
        if (val.is_array()) {
            //Rcpp::Rcout << "is array (in array)\n";
            sl.push_back(getArray(*val.as_array(), escape));
            nonested = false;
        } else if (val.is_value()) {
            sl.push_back(getValue(val, escape));
        } else {
            Rcpp::Rcout << "unknown type in array: " << val.type() << "\n";
        }
    }
    if (nonested)
        return collapsedList(Rcpp::as<Rcpp::List>(sl));
    else
        return Rcpp::as<Rcpp::List>(sl);
}

//' @noRd
// [[Rcpp::export()]]
Rcpp::List tomlparseImpl(const std::string input,
                         bool verbose=false,
                         bool fromfile=true,
                         bool includize=false,
                         bool escape=true) {

    if (includize == true)
        Rcpp::warning("The 'includize' option is included for legacy support and has no effect.");

    const toml::table tbl = (fromfile) ? toml::parse_file(input) : toml::parse(input);

    //if (verbose) visitTable(tbl);

    Rcpp::StretchyList sl;
    //tbl.for_each([sl](const toml::key& key, auto&& val) {
    for (auto it = tbl.cbegin(); it != tbl.cend(); it++) {
        const toml::key& key = it->first;
        const toml::node& nod = it->second;
        //Rcpp::Rcout << key << " ";
        if (nod.is_array_of_tables()) {
            //Rcpp::Rcout << key << " is array of tables\n";
            Rcpp::StretchyList l;
            const toml::array& arr = *tbl.get_as<toml::array>(key);
            for (auto ait = arr.cbegin(); ait != arr.cend(); ait++) {
                l.push_back(getTable(*ait->as_table(), escape));
            }
            sl.push_back(Rcpp::Named(key.data()) = Rcpp::as<Rcpp::List>(l));
        } else if (nod.is_table()) {
            //Rcpp::Rcout << "is table\n";
            sl.push_back(Rcpp::Named(key.data()) = getTable(*nod.as_table(), escape));
            //visitTable(*nod.as_table(), ind + std::string("  "));
        } else if (nod.is_array()) {
            //Rcpp::Rcout << "is array\n";
            //visitArray(*nod.as_array(), ind + std::string("  "));
            sl.push_back(Rcpp::Named(key.data()) = getArray(*nod.as_array(), escape));
        } else if (nod.is_value()) {
            //Rcpp::Rcout << "is value of type: " << nod.type() << "\n";
            sl.push_back(Rcpp::Named(key.data()) = getValue(nod, escape));
        } else {
            Rcpp::Rcout << "unknown type: " << nod.type() << "\n";
        }
    }

    return Rcpp::as<Rcpp::List>(sl);
}
