
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(to_lua)

BOOST_AUTO_TEST_CASE(to_integer) {
    clg::vm v;
    clg::push_to_lua(v, 228);
    BOOST_CHECK_EQUAL(clg::get_from_lua<int>(v), 228);
}
BOOST_AUTO_TEST_CASE(to_float) {
    clg::vm v;
    clg::push_to_lua(v, 11.7f);
    auto x = clg::get_from_lua<float>(v);
    BOOST_CHECK_CLOSE(x, 11.7f, 0.01f);
}
BOOST_AUTO_TEST_CASE(to_string) {
    clg::vm v;
    clg::push_to_lua(v, std::string("zhopa"));
    BOOST_CHECK_EQUAL(clg::get_from_lua<std::string>(v), "zhopa");
}
BOOST_AUTO_TEST_CASE(to_cstring) {
    clg::vm v;
    clg::push_to_lua(v, "zhopa");
    BOOST_CHECK_EQUAL(clg::get_from_lua<std::string>(v), "zhopa");
}
BOOST_AUTO_TEST_CASE(to_bool) {
    clg::vm v;
    clg::push_to_lua(v, true);
    BOOST_CHECK_EQUAL(clg::get_from_lua<bool>(v), true);
    clg::push_to_lua(v, false);
    BOOST_CHECK_EQUAL(clg::get_from_lua<bool>(v), false);
}

BOOST_AUTO_TEST_SUITE_END()