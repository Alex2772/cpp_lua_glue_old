#pragma once

#include "weak_ref.hpp"
#include "table.hpp"

namespace clg {

    /**
     * @brief When extended from, allows to avoid extra overhead when passed to lua. Also allows lua code to use the
     * object as a table.
     * @tparam T The derived type (see example)
     * @details
     * @code{cpp}
     * class Person: clg::lua_self<Person> {
     * public:
     *   // ...
     * };
     * @endcode
     *
     * In lua:
     * @code{lua}
     * p = Person:new()
     * p['myCustomField'] = 'my extra data'
     * @endcode
     */
    class lua_self {
    private:
        friend clg::ref& lua_self_weak_ptr_and_data_holder(lua_self& s);
        friend clg::weak_ref& lua_self_shared_ptr_holder(lua_self& s);
        clg::ref      mWeakPtrAndDataHolder;
        clg::weak_ref mSharedPtrHolder;

    };


    inline clg::ref& lua_self_weak_ptr_and_data_holder(lua_self& s) {
        return s.mWeakPtrAndDataHolder;
    }
    inline clg::weak_ref& lua_self_shared_ptr_holder(lua_self& s) {
        return s.mSharedPtrHolder;
    }

    /**
     * userdata
     */
    template<typename T>
    struct converter<std::shared_ptr<T>> {
        static constexpr bool use_lua_self = std::is_base_of_v<clg::lua_self, T>;

        static std::shared_ptr<T> from_lua(lua_State* l, int n) {
            if (lua_isnil(l, n)) {
                return nullptr;
            }

            if constexpr(use_lua_self) {
                if (lua_istable(l, n)) {
                    lua_getfield(l, n, "clg_strongref");
                    if (!lua_isuserdata(l, -1)) {
                        lua_pop(l, 1);
                        detail::throw_converter_error(l, n, "not a cpp object");
                    }
                    auto p = reinterpret_cast<shared_ptr_helper*>(lua_touserdata(l, -1))->as<T>();
                    lua_pop(l, 1);
                    return p;
                }
                return nullptr;
            } else {
                if (lua_isuserdata(l, n)) {
                    return reinterpret_cast<shared_ptr_helper*>(lua_touserdata(l, n))->as<T>();
                }
                detail::throw_converter_error(l, n, "not a userdata");
                return nullptr;
            }
        }

        static void push_shared_ptr_userdata(lua_State* l, std::shared_ptr<T> v) {
            auto classname = clg::class_name<T>();
            auto t = reinterpret_cast<shared_ptr_helper*>(lua_newuserdata(l, sizeof(shared_ptr_helper)));
            new(t) shared_ptr_helper(std::move(v));
            luaL_getmetatable(l, classname.c_str());
            if (lua_isnil(l, -1)) {
                lua_pop(l, 1);
            } else {
                lua_setmetatable(l, -2);
            }
        }

        static void push_weak_ptr_userdata(lua_State* l, std::weak_ptr<T> v) {
            auto classname = clg::class_name<T>();
            auto t = reinterpret_cast<weak_ptr_helper*>(lua_newuserdata(l, sizeof(weak_ptr_helper)));
            new(t) weak_ptr_helper(std::move(v));
            luaL_getmetatable(l, classname.c_str());
            if (lua_isnil(l, -1)) {
                lua_pop(l, 1);
            } else {
                lua_setmetatable(l, -2);
            }
        }

        static void push_strong_ref_holder_object(lua_State* l, std::shared_ptr<T> v, clg::ref dataHolderRef) {
            push_shared_ptr_userdata(l, std::move(v));
            clg::push_to_lua(l, clg::table{{"clg_strongref", clg::ref::from_stack(l)}});
            clg::push_to_lua(l, clg::table{
                { "__index", dataHolderRef },
                { "__newindex", std::move(dataHolderRef) },
            });
            lua_setmetatable(l, -2);
        }

        static int to_lua(lua_State* l, std::shared_ptr<T> v) {
            if (v == nullptr) {
                lua_pushnil(l);
                return 1;
            }

            if constexpr(!use_lua_self) {
                push_shared_ptr_userdata(l, std::move(v));
            } else {
                auto& weakRef = lua_self_shared_ptr_holder(*v);
                if (auto lock = weakRef.lock()) {
                    lock.push_value_to_stack();
                    return 1;
                }

                auto& dataHolder = lua_self_weak_ptr_and_data_holder(*v);
                if (dataHolder.isNull()) {
                    // should compose strong ref holder and weak ref holder objects
                    // weak ref and data holder object
                    lua_createtable(l, 0, 0);
                    push_weak_ptr_userdata(l, v);
                    clg::push_to_lua(l, clg::table{{"__index", clg::ref::from_stack(l)}});
                    lua_setmetatable(l, -2);

                    dataHolder = clg::ref::from_stack(l);
                }

                push_strong_ref_holder_object(l, std::move(v), std::move(dataHolder));
                lua_pushvalue(l, -1);
                weakRef = clg::weak_ref(clg::ref::from_stack(l));
                return 1;
            }
            return 1;
        }
    };
}