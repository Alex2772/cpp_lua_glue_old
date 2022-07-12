//
// Created by Alex2772 on 11/3/2021.
//

#pragma once

#include "clg.hpp"
#include <vector>
#include <cassert>
#include <cstdio>

namespace clg {
    namespace impl {
        struct Method {
            std::string name;
            lua_CFunction cFunction;
        };
    }
    using lua_cfunctions = std::vector<impl::Method>;

    namespace impl {
#if LUA_VERSION_NUM == 501
        static void newlib(lua_State* L, std::vector<luaL_Reg>& l) {
            lua_createtable(L, 0, l.size() - 1);
            luaL_setfuncs(L, l.data(), 0);
        }
#else
    static void newlib(lua_State* L, std::vector<luaL_Reg>& l) {
        luaL_newlib(L, l.data());
    }
#endif
        static void newlib(lua_State* L, const lua_cfunctions& l) {
            std::vector<luaL_Reg> reg;
            reg.reserve(l.size() + 1);
            for (const auto& c : l) {
                reg.push_back({c.name.c_str(), c.cFunction});
            }
            reg.push_back({nullptr, nullptr});
            newlib(L, reg);
        }
    }

    template<class C>
    class class_registrar {
    friend class clg::state_interface;
    private:
        state_interface& mClg;

        class_registrar(state_interface& clg):
            mClg(clg)
        {

        }

        lua_cfunctions mMethods;
        lua_cfunctions mStaticFunctions;
        std::vector<lua_CFunction> mConstructors;

        template<auto methodPtr>
        struct method_helper {
            using class_info = state_interface::callable_class_info<decltype(methodPtr)>;

            template<typename... Args>
            struct wrapper_function_helper_t {};
            template<typename... Args>
            struct wrapper_function_helper_t<state_interface::types<Args...>> {
                static typename class_info::return_t method(std::shared_ptr<C> self, Args... args) {
                    if (std::is_same_v<void, typename class_info::return_t>) {
                        (self.get()->*methodPtr)(std::move(args)...);
                    } else {
                        return (self.get()->*methodPtr)(std::move(args)...);
                    }
                }
                static clg::builder_return_type builder_method(std::shared_ptr<C> self, Args... args) {
                    (self.get()->*methodPtr)(std::move(args)...);
                    return {};
                }
                using my_instance = typename state_interface::register_function_helper<typename class_info::return_t, std::shared_ptr<C>, Args...>::template instance<method>;
                using my_instance_builder = typename state_interface::register_function_helper<clg::builder_return_type, std::shared_ptr<C>, Args...>::template instance<builder_method>;
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
                static typename class_info::return_t static_method(void* self, Args... args) {
                    if (std::is_same_v<void, typename class_info::return_t>) {
                        method(std::move(args)...);
                    } else {
                        return method(std::move(args)...);
                    }
                }
                static typename class_info::return_t static_method_no_this(Args... args) {
                    if (std::is_same_v<void, typename class_info::return_t>) {
                        method(std::move(args)...);
                    } else {
                        return method(std::move(args)...);
                    }
                }
                using my_instance = typename state_interface::register_function_helper<typename class_info::return_t, void*, Args...>::template instance<static_method>;
                using my_instance_no_this = typename state_interface::register_function_helper<typename class_info::return_t, Args...>::template instance<static_method_no_this>;
            };

            using wrapper_function_helper = wrapper_function_helper_t<typename class_info::args>;
        };

        template<typename... Args>
        struct constructor_helper {
            static std::shared_ptr<C> construct(void* self, Args... args) {
                return std::make_shared<C>(std::move(args)...);
            }
        };

        static int gc(lua_State* l) {
            if (lua_isuserdata(l, 1)) {
                static_cast<clg::shared_ptr_helper*>(lua_touserdata(l, 1))->~shared_ptr_helper();
            }
            return 0;
        }
        static int eq(lua_State* l) {
            auto v1 = get_from_lua<std::shared_ptr<C>>(l, 1);
            auto v2 = get_from_lua<std::shared_ptr<C>>(l, 2);
            push_to_lua(l, v1 == v2);
            return 1;
        }
        static int concat(lua_State* l) {
            auto v1 = any_to_string(l, 1);
            auto v2 = any_to_string(l, 2);
            v1 += v2;
            push_to_lua(l, v1);
            return 1;
        }
        static int tostring(lua_State* l) {
            auto v1 = get_from_lua<std::shared_ptr<C>>(l, 1);
            push_to_lua(l, toString(v1));
            return 1;
        }

        static std::string toString(const std::shared_ptr<C>& v) {
            char buf[64];
            std::sprintf(buf, "%s<%p>", class_name<C>().c_str(), v.get());
            return buf;
        }

    public:
        ~class_registrar() {
            std::vector<luaL_Reg> staticFunctions;
            auto top = lua_gettop(mClg);

            for (auto& c : mConstructors) {
                staticFunctions.push_back({"new", c});
            }
            for (auto& c : mStaticFunctions) {
                staticFunctions.push_back({c.name.c_str(), c.cFunction});
            }

            staticFunctions.push_back({nullptr, nullptr});

            auto classname = clg::class_name<C>();

            // clazz = staticFunctions
            impl::newlib(mClg, staticFunctions);
            int clazzId = lua_gettop(mClg);

            // metatable = { __gc = ... }
            luaL_newmetatable(mClg, classname.c_str());
            int metatableId = lua_gettop(mClg);
            luaL_Reg metatableFunctions[] = {
                    { "__gc", gc },
                    { "__eq", eq },
                    { "__concat", concat },
                    { "__tostring", tostring },
                    { nullptr },
            };
            luaL_setfuncs(mClg, metatableFunctions, 0);

            // metatable.__index = methods
            impl::newlib(mClg, mMethods);
            lua_setfield(mClg, metatableId, "__index");

            // setmetatable(clazz, metatable)
            lua_setmetatable(mClg, clazzId);


            // _G["Classname"] = clazz
            lua_setglobal(mClg, classname.c_str());
            assert(top == lua_gettop(mClg));
        }


        template<typename... Args>
        class_registrar<C>& constructor() {
            using my_register_function_helper = clg::state_interface::register_function_helper<std::shared_ptr<C>, void*, Args...>;
            using my_instance = typename my_register_function_helper::template instance<constructor_helper<Args...>::construct>;

            mConstructors.push_back({
                my_instance::call
            });
            return *this;
        }

        template<auto m>
        class_registrar<C>& method(std::string name) {
            using wrapper_function_helper = typename method_helper<m>::wrapper_function_helper;
            using my_instance = typename wrapper_function_helper::my_instance;
            mMethods.push_back({
               std::move(name),
               my_instance::call
            });
            return *this;
        }

        template<auto m>
        class_registrar<C>& builder_method(std::string name) {
            using wrapper_function_helper = typename method_helper<m>::wrapper_function_helper;
            using my_instance = typename wrapper_function_helper::my_instance_builder;
            mMethods.push_back({
               std::move(name),
               my_instance::call
            });
            return *this;
        }

        template<auto m>
        class_registrar<C>& staticFunction(std::string name) {
            using wrapper_function_helper = typename static_function_helper<m>::wrapper_function_helper;

#if LUA_VERSION_NUM == 501
            constexpr auto call = wrapper_function_helper::my_instance_no_this::call;
#else
            constexpr auto call = wrapper_function_helper::my_instance::call;
#endif
            mStaticFunctions.push_back({
               std::move(name),
               call
            });
            return *this;
        }
    };
}
