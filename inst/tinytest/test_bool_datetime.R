
library(RcppTomlPlusPlus)

toml <- parseToml("bool_datetime.toml")

expect_equal(names(toml), c("boolean", "datetime"))

expect_true(toml$boolean$True)
expect_false(toml$boolean$False)

ref1 <- as.POSIXct("1979-05-27 07:32:00", tz="UTC", usetz=TRUE)
ref2 <- as.POSIXct("1979-05-27 07:32:00.999999", tz="UTC", usetz=TRUE)
expect_equal(toml$datetime$key1, ref1)
expect_equal(toml$datetime$key2, ref1)
expect_equal(toml$datetime$key3, ref2)