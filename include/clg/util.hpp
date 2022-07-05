//
// Created by Alex2772 on 11/3/2021.
//

#pragma once

#ifdef __linux
#include <cxxabi.h>
#endif

#include <algorithm>
#include <cctype>
#include <string>
#include <cassert>

namespace clg {
    template<class T>
    std::string class_name() {
        std::string s = typeid(T).name();
#ifdef __linux
        int status;
        auto c = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
        s = c;
        delete[] c;
#endif

        auto it = std::find_if(s.rbegin(),  s.rend(), [](char c) {
            return !std::isalnum(c);
        });

        return {it.base(), s.end()};
    }


    /**
     * @brief проверяет, что стек луа не поменялся в скоупе RAII
     */
    struct stack_integrity_check {
    public:
        stack_integrity_check(lua_State* lua): mLua(lua) {
            mStack = lua_gettop(mLua);
        }

        ~stack_integrity_check() {
            assert(lua_gettop(mLua) == mStack);
        }

    private:
        lua_State* mLua;
        int mStack;
    };
}