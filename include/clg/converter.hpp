//
// Created by alex2 on 02.05.2021.
//

#pragma once

#include "lua.hpp"
#include "exception.hpp"
#include <tuple>

namespace clg {

    static std::string any_to_string(lua_State* l, int n) {
        size_t len;
        auto c = luaL_tolstring(l, n, &len);
        return std::string(c, c + len);
    }

    template<typename T>
    struct converter {
        static T from_lua(lua_State* l, int n) {
            throw clg_exception("unimplemented converter");
        }
        static int to_lua(lua_State* l, const T&) {
            throw clg_exception("unimplemented converter");
        }
    };
    
    namespace detail {
        static void throw_converter_error(lua_State* l, int n, const char* message) {
            throw clg_exception(any_to_string(l, n) + " is " + message);
        }
    }

    template<typename T>
    struct converter_number {
        static T from_lua(lua_State* l, int n) {
            if (lua_isinteger(l, n)) {
                return static_cast<T>(lua_tointeger(l, n));
            }
            if (lua_isboolean(l, n)) {
                return static_cast<T>(lua_toboolean(l, n));
            }
            if (lua_isnumber(l, n)) {
                return static_cast<T>(lua_tonumber(l, n));
            }
            detail::throw_converter_error(l, n, "not an integer, number or boolean");
            throw;
        }
    };

    template<>
    struct converter<int>: converter_number<int>{
        static int to_lua(lua_State* l, int v) {
            lua_pushinteger(l, v);
            return 1;
        }
    };

    template<>
    struct converter<float>: converter_number<float>{
        static int to_lua(lua_State* l, float v) {
            lua_pushnumber(l, v);
            return 1;
        }
    };

    template<>
    struct converter<double>: converter<float>{};

    template<>
    struct converter<std::string> {
        static std::string from_lua(lua_State* l, int n) {
            if (!lua_isstring(l, n)) {
                detail::throw_converter_error(l, n, "not a string");
            }
            return lua_tostring(l, n);
        }
        static int to_lua(lua_State* l, const std::string& v) {
            lua_pushstring(l, v.c_str());
            return 1;
        }
    };
    template<>
    struct converter<const char*> {
        static const char* from_lua(lua_State* l, int n) {
            if (!lua_isstring(l, n)) {
                detail::throw_converter_error(l, n, "not a string");
            }
            return lua_tostring(l, n);
        }
        static int to_lua(lua_State* l, const char* v) {
            lua_pushstring(l, v);
            return 1;
        }
    };
    template<int N>
    struct converter<char[N]> {
        static const char* from_lua(lua_State* l, int n) {
            if (!lua_isstring(l, n)) {
                detail::throw_converter_error(l, n, "not a string");
            }
            return lua_tostring(l, n);
        }
        static int to_lua(lua_State* l, const char* v) {
            lua_pushstring(l, v);
            return 1;
        }
    };
    template<>
    struct converter<bool> {
        static bool from_lua(lua_State* l, int n) {
            if (!lua_isboolean(l, n)) {
                detail::throw_converter_error(l, n, "not a boolean");
            }
            return lua_toboolean(l, n);
        }
        static int to_lua(lua_State* l, bool v) {
            lua_pushboolean(l, v);
            return 1;
        }
    };

    template<>
    struct converter<std::nullptr_t> {
        static std::nullptr_t from_lua(lua_State* l, int n) {
            if (!lua_isnil(l, n)) {
                detail::throw_converter_error(l, n, "not a nil");
            }
            return nullptr;
        }
        static int to_lua(lua_State* l, std::nullptr_t v) {
            lua_pushnil(l);
            return 1;
        }
    };


    /**
     * userdata
     */
    template<typename T>
    struct converter<T*> {
        static T* from_lua(lua_State* l, int n) {
            if (!lua_islightuserdata(l, n)) {
                detail::throw_converter_error(l, n, "not a userdata");
            }
            return reinterpret_cast<T*>(lua_touserdata(l, n));
        }
        static int to_lua(lua_State* l, T* v) {
            lua_pushlightuserdata(l, v);
            return 1;
        }
    };

    template<typename T>
    static T get_from_lua(lua_State* l) {
        T t = converter<T>::from_lua(l, -1);
        lua_pop(l, 1);
        return t;
    }

    template<typename T>
    static T get_from_lua(lua_State* l, unsigned index) {
        return converter<T>::from_lua(l, index);;
    }

    /**
     * @tparam T
     * @param l
     * @param value
     * @return количество запушенных значений
     */
    template<typename T>
    static int push_to_lua(lua_State* l, const T& value) {
        return converter<T>::to_lua(l, value);
    }

    template<typename... Args>
    struct converter<std::tuple<Args...>> {
        static int to_lua(lua_State* l, const std::tuple<Args...>& v) {
            converter<std::tuple<Args...>> t(l);
            std::apply([&](Args... a) {
                t.push(std::forward<Args>(a)...);
            }, v);
            return sizeof...(Args);
        }

    private:
        lua_State* mState;

        converter(lua_State* state) : mState(state) {}

        template<typename Arg, typename... MyArgs>
        void push(Arg&& arg, MyArgs&&... args) {
            push_to_lua(mState, arg);

            push(std::forward<MyArgs>(args)...);
        }

        void push() {}
    };
}