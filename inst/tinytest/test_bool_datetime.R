
library(RcppTomlPlusPlus)

toml <- parseToml("bool_datetime.toml")

expect_equal(names(toml), c("boolean", "date", "datetime", "time"))

expect_true(toml$boolean$True)
expect_false(toml$boolean$False)

ref1 <- as.POSIXct("1979-05-27 07:32:00", tz="UTC", usetz=TRUE)
ref2 <- as.POSIXct("1979-05-27 07:32:00.999999", tz="UTC", usetz=TRUE)
#ref3 <- as.POSIXct("1979-05-27 07:32:00")
expect_equal(toml$datetime$key1, ref1)
expect_equal(toml$datetime$key2, ref1)
expect_equal(toml$datetime$key3, ref2)
#expect_equal(toml$datetime$key4, ref3)

d1 <- as.Date("1979-05-27")
expect_equal(toml$date$dat1, d1)

t1 <- "07:32:00"
t2 <- "00:32:00.999999"
expect_equal(toml$time$time1, t1)
expect_equal(toml$time$time2, t2)
