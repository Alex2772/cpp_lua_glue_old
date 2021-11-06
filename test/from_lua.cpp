
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(from_lua)

BOOST_AUTO_TEST_CASE(return_integer) {
    clg::vm v;
    auto x = v.do_string<int>("return 4 + 2");
    BOOST_CHECK_EQUAL(x, 6);
}
BOOST_AUTO_TEST_CASE(return_float) {
    clg::vm v;
    auto x = v.do_string<float>("return 4.2 + 7.5");
    BOOST_CHECK_CLOSE(x, 11.7f, 0.01f);
}
BOOST_AUTO_TEST_CASE(return_string) {
    clg::vm v;
    auto x = v.do_string<std::string>("return \"call\"");
    BOOST_CHECK_EQUAL(x, "call");
}
BOOST_AUTO_TEST_CASE(return_cstring) {
    clg::vm v;
    auto x = v.do_string<const char*>("return \"azaza\"");
    BOOST_CHECK_EQUAL(std::string(x), "azaza");
}
BOOST_AUTO_TEST_CASE(return_bool) {
    clg::vm v;
    auto x = v.do_string<bool>("return true");
    BOOST_TEST(x);
    x = v.do_string<bool>("return false");
    BOOST_TEST(!x);
}
BOOST_AUTO_TEST_CASE(return_table1) {
    clg::vm v;
    auto x = v.do_string<clg::table>("return { ['x'] = 'a', ['y'] = 'b' }");
    BOOST_CHECK_EQUAL(std::get<std::string>(x["x"]), "a");
    BOOST_CHECK_EQUAL(std::get<std::string>(x["y"]), "b");
}

BOOST_AUTO_TEST_CASE(return_table2) {
    clg::vm v;
    auto x = v.do_string<clg::table>("return { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }").toArray();
    std::vector<clg::value> array = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    BOOST_TEST(x == array);
}

BOOST_AUTO_TEST_SUITE_END()