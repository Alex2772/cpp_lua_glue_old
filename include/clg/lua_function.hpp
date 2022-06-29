//
// Created by Alex2772 on 10/31/2021.
//

#pragma once

#include "lua.hpp"
#include <utility>
#include <variant>
#include "converter.hpp"
#include "ref.hpp"

namespace clg {
    class lua_function {
        friend class state_interface;
        template<typename T>
        friend struct clg::converter;


    public:
        lua_function(lua_State* lua, ref ref) : mLua(lua), mRef(std::move(ref)) {
            push_function_to_be_called();
            assert(lua_isfunction(lua, -1));
            lua_pop(lua, 1);
        }
        lua_function() = default;
        lua_function(lua_function&& rhs) noexcept: mLua(rhs.mLua), mRef(std::move(rhs.mRef)) {}
        lua_function(const lua_function& rhs): mLua(rhs.mLua), mRef(rhs.mRef) {}

        lua_function& operator=(lua_function&& rhs) noexcept {
            mLua = rhs.mLua;
            mRef = std::move(rhs.mRef);
            return *this;
        }
        lua_function& operator=(std::nullptr_t rhs) noexcept {
            mRef = nullptr;
            return *this;
        }

        template<typename... Args>
        void operator()(Args&& ... args) {
            push_function_to_be_called();
            push(std::forward<Args>(args)...);
            do_call(sizeof...(args), 0);
        }

        template<typename Return, typename... Args>
        Return call(Args&& ... args) {
            stack_integrity stack(mLua);
            push_function_to_be_called();
            push(std::forward<Args>(args)...);

            if constexpr (std::is_same_v < Return, dynamic_result >) {
                do_call(sizeof...(args), LUA_MULTRET);
                return get_from_lua<Return>(mLua);
            } else if constexpr (std::is_same_v < Return, void >) {
                do_call(sizeof...(args), 0);
            } else {
                do_call(sizeof...(args), 1);
                return get_from_lua<Return>(mLua);
            }
        }

        lua_State* mLua;
        clg::ref mRef;

        lua_function(clg::ref name, lua_State* lua) : mRef(std::move(name)), mLua(lua) {}

        template<typename Arg, typename... Args>
        void push(Arg&& arg, Args&& ... args) {
            push_to_lua(mLua, std::forward<Arg>(arg));

            push(std::forward<Args>(args)...);
        }

        void push() {}

        void push_function_to_be_called() {
            mRef.push_value_to_stack();
        }

        void do_call(unsigned args, int results) {
            if (lua_pcall(mLua, args, results, 0)) {
                try {
                    auto name = any_to_string(mLua);
                    throw lua_exception("failed to call " + name + ": " + any_to_string(mLua));
                } catch (...) {
                    throw lua_exception(get_from_lua<std::string>(mLua));
                }
            }
        }
    };

    template<>
    struct converter<clg::lua_function> {
        static clg::lua_function from_lua(lua_State* l, int n) {
            lua_pushvalue(l, n);
            return { l, clg::ref::from_stack(l) };
        }
        static int to_lua(lua_State* l, const clg::ref& ref) {
            ref.push_value_to_stack();
            return 1;
        }
    };

}