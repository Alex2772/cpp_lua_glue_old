//
// Created by alex2 on 02.05.2021.
//

#pragma once

#include "lua.hpp"
#include "converter.hpp"
#include "dynamic_result.hpp"
#include "lua_function.hpp"
#include "util.hpp"

#include <cstring>

namespace clg {

    template<class C>
    class class_registrar;


    /**
     * Базовый интерфейс для работы с Lua. Не инициализирует Lua самостоятельно.
     */
    class state_interface {
    private:
        lua_State* mState;

        void throw_syntax_error() {
            throw lua_exception(get_from_lua<std::string>(mState));
        }

        void register_function_raw(const std::string& name, int(* function)(lua_State* s)) {
            lua_register(mState, name.c_str(), function);
        }

    public:

        template<typename... TupleArgs>
        struct tuple_fill_from_lua_helper {
            lua_State* l;
            tuple_fill_from_lua_helper(lua_State* l) : l(l) {}

            using target_tuple = std::tuple<TupleArgs...>;

            template<unsigned index>
            void fill(target_tuple& t) {}

            template<unsigned index, typename Arg, typename... Args>
            void fill(target_tuple& t) {
                std::get<index>(t) = clg::get_from_lua<Arg>(l, index + 1);
                fill<index + 1, Args...>(t);
            }
        };

        template<typename Return, typename... Args>
        struct register_function_helper {
            using function_t = Return(*)(Args...);

            template<function_t f>
            struct instance {
                static int call(lua_State* s) {
                    try {
                        size_t argsCount = lua_gettop(s);
                        if (argsCount != sizeof...(Args)) {
                            throw std::runtime_error("invalid argument count! expected "
                                                     + std::to_string(sizeof...(Args))
                                                     + ", actual " + std::to_string(argsCount));
                        }

                        std::tuple<std::decay_t<Args>...> argsTuple;
                        tuple_fill_from_lua_helper<std::decay_t<Args>...>(s).template fill<0, std::decay_t<Args>...>(argsTuple);
                        lua_pop(s, sizeof...(Args));
                        if constexpr (std::is_same_v<void, Return>) {
                            // ничего не возвращается
                            (std::apply)(f, argsTuple);
                            return 0;
                        } else {
                            // возвращаем одно значение
                            return clg::push_to_lua(s, (std::apply)(f, argsTuple));
                        }
                    } catch (const std::exception& e) {
                        luaL_error(s, "cpp exception: %s", e.what());
                        return 0;
                    }
                }
            };
        };

        template<typename Return, typename... Args>
        register_function_helper<Return, Args...> make_register_function_helper(Return(*)(Args...)) {
            return {};
        }

        template<class...>
        struct types {
            using type = types;
        };

        template<typename Sig>
        struct callable_class_info;

        template<typename Class, typename R, typename... Args>
        struct callable_class_info<R(Class::*)(Args...) const> {
            using class_t = Class;
            using args = types<Args...>;
            using return_t = R;
        };
        template<typename Class, typename R, typename... Args>
        struct callable_class_info<R(Class::*)(Args...)> {
            using class_t = Class;
            using args = types<Args...>;
            using return_t = R;
        };

        template<typename Callable>
        struct callable_helper {
            using function_info = callable_class_info<decltype(&Callable::operator())>;

            template<typename... Args>
            struct wrapper_function_helper_t {};
            template<typename... Args>
            struct wrapper_function_helper_t<types<Args...>> {
                static typename function_info::return_t wrapper_function(Args... args) {
                    if (std::is_same_v<typename function_info::return_t, void>) {
                        (*callable())(args...);
                    } else {
                        return (*callable())(args...);
                    }
                }
            };

            using wrapper_function_helper = wrapper_function_helper_t<typename function_info::args>;

            static Callable*& callable() {
                static Callable* callable;
                return callable;
            }

        };

        explicit state_interface(lua_State* state) : mState(state) {}


        template<class C>
        class_registrar<C> register_class() {
            return class_registrar<C>(*this);
        }

        template<auto f>
        void register_function(const std::string& name) {
            using my_register_function_helper = decltype(make_register_function_helper(f));
            using my_instance = typename my_register_function_helper::template instance<f>;
            register_function_raw(name, my_instance::call);
        }
        template<typename Callable>
        void register_function(const std::string& name, Callable callable) {
            using helper = callable_helper<Callable>;
            helper::callable() = new Callable(std::move(callable));
            register_function<helper::wrapper_function_helper::wrapper_function>(name);
        }

        template<typename ReturnType = void>
        ReturnType do_string(const std::string& exec) {
            if (luaL_dostring(mState, exec.c_str()) != 0) {
                throw_syntax_error();
            }
            if constexpr (!std::is_same_v<void, ReturnType>) {
                return get_from_lua<ReturnType>(mState);
            }
        }
        template<typename ReturnType = void>
        ReturnType do_file(const std::string& exec) {
            if (luaL_dofile(mState, exec.c_str()) != 0) {
                throw_syntax_error();
            }
            if constexpr (!std::is_same_v<void, ReturnType>) {
                return get_from_lua<ReturnType>(mState);
            }
        }

        operator lua_State*() const {
            return mState;
        }


        /**
         * Вызов глобальной функции.
         *
         * @param v название функции для вызова
         * @return обёртка для передачи аргументов в функцию
         */
        lua_function operator[](const std::string& v) {
            return {v, *this};
        }

        void stacktrace() {
            int t = lua_gettop(mState);
            for (int i = 0; i <= t; ++i) {
                std::cout << "[" << i << "] " << any_to_string(mState, i) << std::endl;
            }
        }
    };

    /**
     * В отличии от interface, этот класс сам создаёт виртуальную машину Lua, загружает базовые библиотеки и отвечает за
     * её освобождение.
     */
    class vm: public state_interface {
    public:
        vm(): state_interface(luaL_newstate()) {
            luaL_openlibs(*this);
        }
        ~vm() {
            lua_close(*this);
        }

    };
}

#include "class_registrar.hpp"