//
// Created by alex2 on 02.05.2021.
//

#pragma once

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
#include <cstring>

namespace clg {

    class lua_exception: public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };


    template<typename T>
    struct converter {
        static T from_lua(lua_State* l, int n) {
            static_assert(0, "unimplemented!");
        }
    };

    template<>
    struct converter<int> {
        static int from_lua(lua_State* l, int n) {
            if (lua_isinteger(l, n)) {
                throw lua_exception("not an integer");
            }
            return lua_tointeger(l, n);
        }
    };
    template<>
    struct converter<int> {
        static int from_lua(lua_State* l, int n) {
            if (lua_isinteger(l, n)) {
                throw lua_exception("not an integer");
            }
            return lua_tointeger(l, n);
        }
    };

    template<typename T>
    static T get_from_lua(lua_State* l) {
        T t = converter<T>::from_lua(l, -1);
        lua_pop(l, 1);
        return t;
    }


    /**
     * Базовый интерфейс для работы с Lua. Не инициализирует Lua самостоятельно.
     */
    class interface {
    private:
        lua_State* mState;

    public:
        explicit interface(lua_State* state) : mState(state) {}

        void register_function(const std::string& name, int(* function)(lua_State* s)) {
            lua_register(mState, name.c_str(), function);
        }

        template<typename ReturnType>
        int do_string(const std::string& exec) {
            luaL_dostring(mState, exec.c_str());
            return get_from_lua<ReturnType>(mState);
        }

        operator lua_State*() const {
            return mState;
        }
    };

    /**
     * В отличии от interface, этот класс сам создаёт виртуальную машину Lua, загружает базовые библиотеки и отвечает за
     * её освобождение.
     */
    class vm: public interface {
    public:
        vm(): interface(luaL_newstate()) {
            luaL_openlibs(*this);
        }
        ~vm() {
            lua_close(*this);
        }
    };
}