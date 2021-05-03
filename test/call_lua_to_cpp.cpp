
#include <boost/test/unit_test.hpp>
#include <clg/clg.hpp>

BOOST_AUTO_TEST_SUITE(call_lua_to_cpp)


bool called;
void func_call() {
    called = true;
}

BOOST_AUTO_TEST_CASE(call) {
    clg::vm v;
    v.register_function<func_call>("call");
    v.do_string<void>("call()");

    BOOST_TEST(called);
}

int func_call_return() {
    return 123;
}

BOOST_AUTO_TEST_CASE(call_return) {
    clg::vm v;
    v.register_function<func_call_return>("call");
    int result = v.do_string<int>("return call()");

    BOOST_CHECK_EQUAL(result, 123);
}

int func_call_return_args1(int a, int b) {
    return a + b;
}

BOOST_AUTO_TEST_CASE(call_return_args1) {
    clg::vm v;
    v.register_function<func_call_return_args1>("call");
    int result = v.do_string<int>("return call(3, 5)");

    BOOST_CHECK_EQUAL(result, 8);
}

int func_call_return_args2(int a, int b) {
    return a - b;
}

BOOST_AUTO_TEST_CASE(call_return_args2) {
    clg::vm v;
    v.register_function<func_call_return_args2>("call");
    int result = v.do_string<int>("return call(12, 3)");

    BOOST_CHECK_EQUAL(result, 9);
}

std::string func_call_return_args3(const std::string& hello) {
    return hello + " world";
}

BOOST_AUTO_TEST_CASE(call_return_args3) {
    clg::vm v;
    v.register_function<func_call_return_args3>("call");
    std::string result = v.do_string<std::string>("return call(\"hello\")");

    BOOST_CHECK_EQUAL(result, "hello world");
}

std::string func_call_return_args4_a(const std::string& hello) {
    return hello + " world";
}
std::string func_call_return_args4_b(const std::string& hello) {
    return ", " + hello + "!";
}

BOOST_AUTO_TEST_CASE(call_multiple_functions) {
    clg::vm v;
    v.register_function<func_call_return_args4_a>("call_a");
    v.register_function<func_call_return_args4_b>("call_b");
    std::string result = v.do_string<std::string>("return call_a(\"hello\")..call_b(\"loshara\")");

    BOOST_CHECK_EQUAL(result, "hello world, loshara!");
}

/*
 *
 * Если в GetChaAttr передать 3 аргумента, то функция возвращает два результата
 * local result1, result2 = GetChaAttr(role, x1, x2)
 *
 * Если в GetChaAttr передать 2 аргумента, то функция возвращает один результат
 * local result1 = GetChaAttr(role, x1)
 *
 */

std::tuple<int, int> func_return_1_2() {
    return { 1, 2 };
}

BOOST_AUTO_TEST_CASE(call_return_multiple) {
    clg::vm v;
    v.register_function<func_return_1_2>("call");
    std::string result = v.do_string<std::string>(
            "local a,b = call()\n"
            "return a..b"
            );


    // BickerNotice(role , "Фея уже имеет такой навык. Использование невозможно. " )

    BOOST_CHECK_EQUAL(result, "12");
}


/*
 * lua_pushlightuserdata
inline int lua_BickerNotice(lua_State* L)
{
	BOOL bValid = (lua_gettop(L) == 2 && lua_islightuserdata(L, 1) && lua_isstring(L, 2));
	if (!bValid)
	{
		E_LUAPARAM;
		return 0;
	}

	CCharacter* pChar = (CCharacter*)lua_touserdata(L, 1);
	const char* pszData = lua_tostring(L, 2);
	if (!pChar || !pszData)
	{
		E_LUANULL;
		return 0;
	}

	pChar->BickerNotice(pszData);
	return 0;
}

 *
 *
 *
void CCharacter::BickerNotice(const char szData[], ...)
{
	// Modify by lark.li 20080801 begin
	char szTemp[250];
	memset(szTemp, 0, sizeof(szTemp));
	va_list list;
	va_start(list, szData);
	_vsnprintf(szTemp, sizeof(szTemp) - 1, szData, list);
	//vsprintf( szTemp, szData, list );
	// End
	va_end(list);

	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_BICKER_NOTICE);
	WRITE_STRING(packet, szTemp);

	this->ReflectINFof(this, packet);
}
 *
 */

BOOST_AUTO_TEST_SUITE_END()