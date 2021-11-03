//
// Created by Alex2772 on 11/3/2021.
//

#pragma once

#include "clg.hpp"

namespace clg {
    template<class C>
    class class_registrar {
    friend class clg::state_interface;
    private:
        state_interface& mClg;

        class_registrar(state_interface& clg):
            mClg(clg)
        {

        }

        struct Method {
            std::string name;
            lua_CFunction cFunction;
        };

        std::vector<Method> mMethods;
        std::vector<lua_CFunction> mConstructors;

        template<auto method>
        struct method_helper {
            using class_info = state_interface::callable_class_info<decltype(method)>;

            template<typename... Args>
            struct wrapper_function_helper_t {};
            template<typename... Args>
            struct wrapper_function_helper_t<state_interface::types<Args...>> {
                static typename class_info::return_t call(C* self, Args... args) {
                    if (std::is_same_v<void, typename class_info::return_t>) {
                        (self->*method)(args...);
                    } else {
                        return (self->*method)(args...);
                    }
                }
                using my_instance = typename state_interface::register_function_helper<typename class_info::return_t, C*, Args...>::template instance<call>;
            };

            using wrapper_function_helper = wrapper_function_helper_t<typename class_info::args>;
        };
        template<typename... Args>
        struct constructor_helper {
            static C* construct(void* self, Args... args) {
                return new C(args...);
            }
        };

        static int gc(lua_State* l) {
            delete reinterpret_cast<C*>(lua_touserdata(l, 0));

            return 0;
        }

    public:
        ~class_registrar() {
            std::vector<luaL_Reg> entries;
            for (auto& c : mConstructors) {
                entries.push_back({"new", c});
            }
            for (auto& c : mMethods) {
                entries.push_back({c.name.c_str(), c.cFunction});
            }
            entries.push_back({nullptr, nullptr});
            const luaL_Reg meta[] = {
                    { "__gc", gc },
                    { nullptr, nullptr }
            };
            auto classname = clg::class_name<C>();
            luaL_newlib(mClg, entries.data());
            luaL_newmetatable(mClg, classname.c_str());

            int lib_id, meta_id;

            /* newclass = {} */
            lua_createtable(mClg, 0, 0);
            lib_id = lua_gettop(mClg);

            /* metatable = {} */
            luaL_newmetatable(mClg, classname.c_str());
            meta_id = lua_gettop(mClg);
            luaL_setfuncs(mClg, meta, 0);

            /* metatable.__index = _methods */
            luaL_newlib(mClg, entries.data());
            lua_setfield(mClg, meta_id, "__index");

            /* metatable.__metatable = meta */
            luaL_newlib(mClg, meta);
            lua_setfield(mClg, meta_id, "__metatable");

            /* class.__metatable = metatable */
            lua_setmetatable(mClg, lib_id);

            /* _G["Foo"] = newclass */
            lua_setglobal(mClg, classname.c_str());
        }


        template<typename... Args>
        class_registrar<C>& constructor() {
            using my_register_function_helper = clg::state_interface::register_function_helper<C*, void*, Args...>;
            using my_instance = typename my_register_function_helper::template instance<constructor_helper<Args...>::construct>;

            mConstructors.push_back({
                my_instance::call
            });
            return *this;
        }

        template<auto method>
        class_registrar<C>& method(const std::string& name) {
            using wrapper_function_helper = typename method_helper<method>::wrapper_function_helper;
            using my_instance = typename wrapper_function_helper::my_instance;
            mMethods.push_back({
               name,
               my_instance::call
            });
            return *this;
        }
    };
}
