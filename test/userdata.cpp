
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(userdata)

struct user {
    int id;
    std::string name;
};

BOOST_AUTO_TEST_CASE(check1) {
    user u;
    u.id = 228;
    u.name = "alesha123";

    clg::vm v;
    v.do_string<void>("function govno(a) return a end");

    auto returned = v["govno"].call<user*>(&u);
    BOOST_CHECK_EQUAL(returned, &u);
}

int get_id(user* u) {
    return u->id;
}

BOOST_AUTO_TEST_CASE(check2) {
    user u;
    u.id = 228;
    u.name = "alesha123";

    clg::vm v;

    v.register_function<get_id>("get_id");
    v.do_string<void>("function govno(a) return 'id:'..get_id(a) end");

    auto returned = v["govno"].call<std::string>(&u);
    BOOST_CHECK_EQUAL(returned, "id:228");
}

BOOST_AUTO_TEST_SUITE_END()