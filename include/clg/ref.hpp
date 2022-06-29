#pragma once

#include <lua.hpp>
#include "converter.hpp"
#include <cassert>
#include <algorithm>

namespace clg {
    class ref {
    public:
        ref() = default;
        ref(const ref& other) noexcept: mLua(other.mLua), mPtr([&] {
            other.push_value_to_stack();
            return luaL_ref(other.mLua, LUA_REGISTRYINDEX);
        }()) {}
        ref(ref&& other) noexcept: mLua(other.mLua), mPtr(other.mPtr) {
            other.mLua = nullptr;
            other.mPtr = -1;
        }

        ref& operator=(ref&& other) noexcept {
            mLua = other.mLua;
            mPtr = other.mPtr;
            other.mLua = nullptr;
            other.mPtr = -1;
            return *this;
        }

        ~ref() {
            if (mPtr != -1) {
                luaL_unref(mLua, LUA_REGISTRYINDEX, mPtr);
            }
        }


        ref& operator=(std::nullptr_t) noexcept {
            if (mPtr != -1) {
                luaL_unref(mLua, LUA_REGISTRYINDEX, mPtr);
                mPtr = -1;
            }
            return *this;
        }

        static ref from_stack(lua_State* state) noexcept {
            return { state };
        }

        void push_value_to_stack() const noexcept {
            assert(mLua != nullptr);
            lua_rawgeti(mLua, LUA_REGISTRYINDEX, mPtr);
        }

    private:
        lua_State* mLua = nullptr;
        int mPtr = -1;

        ref(lua_State* state) noexcept: mLua(state), mPtr(luaL_ref(state, LUA_REGISTRYINDEX)) {}
    };

    template<>
    struct converter<clg::ref> {
        static ref from_lua(lua_State* l, int n) {
            lua_pushvalue(l, n);
            return clg::ref::from_stack(l);
        }
        static int to_lua(lua_State* l, const clg::ref& ref) {
            ref.push_value_to_stack();
            return 1;
        }
    };
}