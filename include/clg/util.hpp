//
// Created by Alex2772 on 11/3/2021.
//

#pragma once

namespace clg {
    template<class T>
    std::string class_name() {
        std::string s = typeid(T).name();
        auto it = std::find_if(s.rbegin(),  s.rend(), [](char c) {
            return !isalnum(c);
        });
        return {it.base(), s.end()};
    }
}