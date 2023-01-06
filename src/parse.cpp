
#include <Rcpp/Lightest>

#include <toml++/toml.h>

SEXP getTable(const toml::table& tbl);
SEXP getArray(const toml::array& array);
SEXP getValue(const toml::node& node);

void visitTable(const toml::table& tbl, const std::string& ind);

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

SEXP getTable(const toml::table& tbl);

SEXP getValue(const toml::node& nod) {
    toml::node_type nodetype = nod.type();
    //Rcpp::Rcout << "Type of node is " << nodetype << std::endl;
    if (nodetype == toml::node_type::string) {
        std::string val{*nod.as_string()};
        return Rcpp::wrap(val);
    }
    else if (nodetype == toml::node_type::integer) {
        int64_t val{*nod.as_integer()};
        return Rcpp::wrap(val);
    }
    else if (nodetype == toml::node_type::floating_point) {
        double val{*nod.as_floating_point()};
        return Rcpp::wrap(val);
    }
    else if (nodetype == toml::node_type::boolean) {
        bool val = nod.as_boolean();
        return Rcpp::wrap(val);
    }
    else if (nodetype == toml::node_type::date) {
        const toml::date val{*nod.as_date()};
        Rcpp::Date d(val.year, val.month, val.day);
        return Rcpp::wrap(d);
    }
    else if (nodetype == toml::node_type::date_time) {
        const toml::date_time val{*nod.as_date_time()};
        std::stringstream s;    // because we have no Datetime ctor from components :-/
        s << val;
        //Rcpp::Rcout << "Val is " << s.str() << std::endl;
        Rcpp::Datetime d{s.str(), "%Y-%m-%dT%H:%M:%OS"};
        return Rcpp::wrap(d);
    }
    return R_NilValue;
}

SEXP getTable(const toml::table& tbl) {
    Rcpp::StretchyList sl;
    //tbl.for_each([ind,sl](const toml::key& key, auto&& val) {
    for (auto it = tbl.cbegin(); it != tbl.cend(); it++) {
        const toml::key& key = it->first;
        const toml::node& val = it->second;
        if (val.is_array_of_tables()) {
            Rcpp::Rcout << "is array of tables\n";
        } else if (val.is_table()) {
            //Rcpp::Rcout << "is table\n";
            sl.push_back(Rcpp::Named(key.data()) = getTable(*val.as_table()));
        } else if (val.is_array()) {
            //Rcpp::Rcout << "is array\n";
            //visitArray(*val.as_array(), ind + std::string("  "));
            //sl.push_back(getArray(*val.as_array()));
            sl.push_back(Rcpp::Named(key.data()) = getArray(*val.as_array()));
         } else if (val.is_value()) {
            //Rcpp::Rcout << "is value of type: " << val.type() << "\n";
            sl.push_back(Rcpp::Named(key.data()) = getValue(val));
        } else {
            Rcpp::Rcout << "unknown type in table: " << val.type() << "\n";
        }
    }
    return Rcpp::as<Rcpp::List>(sl);
}

SEXP collapsedList(Rcpp::List ll) {
    if (ll.length() == 0) return R_NilValue;
    Rcpp::List::iterator it = ll.begin();
    switch(TYPEOF(*it)) {
        case REALSXP: {
            Rcpp::NumericVector v(ll.begin(), ll.end());
            Rcpp::RObject ro = ll[0];
            if (ro.hasAttribute("class")) {
                Rcpp::CharacterVector cv = ro.attr("class");
                if ((cv.size() == 1) && std::string(cv[0]) == "Date") {
                    Rcpp::DateVector dv(v);
                    return dv;
                }
                if ((cv.size() == 2) && std::string(cv[1]) == "POSIXt") {
                    Rcpp::DatetimeVector dtv(v);
                    return dtv;
                }
            }
            return v;
            break;              // not reached ...
        }
        case INTSXP: {
            Rcpp::IntegerVector v(ll.begin(), ll.end());
            return v;
            break;              // not reached ...
        }
        case LGLSXP: {
            Rcpp::LogicalVector v(ll.begin(), ll.end());
            return v;
            break;              // not reached ...
        }
        case STRSXP: {              // minor code smell that this is different :-/
            int n = ll.size();
            Rcpp::CharacterVector v(n);
            for (int i=0; i<n; i++) {
                std::string s = Rcpp::as<std::string>(ll[i]);
                Rcpp::String se(s, CE_UTF8);
                v[i] = se;
            }
            return v;
            break;              // not reached ...
        }
    }
    return ll;
}

