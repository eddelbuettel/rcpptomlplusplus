
#include <Rcpp/Lightest>
#include <toml++/toml.h>

// cf 'man timegm' for the workaround on non-Linux systems
inline time_t local_timegm(struct tm *tm) {
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // and there may be more OSs that have timegm() ...
    return timegm(tm);
#elif defined(__MINGW32__) || defined(__MINGW64__)
    return Rcpp::mktime00(*tm);  // Rcpp exports a copy of the R-internal function
#else
    char *tz = getenv("TZ");
    if (tz) tz = strdup(tz);
    setenv("TZ", "", 1);
    tzset();
    time_t ret = mktime(tm);
    if (tz) {
        setenv("TZ", tz, 1);
        free(tz);
    } else
        unsetenv("TZ");
    tzset();
    return ret;
#endif
}
// use 'format(z, tz="UTC", usetz=TRUE)' to see it in UTC
// [[Rcpp::export]]
Rcpp::Datetime withutc() {
    const toml::date date{2023,1,6};
    const toml::time time{1,2,3,123456789};
    const std::optional<toml::time_offset> offset;

    std::tm tm{};
    tm.tm_year = date.year - 1900;
    tm.tm_mon = date.month - 1;
    tm.tm_mday = date.day;
    tm.tm_hour = time.hour;
    tm.tm_min = time.minute;
    tm.tm_sec = time.second;
    //tm.tm_isdst = 1; // not filled
    time_t tt = local_timegm(&tm); // helper also used earlier in RcppTOML
    tt = tt - offset->minutes*60;
    Rcpp::DatetimeVector dt(1, "UTC"); // we always return UTC
    dt[0] =  tt + time.nanosecond * 1.0e-9;
    return Rcpp::wrap(dt);

}

// [[Rcpp::export]]
Rcpp::Datetime withoututc() {
    const toml::date date{2023,1,6};
    const toml::time time{1,2,3,123456789};
    const std::optional<toml::time_offset> offset;

    std::tm tm{};
    tm.tm_year = date.year - 1900;
    tm.tm_mon = date.month - 1;
    tm.tm_mday = date.day;
    tm.tm_hour = time.hour;
    tm.tm_min = time.minute;
    tm.tm_sec = time.second;
    //tm.tm_isdst = 1; // not filled
    time_t tt = local_timegm(&tm); // helper also used earlier in RcppTOML
    tt = tt - offset->minutes*60;
    Rcpp::DatetimeVector dt(1);
    dt[0] =  tt + time.nanosecond * 1.0e-9;
    return Rcpp::wrap(dt);

}
