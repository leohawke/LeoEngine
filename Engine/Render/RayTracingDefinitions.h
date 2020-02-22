#pragma once

#define RAY_TRACING_REGISTER_SPACE_LOCAL  0 // default register space for hit group (closest hit, any hit, intersection) shader resources
#define RAY_TRACING_REGISTER_SPACE_GLOBAL 1 // register space for ray generation and miss shaders 
#define RAY_TRACING_REGISTER_SPACE_SYSTEM 2 // register space for "system" parameters (index buffer, vertex buffer, fetch parameters)
