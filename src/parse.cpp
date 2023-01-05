
#include <Rcpp/Lightest>

#include <toml++/toml.h>

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

SEXP getValue(const toml::node& nod) {
    Rcpp::Rcout << "Type of node is " << nod.type() << std::endl;
    return R_NilValue;
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
        Rcpp::Rcout << key << " ";
        if (nod.is_array_of_tables()) {
            Rcpp::Rcout << "is array of tables\n";
        } else if (nod.is_table()) {
            Rcpp::Rcout << "is table\n";
            //visitTable(*nod.as_table(), ind + std::string("  "));
        } else if (nod.is_array()) {
            Rcpp::Rcout << "is array\n";
            //visitArray(*nod.as_array(), ind + std::string("  "));
        } else if (nod.is_value()) {
            Rcpp::Rcout << "is value of type: " << nod.type() << "\n";
            sl.push_back(Rcpp::Named(key.data()) = getValue(nod));
            //getValues
        } else {
            Rcpp::Rcout << "unknown type: " << nod.type() << "\n";
        }
    }

    return Rcpp::as<Rcpp::List>(sl);
}
