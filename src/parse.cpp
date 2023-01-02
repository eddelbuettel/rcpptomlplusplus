
#include <Rcpp/Lightest>

#include <toml++/toml.h>

void visitTable(const toml::table& tbl, const std::string& ind = "") {
    tbl.for_each([ind](const toml::key& key, auto&& val) {
        Rcpp::Rcout << ind << key << " ";
        if (val.is_array_of_tables()) {
            Rcpp::Rcout << "is array of tables\n";
        }
        if (val.is_table()) {
            Rcpp::Rcout << "is table\n";
            visitTable(*val.as_table(), ind + std::string("  "));
        }
        if (val.is_array()) {
            Rcpp::Rcout << "is array\n";
        }
        if (val.is_value()) {
            Rcpp::Rcout << "is value: " << val << "\n";
        }
    });
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

    visitTable(tbl);

    Rcpp::StretchyList sl;
    return Rcpp::as<Rcpp::List>(sl);
}
