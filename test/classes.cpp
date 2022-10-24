
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>
#include <set>

BOOST_AUTO_TEST_SUITE(classes)

thread_local bool constructorCalled = false;
thread_local bool destructorCalled = false;
thread_local bool check = false;
thread_local bool checkAnimal = false;
    thread_local std::set<int> destroyedObjects;

    struct Object {
    public:
        void check1() {
            BOOST_CHECK_EQUAL(mString, "hello");
            mString = "world";
        }
        void check2() {
            BOOST_CHECK_EQUAL(mString, "world");
            mString = "!";
        }

        [[nodiscard]]
        const std::string& string() const noexcept {
            return mString;
        }
    private:
        std::string mString = "hello";
    };
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

    class SomeClass {
    private:
        int mValue;

    public:
        SomeClass(int value) : mValue(value) {}

        ~SomeClass() {
            destroyedObjects.insert(mValue);
        }

        int getValue() {
            return mValue;
        }

        static std::shared_ptr<SomeClass>& get228() {
            static auto s = std::make_shared<SomeClass>(228);
            return s;
        }
        static std::shared_ptr<SomeClass>& get322() {
            static auto s = std::make_shared<SomeClass>(322);
            return s;
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
        return std::make_shared<Person>("loh", "bolotniy");
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

        {
            bool result = v.do_string<bool>(R"(
p1 = Person:new('loh', 'bolotniy')
p2 = Person:new('loh', 'bolotniy')
print("p1: "..p1)
print("p2: "..p2)
return p1 == p2
)");
            BOOST_TEST(!result);
        }

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
BOOST_AUTO_TEST_CASE(builder_methods) {

    clg::vm v;


    v.register_class<Object>()
            .constructor<>()
            .builder_method<&Object::check1>("check1")
            .builder_method<&Object::check2>("check2")
                    ;
    auto obj = v.do_string<std::shared_ptr<Object>>("return Object:new():check1():check2()");
    BOOST_CHECK_EQUAL(obj->string(), "!");
}

BOOST_AUTO_TEST_CASE(same_object) {

    {
        check = false;
        checkAnimal = false;
        destructorCalled = false;

        clg::vm v;

        v.register_class<SomeClass>()
                .staticFunction<&SomeClass::get228>("get228")
                .staticFunction<&SomeClass::get322>("get322")
                .method<&SomeClass::getValue>("getValue");

        {
            bool result = v.do_string<bool>(R"(
v1 = SomeClass.get228()
v2 = SomeClass.get228()
print("v1: "..v1)
print("v2: "..v2)
return v1 == v2;
)");
            BOOST_TEST(result);
        }
        {
            bool result = v.do_string<bool>(R"(
v1 = SomeClass.get228()
v2 = SomeClass.get228()
return v1:getValue() == v2:getValue();
)");
            BOOST_TEST(result);
        }

        {
            bool result = v.do_string<bool>(R"(
v1 = SomeClass.get228()
v2 = SomeClass.get228()
return v1 ~= v2
)");
            BOOST_TEST(!result);
        }
        {
            bool result = v.do_string<bool>(R"(
v1 = SomeClass.get228()
v2 = SomeClass.get322()
return v1 ~= v2
)");
            BOOST_TEST(result);
        }

        // destroy only 228; reference should be kept
        SomeClass::get228() = nullptr;
        BOOST_TEST(destroyedObjects.empty());
    }

    // 228 should be destroyed
    BOOST_TEST((destroyedObjects.find(228) != destroyedObjects.end()));

    // 322 should be kept since it is still alive in c++
    BOOST_TEST((destroyedObjects.find(322) == destroyedObjects.end()));

    // now destroy 322
    SomeClass::get322() = nullptr;

    // check both are destroyed
    BOOST_CHECK_EQUAL(destroyedObjects.size(), 2);
}



BOOST_AUTO_TEST_CASE(staticMethod) {
    constructorCalled = false;
    check = false;
    clg::vm v;
    v.register_class<Person>()
            .staticFunction<Person::doXor>("doXor");

    int result = v.do_string<int>("return Person.doXor(1, 2)");
    BOOST_CHECK_EQUAL(result, (1 ^ 2));
}

struct IName: clg::allow_lua_inheritance {
public:
    virtual std::string name() = 0;

    std::string callNameFromBaseClass() {
        return name();
    }
};

struct NameHello: IName {
public:
    std::string name() override {
        return "hello";
    }

    static std::shared_ptr<NameHello> make() {
        return std::make_shared<NameHello>();
    }
};

BOOST_AUTO_TEST_CASE(inheritance_cpp) {
    clg::vm v;

    v.register_class<IName>()
            .method<&IName::name>("name");

    v.register_class<NameHello>()
            .staticFunction<NameHello::make>("make");

    v.register_function("getName", [](const std::shared_ptr<IName>& n) {
        return n->name();
    });

    auto result = v.do_string<std::string>("return getName(NameHello.make())");
    BOOST_CHECK_EQUAL(result, "hello");
}

BOOST_AUTO_TEST_CASE(inheritance_lua) {
    clg::vm v;

    v.register_class<IName>()
            .method<&IName::name>("name")
            .method<&IName::callNameFromBaseClass>("callNameFromBaseClass");

    v.register_class<NameHello>()
            .staticFunction<NameHello::make>("make");

    auto result = v.do_string<std::string>("return NameHello.make():callNameFromBaseClass()");
    BOOST_CHECK_EQUAL(result, "hello");
}

struct ClassIsMetatable: clg::lua_self, std::enable_shared_from_this<ClassIsMetatable> {
public:
    void callCallback(clg::function c) {
        BOOST_TEST(c.call<bool>(shared_from_this()));
    }
};

BOOST_AUTO_TEST_CASE(class_is_metatable) {
    clg::vm v;
    v.register_class<ClassIsMetatable>()
            .constructor<>()
            .method<&ClassIsMetatable::callCallback>("callCallback")
            ;

    auto result = v.do_string<bool>(R"(
c = ClassIsMetatable:new()
c.test = true
test = false
c:callCallback(function(self)
  test = self.test
  return true
end)
return test
)");
    BOOST_TEST(result);
}


BOOST_AUTO_TEST_CASE(class_compare) {
    clg::vm v;
    v.register_class<ClassIsMetatable>()
            .constructor<>()
            .method<&ClassIsMetatable::callCallback>("callCallback")
            ;

    auto result = v.do_string<bool>(R"(
c = ClassIsMetatable:new()
c.test = true
test = false
c:callCallback(function(self)
  test = self.test
  return c == self
end)
return test
)");
    BOOST_TEST(result);
}


thread_local bool memleak_destructor_called = false;

struct Memleak: clg::lua_self, std::enable_shared_from_this<Memleak> {
public:
    ~Memleak() {
        memleak_destructor_called = true;
    }

    const std::string& test() {
        return str;
    }
private:
    std::string str = "hello";
};
BOOST_AUTO_TEST_CASE(memleak1) {
    memleak_destructor_called = false;
    clg::vm v;
    v.register_class<Memleak>()
            .constructor<>()
            ;

    v.do_string<>(R"(
c = Memleak:new()
c = nil
collectgarbage();
)");
    BOOST_TEST(memleak_destructor_called);
}

BOOST_AUTO_TEST_CASE(memleak2) {
    memleak_destructor_called = false;
    clg::vm v;
    v.register_class<Memleak>()
            .constructor<>()
            .method<&Memleak::test>("test")
            ;

    v.do_string<>(R"(
function func(obj)
  return obj:test()
end
)");
    auto obj = std::make_shared<Memleak>();
    BOOST_CHECK_EQUAL(v.global_function("func").call<std::string>(obj), "hello");
    v.collectGarbage();
    BOOST_TEST(!memleak_destructor_called);
    obj = nullptr;
    BOOST_TEST(memleak_destructor_called);
}

BOOST_AUTO_TEST_CASE(memleak3) {
    memleak_destructor_called = false;
    clg::vm v;
    v.register_class<Memleak>()
            .constructor<>()
            .method<&Memleak::test>("test")
            ;

    v.do_string<>(R"(
function func(obj)
  _G['obj'] = obj
  return obj:test()
end
)");
    auto obj = std::make_shared<Memleak>();
    BOOST_CHECK_EQUAL(v.global_function("func").call<std::string>(obj), "hello");
    v.collectGarbage();
    BOOST_TEST(!memleak_destructor_called);

    obj = nullptr;
    v.collectGarbage();
    BOOST_TEST(!memleak_destructor_called);

    v.do_string<>(R"(
_G['obj'] = nil
)");
    v.collectGarbage();
    BOOST_TEST(memleak_destructor_called);
}


struct SelfAssign: clg::lua_self {
public:
    void performAssign() {
        self()["test"] = "hello";
    }
};
BOOST_AUTO_TEST_CASE(self_assign) {
    clg::vm v;
    v.register_class<SelfAssign>()
            .constructor<>()
            .method<&SelfAssign::performAssign>("performAssign")
                    ;

    BOOST_CHECK_EQUAL(v.do_string<std::string>(R"(
v = SelfAssign:new()
v:performAssign()
return v.test
)"), "hello");
}



struct SpecificOverload {
public:
    void call(int) {
        called = 1;
    }

    void call(double) {
        called = 2;
    }

    int called = 0;
};

BOOST_AUTO_TEST_CASE(specific_overload) {
    clg::vm v;
    v.register_class<SpecificOverload>()
            .method<clg::select_overload<int>::of(&SpecificOverload::call)>("call1")
            .method<clg::select_overload<double>::of(&SpecificOverload::call)>("call2")
                    ;

    auto obj = std::make_shared<SpecificOverload>();
    v.set_global_value("obj", obj);

    BOOST_CHECK_EQUAL(obj->called, 0);

    v.do_string<>(R"(
obj:call1(0)
)");
    BOOST_CHECK_EQUAL(obj->called, 1);
    v.do_string<>(R"(
obj:call2(0)
)");
    BOOST_CHECK_EQUAL(obj->called, 2);
}

BOOST_AUTO_TEST_SUITE_END()