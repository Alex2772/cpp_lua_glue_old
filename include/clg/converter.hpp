//
// Created by alex2 on 02.05.2021.
//

#pragma once

#include "lua.hpp"
#include "exception.hpp"
#include "util.hpp"
#include <tuple>

namespace clg {

    static std::string any_to_string(lua_State* l, int n) {
        return lua_typename(l, lua_type(l, n));
    }

    namespace detail {
        static void throw_converter_error(lua_State* l, int n, const char* message) {
            throw clg_exception(any_to_string(l, n) + " is " + message);
        }
    }

    template<typename T>
    struct converter {
        static T from_lua(lua_State* l, int n) {
            if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
                if (lua_isinteger(l, n)) {
                    return static_cast<T>(lua_tointeger(l, n));
                }
                if (lua_isboolean(l, n)) {
                    return static_cast<T>(lua_toboolean(l, n));
                }
                if (lua_isnumber(l, n)) {
                    return static_cast<T>(lua_tonumber(l, n));
                }
                detail::throw_converter_error(l, n, "not a integer, nor boolean nor number");
            }
            throw clg_exception("unimplemented converter");
        }
        static int to_lua(lua_State* l, const T& v) {
            if constexpr (std::is_integral_v<T>) {
                if constexpr (std::is_same_v<T, bool>) {
                    lua_pushboolean(l, v);
                } else {
                    lua_pushinteger(l, v);
                }
                return 1;
            } else if constexpr (std::is_floating_point_v<T>) {
                lua_pushnumber(l, v);
                return 1;
            }
            throw clg_exception("unimplemented converter");
        }
    };

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
    template<>
    struct converter<void*> {
        static void* from_lua(lua_State* l, int n) {
            return nullptr;
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
            if (lua_isuserdata(l, n)) {
                return *reinterpret_cast<T**>(lua_touserdata(l, n));
            }
            detail::throw_converter_error(l, n, "not a userdata");
            return nullptr;
        }
        static int to_lua(lua_State* l, T* v) {
            auto classname = clg::class_name<T>();
            T** t = reinterpret_cast<T**>(lua_newuserdata(l, sizeof(T*)));
            *t = v;
            luaL_getmetatable(l, classname.c_str());
            if (lua_isnil(l, -1)) {
                lua_pop(l, 1);
            } else {
                lua_setmetatable(l, -2);
            }
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
        return converter<T>::from_lua(l, index);
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
            (std::apply)([&](Args... a) {
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