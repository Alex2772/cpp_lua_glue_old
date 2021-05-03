
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(basic)

BOOST_AUTO_TEST_CASE(basic) {
    clg::vm v;
    auto x = v.do_string<int>("return 4 + 2");
    BOOST_CHECK_EQUAL(x, 6);
}

BOOST_AUTO_TEST_SUITE_END()