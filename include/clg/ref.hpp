//
// Created by Alex2772 on 11/8/2021.
//

#pragma once

#include <map>

namespace clg {
    struct ref {
        struct info {
            void* ref;
            size_t count = 0;
        };

        static std::map<void*, info>& map() {
            static std::map<void*, info> m;
            return m;
        }

        template<class T>
        static info* wrap(T* ref) {
            auto it = map().find(ref);
            if (it != map().end()) {
                it->second.count += 1;
                return &it->second;
            }
            auto& i = map()[ref];
            i = { ref, 1 };
            return &i;
        }

        template<class T>
        static void dec(info* i) {
            i->count -= 1;
            if (i->count == 0) {
                auto v = i->ref;
                map().erase(i->ref);
                delete reinterpret_cast<T*>(v);
            }
        }
    };
}