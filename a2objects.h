#pragma once

#include "a2types.h"

namespace a2 {

inline A2World* World() {
    return *reinterpret_cast<A2World**>(0x6A8B8C);
}

} // namespace a2
