//
// Created by alex2 on 02.05.2021.
//

#pragma once

#include "lua.hpp"
#include "converter.hpp"
#include "dynamic_result.hpp"
#include "function.hpp"
#include "util.hpp"
#include "vararg.hpp"
#include "shared_ptr_helper.hpp"
#include "magic_enum.hpp"

#include <cstring>
#include <thread>

namespace clg {

    template<class C>
    class class_registrar;

    /**
     * @brief Helper type. When used as a return type, the first argument passed to a c++ function is returned to lua.
     */
    struct builder_return_type {};

    struct substitution_error: std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    namespace impl {
        struct Method {
            std::string name;
            lua_CFunction cFunction;
        };
    }
    using lua_cfunctions = std::vector<impl::Method>;


    struct class_inherit_metainfo {
        std::function<bool(clg::allow_lua_inheritance* child)> isBaseOf;
        lua_cfunctions methods;
    };


    /**
     * Базовый интерфейс для работы с Lua. Не инициализирует Lua самостоятельно.
     */
    class state_interface {
    private:
        lua_State* mState;
        std::thread::id mMyThread = std::this_thread::get_id();

        void throw_syntax_error() {
            auto s = get_from_lua<std::string>(mState);
            throw lua_exception(std::move(s));
        }

        void register_function_raw(const std::string& name, int(* function)(lua_State* s)) {
            lua_register(mState, name.c_str(), function);
        }


        std::vector<class_inherit_metainfo> mClassMetainfo;

    public:


        std::vector<class_inherit_metainfo>& class_metainfo() {
            return mClassMetainfo;
        }

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

            static constexpr bool is_vararg = std::is_same_v<std::tuple<Args...>, std::tuple<vararg>>;

            template<function_t f, bool passthroughSubstitutionError = false>
            struct instance {
                static int call(lua_State* s) noexcept(!passthroughSubstitutionError) {
                    try {
                        size_t argsCount = lua_gettop(s);
                        std::tuple<std::decay_t<Args>...> argsTuple;

                        try {
                            if constexpr (!is_vararg) {
                                if (argsCount != sizeof...(Args)) {
                                    throw std::runtime_error("invalid argument count! expected "
                                                             + std::to_string(sizeof...(Args))
                                                             + ", actual " + std::to_string(argsCount));
                                }
                            }

                            tuple_fill_from_lua_helper<std::decay_t<Args>...>(s).template fill<0, std::decay_t<Args>...>(argsTuple);
                        } catch (const std::exception& e) {
                            throw substitution_error(e.what());
                        }

                        if constexpr (std::is_same_v<Return, builder_return_type>) {
                            lua_pop(s, sizeof...(Args) - 1);
                            (std::apply)(f, std::move(argsTuple));
                            return 1;
                        } else if constexpr (std::is_void_v<Return>) {
                            lua_pop(s, sizeof...(Args));
                            // ничего не возвращается
                            (std::apply)(f, std::move(argsTuple));
                            return 0;
                        } else {
                            if constexpr (!is_vararg) {
                                lua_pop(s, sizeof...(Args));
                            }
                            // возвращаем одно значение
                            return clg::push_to_lua(s, (std::apply)(f, std::move(argsTuple)));
                        }
                    } catch (const substitution_error& e) {
                        if constexpr (passthroughSubstitutionError) {
                            throw;
                        }
                        luaL_error(s, "cpp exception: %s", e.what());
                        return 0;
                    } catch (const std::exception& e) {
                        luaL_error(s, "cpp exception: %s", e.what());
                        return 0;
                    }
                }
            };
        };

        template<typename Return, typename... Args>
        static register_function_helper<Return, Args...> make_register_function_helper(Return(*)(Args...)) {
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
            static constexpr bool is_noexcept = false;
            static constexpr bool is_const = true;
        };
        template<typename Class, typename R, typename... Args>
        struct callable_class_info<R(Class::*)(Args...)> {
            using class_t = Class;
            using args = types<Args...>;
            using return_t = R;
            static constexpr bool is_noexcept = false;
            static constexpr bool is_const = false;
        };

        template<typename Class, typename R, typename... Args>
        struct callable_class_info<R(Class::*)(Args...) const noexcept> {
            using class_t = Class;
            using args = types<Args...>;
            using return_t = R;
            static constexpr bool is_noexcept = true;
            static constexpr bool is_const = true;
        };
        template<typename Class, typename R, typename... Args>
        struct callable_class_info<R(Class::*)(Args...) noexcept> {
            using class_t = Class;
            using args = types<Args...>;
            using return_t = R;
            static constexpr bool is_noexcept = true;
            static constexpr bool is_const = false;
        };

