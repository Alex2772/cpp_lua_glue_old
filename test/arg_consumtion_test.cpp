
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(arg_consumtion_test)

int func1(lua_State* s) {
    BOOST_CHECK_EQUAL(lua_gettop(s), 3);
    std::string str(lua_tostring(s, -3));
    BOOST_CHECK_EQUAL(str, "govno");
    lua_pop(s, 1);

    return 0;
}
int func2(lua_State* s) {
    BOOST_CHECK_EQUAL(lua_gettop(s), 1);
    std::string str(lua_tostring(s, -1));
    BOOST_CHECK_EQUAL(str, "kek");
    lua_pop(s, 4);

    return 0;
}

BOOST_AUTO_TEST_CASE(f1) {
    clg::vm v;
    lua_register(v, "func1", func1);
    lua_register(v, "func2", func2);
    BOOST_CHECK_NO_THROW(v.do_string<void>("func1('govno', 'garbage', 228)"));
    BOOST_CHECK_EQUAL(lua_gettop(v), 0);
    BOOST_CHECK_NO_THROW(v.do_string<void>("func2('kek')"));
    BOOST_CHECK_EQUAL(lua_gettop(v), 0);
}

BOOST_AUTO_TEST_SUITE_END()