//
// Created by alex2 on 02.05.2021.
//

#pragma once

#include "lua.hpp"
#include "converter.hpp"
#include "dynamic_result.hpp"

#include <cstring>

namespace clg {


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

                        if constexpr (std::is_same_v<void, Return>) {
                            // ничего не возвращается
                            f(clg::get_from_lua<std::decay_t<Args>>(s)...);
                            return 0;
                        } else {
                            // возвращаем одно значение
                            return clg::push_to_lua(s, f(clg::get_from_lua<std::decay_t<Args>>(s)...));
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

    public:
        explicit state_interface(lua_State* state) : mState(state) {}

        template<auto f>
        void register_function(const std::string& name) {
            using my_register_function_helper = decltype(make_register_function_helper(f));
            using my_instance = typename my_register_function_helper::template instance<f>;
            register_function_raw(name, my_instance::call);
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

        class function_call {
            friend class state_interface;
        private:
            state_interface& mClg;
            const std::string& mName;
            function_call(const std::string& name, state_interface& clg) : mName(name), mClg(clg) {}


            template<typename Arg, typename... Args>
            void push(Arg&& arg, Args&&... args) {
                push_to_lua(mClg, std::forward<Arg>(arg));

                push(std::forward<Args>(args)...);
            }

            void push() {}

            void push_function_to_be_called() {
                lua_getglobal(mClg, mName.c_str());
            }

            void do_call(unsigned args, int results) {
                if (lua_pcall(mClg, args, results, 0)) {
                    throw lua_exception(get_from_lua<std::string>(mClg));
                }
            }
        public:

            template<typename... Args>
            void operator()(Args&&... args) {
                push_function_to_be_called();
                push(std::forward<Args>(args)...);
                do_call(sizeof...(args), 0);
            }
            template<typename Return, typename... Args>
            Return call(Args&&... args) {
                push_function_to_be_called();
                push(std::forward<Args>(args)...);

                if constexpr (std::is_same_v<Return, dynamic_result>) {
                    do_call(sizeof...(args), LUA_MULTRET);
                    return get_from_lua<Return>(mClg);
                } else if constexpr (std::is_same_v<Return, void>) {
                    do_call(sizeof...(args), 0);
                } else {
                    do_call(sizeof...(args), 1);
                    return get_from_lua<Return>(mClg);
                }
            }
        };

        /**
         * Вызов глобальной переменной.
         *
         * @param v название функции для вызова
         * @return обёртка для передачи аргументов в функцию
         */
        function_call operator[](const std::string& v) {
            return {v, *this};
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