        template<typename R, typename... Args>
        struct callable_class_info<R(*)(Args...)> {
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
                        (*callable())(std::move(args)...);
                    } else {
                        return (*callable())(std::move(args)...);
                    }
                }
            };

            using wrapper_function_helper = wrapper_function_helper_t<typename function_info::args>;

            static Callable*& callable() {
                static Callable* callable;
                return callable;
            }

        };

        template<typename... Callables>
        struct overloaded_helper {
            static int fake_lua_cfunction(lua_State* L) {
                std::string errorDescription;
                for (const auto& func : callable()) {
                    auto stack = lua_gettop(L);
                    try {
                        return func(L);
                    } catch (const clg::substitution_error& e) {
                        if (!errorDescription.empty()) {
                            errorDescription += "; ";
                        }
                        errorDescription += e.what();
                    }
                    assert(lua_gettop(L) == stack);
                }
                luaL_error(L, "overloaded function substitution error: %s", errorDescription.c_str());
                return 0;
            }

            static std::vector<lua_CFunction>& callable() {
                static std::vector<lua_CFunction> callable;
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

        template<typename FirstCallable, typename... RestCallables>
        std::vector<lua_CFunction>& register_function_overloaded(const std::string& name, FirstCallable&& firstCallable, RestCallables&&... restCallables) {
            using helper = overloaded_helper<FirstCallable, RestCallables...>;
            auto& callable = helper::callable();
            callable = { wrap_lambda_to_cfunction_for_overloading(std::forward<FirstCallable>(firstCallable)),
                         wrap_lambda_to_cfunction_for_overloading(std::forward<RestCallables>(restCallables))...  };
            register_function_raw(name, helper::fake_lua_cfunction);
            return helper::callable();
        }


        template<typename Callable, bool passthroughSubstitutionError = false>
        lua_CFunction wrap_lambda_to_cfunction(Callable&& callable) {
            using helper = callable_helper<Callable>;
            helper::callable() = new Callable(std::forward<Callable>(callable));
            constexpr auto f = helper::wrapper_function_helper::wrapper_function;
            using my_register_function_helper = decltype(make_register_function_helper(f));
            using my_instance = typename my_register_function_helper::template instance<f, passthroughSubstitutionError>;
            return my_instance::call;
        }

        template<typename Callable>
        lua_CFunction wrap_lambda_to_cfunction_for_overloading(Callable&& callable) { // used only for register_function_overloaded
            return wrap_lambda_to_cfunction<Callable, true>(std::forward<Callable>(callable));
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
            assert(("multithreading is not supported", mMyThread == std::this_thread::get_id()));
            return mState;
        }

        template<typename Enum>
        static int to_int_mapper(Enum value) {
            return static_cast<int>(value);
        }
        template<typename T, typename Mapper = decltype(to_int_mapper<T>)>
        void register_enum(Mapper&& mapper = to_int_mapper<T>) noexcept {
            static_assert(std::is_enum_v<T>, "T expected to be enum");
            stack_integrity_check checks(mState);

            lua_createtable(mState, 0, magic_enum::enum_values<T>().size());

            for (const auto value : magic_enum::enum_values<T>()) {
                lua_pushstring(mState, magic_enum::enum_name(value).data());
                clg::push_to_lua(mState, mapper(value));
                lua_settable(mState, -3);
            }
            lua_setglobal(mState, clg::class_name<T>().c_str());
        };


        template<typename T>
        void set_global_value(std::string_view name, const T& value) noexcept {
            clg::stack_integrity_check check(*this);
            push_to_lua(mState, value);
            lua_setglobal(mState, name.data());
        }



        /**
         * Вызов глобальной функции.
         *
         * @param v название функции для вызова
         * @return обёртка для передачи аргументов в функцию
         */
        function global_function(std::string_view v) {
            lua_getglobal(mState, v.data());
            return {clg::ref::from_stack(*this), *this};
        }


        void collectGarbage() {
            lua_gc(mState, LUA_GCCOLLECT, 0);
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


    /**
     * @brief Chooses specific overload of a method.
     * @ingroup useful_traits
     * @details
     * Example:
     * @code{cpp}
     * struct GameObject {
     * public:
     *   void setPos(glm::vec3);
     *   void setPos(glm::vec2);
     * };
     * ...
     * auto setPosVec2 = clg::select_overload<glm::vec2>::of(&GameObject::setPos);
     * @endcode
     */
    template<typename... Args>
    struct select_overload {
        template<typename Return, typename Class>
        static constexpr auto of(Return(Class::*ptr)(Args... args)) -> decltype(ptr) {
            return ptr;
        }
    };
}

#include "class_registrar.hpp"
#include "table.hpp"
#include "object_expose.hpp"
