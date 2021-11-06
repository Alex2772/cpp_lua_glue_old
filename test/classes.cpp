
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(classes)

thread_local bool constructorCalled = false;
thread_local bool destructorCalled = false;
thread_local bool check = false;
thread_local bool checkAnimal = false;

class Person {
private:
    std::string mName;
    std::string mSurname;

public:
    Person(const std::string& name, const std::string& surname) : mName(name), mSurname(surname) {
        constructorCalled = true;
    }
    ~Person() {
        destructorCalled = true;
    }

    void simpleCall() {
        check = true;
        BOOST_CHECK_EQUAL(mName, "loh");
        BOOST_CHECK_EQUAL(mSurname, "bolotniy");
    }
    void simpleCallArgs(int a, int b) {
        check = true;
        BOOST_CHECK_EQUAL(mName, "loh");
        BOOST_CHECK_EQUAL(mSurname, "bolotniy");

        BOOST_CHECK_EQUAL(a, 228);
        BOOST_CHECK_EQUAL(b, 322);
    }
    int simpleCallRet(int a, int b) {
        check = true;
        BOOST_CHECK_EQUAL(mName, "loh");
        BOOST_CHECK_EQUAL(mSurname, "bolotniy");

        BOOST_CHECK_EQUAL(a, 228);
        BOOST_CHECK_EQUAL(b, 322);
        return 123;
    }

    static int doXor(int a, int b) {
        BOOST_CHECK_EQUAL(a, 1);
        BOOST_CHECK_EQUAL(b, 2);
        return a ^ b;
    }
};


class Animal {
private:
    std::string mName;

public:
    Animal(const std::string& name) : mName(name) {

    }

    void check() {
        BOOST_TEST(mName == "azaza");
        checkAnimal = true;
    }
};

BOOST_AUTO_TEST_CASE(class_name) {
    BOOST_CHECK_EQUAL("Person", clg::class_name<Person>());
}
BOOST_AUTO_TEST_CASE(constructor) {
    constructorCalled = false;
    clg::vm v;
    v.register_class<Person>()
            .constructor<std::string, std::string>();

    v.do_string<void>("print(type(Person))\nfor i in pairs(Person) do print(i)\n end\np = Person:new('loh', 'bolotniy')");
    BOOST_TEST(constructorCalled);
}

BOOST_AUTO_TEST_CASE(simpleCall) {
    constructorCalled = false;
    check = false;
    clg::vm v;
    v.register_class<Person>()
            .constructor<std::string, std::string>()
            .method<&Person::simpleCall>("simpleCall");

    v.do_string<void>("p = Person:new('loh', 'bolotniy')\np:simpleCall()");
    BOOST_TEST(constructorCalled);
    BOOST_TEST(check);
}

BOOST_AUTO_TEST_CASE(args) {
    constructorCalled = false;
    check = false;
    clg::vm v;
    v.register_class<Person>()
            .constructor<std::string, std::string>()
            .method<&Person::simpleCallArgs>("simpleCallArgs");

    v.do_string<void>("p = Person:new('loh', 'bolotniy')\np:simpleCallArgs(228, 322)");
    BOOST_TEST(constructorCalled);
    BOOST_TEST(check);
}

BOOST_AUTO_TEST_CASE(args_with_return) {
    constructorCalled = false;
    check = false;
    clg::vm v;
    v.register_function("check", [](int c) {
        BOOST_CHECK_EQUAL(c, 123);
    });
    v.register_class<Person>()
            .constructor<std::string, std::string>()
            .method<&Person::simpleCallRet>("simpleCallRet");

    v.do_string<void>("p = Person:new('loh', 'bolotniy')\ncheck(p:simpleCallRet(228, 322))");
    BOOST_TEST(constructorCalled);
    BOOST_TEST(check);
}

BOOST_AUTO_TEST_CASE(destructor) {
    constructorCalled = false;
    destructorCalled = false;
    {
        clg::vm v;
        v.register_class<Person>()
                .constructor<std::string, std::string>()
                .method<&Person::simpleCallRet>("simpleCallRet");

        v.do_string<void>("p = Person:new('loh', 'bolotniy')");
        BOOST_TEST(constructorCalled);
    }
    BOOST_TEST(destructorCalled);
}

BOOST_AUTO_TEST_CASE(return_class) {
    constructorCalled = false;
    check = false;
    clg::vm v;
    v.register_function("check", [](int c) {
        BOOST_CHECK_EQUAL(c, 123);
    });
    v.register_function("makePerson", []() {
        return new Person("loh", "bolotniy");
    });
    v.register_class<Person>()
            .method<&Person::simpleCallRet>("simpleCallRet");

    v.do_string<void>("p = makePerson()\ncheck(p:simpleCallRet(228, 322))");
    BOOST_TEST(constructorCalled);
    BOOST_TEST(check);
}
BOOST_AUTO_TEST_CASE(multiple_clases) {
    check = false;
    checkAnimal = false;

    clg::vm v;
    v.register_class<Person>()
            .constructor<std::string, std::string>()
            .method<&Person::simpleCall>("simpleCall");
    v.register_class<Animal>()
            .constructor<std::string>()
            .method<&Animal::check>("check");

    v.do_string<void>("p = Person:new('loh', 'bolotniy')\np:simpleCall()");
    BOOST_TEST(check);
    check = false;
    v.do_string<void>("a = Animal:new('azaza')\na:check()");
    BOOST_TEST(checkAnimal);
    BOOST_TEST(!check);
    checkAnimal = false;
    v.do_string<void>("p = Person:new('loh', 'bolotniy')\np:simpleCall()");
    BOOST_TEST(check);
    BOOST_TEST(!checkAnimal);
    v.do_string<void>("a = Animal:new('azaza')\na:check()");
    BOOST_TEST(checkAnimal);
}



BOOST_AUTO_TEST_CASE(staticMethod) {
    constructorCalled = false;
    check = false;
    clg::vm v;
    v.register_class<Person>()
            .staticFunction<Person::doXor>("doXor");

    int result = v.do_string<int>("return Person:doXor(1, 2)");
    BOOST_CHECK_EQUAL(result, (1 ^ 2));
}
BOOST_AUTO_TEST_SUITE_END()