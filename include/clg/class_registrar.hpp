//
// Created by Alex2772 on 11/3/2021.
//

#pragma once

#include "clg.hpp"
#include <vector>
#include <cassert>

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
        std::vector<Method> mStaticFunctions;
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

        template<auto method>
        struct static_function_helper {
            using class_info = state_interface::callable_class_info<decltype(method)>;

            template<typename... Args>
            struct wrapper_function_helper_t {};
            template<typename... Args>
            struct wrapper_function_helper_t<state_interface::types<Args...>> {
                static typename class_info::return_t call(void* self, Args... args) {
                    if (std::is_same_v<void, typename class_info::return_t>) {
                        method(args...);
                    } else {
                        return method(args...);
                    }
                }
                using my_instance = typename state_interface::register_function_helper<typename class_info::return_t, void*, Args...>::template instance<call>;
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
            if (lua_isuserdata(l, 1)) {
                clg::ref::dec<C>(*static_cast<ref::info**>(lua_touserdata(l, 1)));
            }
            return 0;
        }

    public:
        ~class_registrar() {
            std::vector<luaL_Reg> methods;
            std::vector<luaL_Reg> staticFunctions;
            auto top = lua_gettop(mClg);

            for (auto& c : mConstructors) {
                staticFunctions.push_back({"new", c});
            }
            for (auto& c : mStaticFunctions) {
                staticFunctions.push_back({c.name.c_str(), c.cFunction});
            }
            for (auto& c : mMethods) {
                methods.push_back({c.name.c_str(), c.cFunction});
            }

            methods.push_back({nullptr, nullptr});
            staticFunctions.push_back({nullptr, nullptr});

            auto classname = clg::class_name<C>();

            // clazz = staticFunctions
            luaL_newlib(mClg, staticFunctions.data());
            int clazzId = lua_gettop(mClg);

            // metatable = { __gc = ... }
            luaL_newmetatable(mClg, classname.c_str());
            int metatableId = lua_gettop(mClg);
            luaL_Reg metatableFunctions[] = {
                    { "__gc", gc },
                    { nullptr },
            };
            luaL_setfuncs(mClg, metatableFunctions, 0);

            // metatable.__index = methods
            luaL_newlib(mClg, methods.data());
            lua_setfield(mClg, metatableId, "__index");

            // setmetatable(clazz, metatable)
            lua_setmetatable(mClg, clazzId);


            // _G["Classname"] = clazz
            lua_setglobal(mClg, classname.c_str());
            assert(top == lua_gettop(mClg));
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

        template<auto m>
        class_registrar<C>& method(const std::string& name) {
            using wrapper_function_helper = typename method_helper<m>::wrapper_function_helper;
            using my_instance = typename wrapper_function_helper::my_instance;
            mMethods.push_back({
               name,
               my_instance::call
            });
            return *this;
        }

        template<auto m>
        class_registrar<C>& staticFunction(const std::string& name) {
            using wrapper_function_helper = typename static_function_helper<m>::wrapper_function_helper;
            using my_instance = typename wrapper_function_helper::my_instance;
            mStaticFunctions.push_back({
               name,
               my_instance::call
            });
            return *this;
        }
    };
}
