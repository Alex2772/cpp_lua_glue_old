//
// Created by Alex2772 on 11/3/2021.
//

#pragma once

#include "value.hpp"
#include "converter.hpp"
#include <map>
#include <string>

namespace clg {
    class table: public std::map<std::string, clg::value> {
    public:
        using std::map<std::string, clg::value>::map;

        [[nodiscard]]
        std::vector<clg::value> toArray() const {
            std::vector<clg::value> result;
            result.resize(size());
            for (auto& v : *this) {
                try {
                    size_t index = std::stoul(v.first) - 1;
                    if (index >= result.size()) {
                        result.resize(index + 1);
                    }
                    result[index] = v.second;
                } catch (...) {

                }
            }
            return result;
        }
    };

    template<>
    struct converter<clg::table> {
        static clg::table from_lua(lua_State* l, int n) {
            if (!lua_istable(l, n)) {
                clg::detail::throw_converter_error(l, n, "not a table");
            }

            clg::table result;
            if (n < 0) {
                n = lua_gettop(l) + n + 1;
            }
            lua_pushnil(l);
            while (lua_next(l, n) != 0)
            {
                // copy key so that lua_tostring should not break lua_next by modifying key
                lua_pushvalue(l, -2);
                result[clg::get_from_lua<std::string>(l, -1)] = clg::get_from_lua<clg::value>(l, -2);
                lua_pop(l, 2);
            }
            return result;
        }
        /*
        static int to_lua(lua_State* l, std::nullptr_t v) {
            lua_pushnil(l);
            return 1;
        }*/
    };

}