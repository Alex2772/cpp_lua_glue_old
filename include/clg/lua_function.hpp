//
// Created by Alex2772 on 10/31/2021.
//

#pragma once

#include "lua.hpp"
#include <variant>
#include "converter.hpp"

namespace clg {
    class lua_function {
        friend class state_interface;
        template<typename T>
        friend struct clg::converter;

    private:
        lua_State* mClg;
        /**
         * <dl>
         *  <dt><code><b>std::string</b></code> = название функции (см. <a href="#clg::vm::operator[]">clg::vm::operator[]</a>)</dt>
         *  <dt><code><b>int</b></code> = индекс функции в стеке (для калбеков)</dt>
         * </dl>
         */
        std::variant<std::string, int> mName;

        lua_function(const std::string& name, lua_State* clg) : mName(name), mClg(clg) {}
        lua_function(int name, lua_State* clg) : mName(name), mClg(clg) {}


        template<typename Arg, typename... Args>
        void push(Arg&& arg, Args&& ... args) {
            push_to_lua(mClg, std::forward<Arg>(arg));

            push(std::forward<Args>(args)...);
        }

        void push() {}

        void push_function_to_be_called() {
            if (std::holds_alternative<std::string>(mName)) {
                lua_getglobal(mClg, std::get<std::string>(mName).c_str());
                if (lua_isnil(mClg, -1)) {
                    lua_pop(mClg, 1);
                    throw lua_exception("function " + std::get<std::string>(mName) + " is not defined");
                }
            } else {
                lua_topointer(mClg, 1);
            }
        }

        void do_call(unsigned args, int results) {
            if (lua_pcall(mClg, args, results, 0)) {
                try {
                    throw lua_exception("failed to call " + std::get<std::string>(mName) + ": " + get_from_lua<std::string>(mClg));
                } catch (...) {
                    throw lua_exception(get_from_lua<std::string>(mClg));
                }
            }
        }

    public:
        lua_function() = default;

        template<typename... Args>
        void operator()(Args&& ... args) {
            push_function_to_be_called();
            push(std::forward<Args>(args)...);
            do_call(sizeof...(args), 0);
        }

        template<typename Return, typename... Args>
        Return call(Args&& ... args) {
            push_function_to_be_called();
            push(std::forward<Args>(args)...);

            if constexpr (std::is_same_v < Return, dynamic_result >) {
                do_call(sizeof...(args), LUA_MULTRET);
                return get_from_lua<Return>(mClg);
            } else if constexpr (std::is_same_v < Return, void >) {
                do_call(sizeof...(args), 0);
            } else {
                do_call(sizeof...(args), 1);
                return get_from_lua<Return>(mClg);
            }
        }
    };
}