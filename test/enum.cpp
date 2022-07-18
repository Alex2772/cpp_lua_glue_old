
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(enums)

enum MyEnum {
    kValue1,
    kValue2,
    kValue3,
};

BOOST_AUTO_TEST_CASE(from_lua) {
    clg::vm v;
    v.register_enum<MyEnum>();
    BOOST_CHECK_EQUAL(v.do_string<MyEnum>("return MyEnum.kValue2"), MyEnum::kValue2);
}

BOOST_AUTO_TEST_CASE(to_lua) {
    clg::vm v;
    v.register_enum<MyEnum>();

    v.do_string("function test(e) return e == MyEnum.kValue3 end");
    BOOST_TEST(v.global_function("test").call<bool>(MyEnum::kValue3));
}


BOOST_AUTO_TEST_SUITE_END()