SEXP getArray(const toml::array& arr) {
    Rcpp::StretchyList sl;
    bool nonested = true;       // ie no embedded array
    //tbl.for_each([ind,sl](const toml::key& key, auto&& val) {
    for (auto it = arr.cbegin(); it != arr.cend(); it++) {
        const toml::node& val = *it;
        if (val.is_array()) {
            //Rcpp::Rcout << "is array (in array)\n";
            sl.push_back(getArray(*val.as_array()));
            nonested = false;
        } else if (val.is_value()) {
            sl.push_back(getValue(val));
        } else {
            Rcpp::Rcout << "unknown type in array: " << val.type() << "\n";
        }
    }
    if (nonested)
        return collapsedList(Rcpp::as<Rcpp::List>(sl));
    else
        return Rcpp::as<Rcpp::List>(sl);
}


//' Parse a TOML file
//'
//' @param input [character] TOML input, either as chracter value or path to TOML file
//' @param verbose [logical] Optional verbosity flag, no current effect
//' @param fromfile [logical] Optional with default value \sQuote{TRUE} indicating parsing from file
//' @param includize [logical] Optional legacy option, no current effect
//' @param escape [logical] Optiona legacy option, no current effect
//' @return A List with the parsed content
// [[Rcpp::export]]
Rcpp::List tomlparseImpl(const std::string input,
                         bool verbose=false,
                         bool fromfile=true,
                         bool includize=false,
                         bool escape=true) {

    if (includize == true)
        Rcpp::warning("The 'includize' option is included for legacy support and has no effect.");
    if (escape == false)
        Rcpp::warning("The 'escape' option is included for legacy support and has no effect.");

    const toml::table tbl = (fromfile) ? toml::parse_file(input) : toml::parse(input);

    //if (verbose) visitTable(tbl);

    Rcpp::StretchyList sl;
    //tbl.for_each([sl](const toml::key& key, auto&& val) {
    for (auto it = tbl.cbegin(); it != tbl.cend(); it++) {
        const toml::key& key = it->first;
        const toml::node& nod = it->second;
        //Rcpp::Rcout << key << " ";
        if (nod.is_array_of_tables()) {
            Rcpp::Rcout << key << " is array of tables\n";
            Rcpp::StretchyList l;
            const toml::array& arr = *tbl.get_as<toml::array>(key);
            for (auto ait = arr.cbegin(); ait != arr.cend(); ait++) {
                l.push_back(getTable(*ait->as_table()));
            }
            sl.push_back(Rcpp::Named(key.data()) = Rcpp::as<Rcpp::List>(l));
        } else if (nod.is_table()) {
            //Rcpp::Rcout << "is table\n";
            sl.push_back(Rcpp::Named(key.data()) = getTable(*nod.as_table()));
            //visitTable(*nod.as_table(), ind + std::string("  "));
        } else if (nod.is_array()) {
            //Rcpp::Rcout << "is array\n";
            //visitArray(*nod.as_array(), ind + std::string("  "));
            sl.push_back(Rcpp::Named(key.data()) = getArray(*nod.as_array()));
        } else if (nod.is_value()) {
            //Rcpp::Rcout << "is value of type: " << nod.type() << "\n";
            sl.push_back(Rcpp::Named(key.data()) = getValue(nod));
            //getValues
        } else {
            Rcpp::Rcout << "unknown type: " << nod.type() << "\n";
        }
    }

    return Rcpp::as<Rcpp::List>(sl);
}
