#pragma once

#include "tinyECS/registry.hpp"

class EffectSystem
{
public:
    EffectSystem();
    void step(float elapsed_ms);
};
