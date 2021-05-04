
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(error)

BOOST_AUTO_TEST_CASE(do_string) {
    clg::vm v;
    BOOST_CHECK_NO_THROW(v.do_string<int>("return 4 + 2"));
    BOOST_CHECK_THROW(v.do_string<int>("retudrn 4 + 2"), clg::lua_exception);
    BOOST_CHECK_THROW(v.do_string<void>("retudrn 4 + 2"), clg::lua_exception);
}

BOOST_AUTO_TEST_SUITE_END()