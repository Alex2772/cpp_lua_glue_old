
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(call_cpp_to_lua)

BOOST_AUTO_TEST_CASE(call_return) {
    clg::vm v;
    v.do_string<void>("function get322() return 322 end");
    int x = v.global_function("get322").call<int>();
    BOOST_CHECK_EQUAL(x, 322);
}

BOOST_AUTO_TEST_CASE(call_return_one_arg) {
    clg::vm v;
    v.do_string<void>("function plus2(a) return a + 2 end");
    int x = v.global_function("plus2").call<int>(228);
    BOOST_CHECK_EQUAL(x, 230);
}

BOOST_AUTO_TEST_CASE(call_return_multiple_args) {
    clg::vm v;
    v.do_string<void>("function sub_plus_3(a, b) return a - b + 3 end");
    int x = v.global_function("sub_plus_3").call<int>(228, 322);
    BOOST_CHECK_EQUAL(x, -91);
}
BOOST_AUTO_TEST_CASE(call_mutiple_return_multiple_args) {
    clg::vm v;
    v.do_string<>("function get(a) if a == 1 then return 1, 'govno' else return nil end end");
    //v.do_string<>("function get(a) return 1, \"govno\" end");

    auto check = [](const clg::dynamic_result& r) {
        auto s = r.size();
        switch (s) {
            case 2:
                BOOST_TEST(!r.is_nil(0));
                BOOST_TEST(!r.is_nil(1));

                BOOST_CHECK_EQUAL(r.get<int>(0), 1);
                BOOST_CHECK_EQUAL(r.get<std::string>(1), "govno");
                break;

            case 1:
                BOOST_TEST(r.is_nil(0));
                break;

            default:

                BOOST_FAIL("dynamic_result has an incorrect size! " + std::to_string(s));
        }
    };

    check(v.global_function("get").call<clg::dynamic_result>(0));
    check(v.global_function("get").call<clg::dynamic_result>(1));



}
BOOST_AUTO_TEST_SUITE_END()