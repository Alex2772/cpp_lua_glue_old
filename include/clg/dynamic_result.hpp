//
// Created by alex2 on 04.05.2021.
//

#pragma once

#include "converter.hpp"

namespace clg {

    /**
     * Тип возвращаемого значения функцией Lua для тех случаев, когда фукнция может возвращать разное количество и тип
     * значений.
     * \example
     * <code>
     * clg::dynamic_result r = vm["my_func"].call&lt;clg::dynamic_result&gt;();
     * </code>
     *
     */
    class dynamic_result {
    private:
        lua_State* mState;

    public:
        dynamic_result(lua_State* state) : mState(state) {}

        dynamic_result(const dynamic_result&) = delete;

        ~dynamic_result() {
            lua_pop(mState, size());
        }

        template<typename T>
        [[nodiscard]] T get(int index) const {
            return converter<std::decay_t<T>>::from_lua(mState, index + 1);
        }

        [[nodiscard]] bool is_nil(int index) const {
            return lua_isnil(mState, index + 1);
        }

        [[nodiscard]] size_t size() const {
            return lua_gettop(mState);
        }
    };


    template<>
    struct converter<dynamic_result> {
        static dynamic_result from_lua(lua_State* l, int n) {
            return dynamic_result(l);
        }
    };

    template<>
    dynamic_result get_from_lua(lua_State* l) {
        return converter<dynamic_result>::from_lua(l, -1);
    }
}