/**
 * @file optix_init.cpp
 * @brief OptiX Function Table initialization
 * 
 * This file provides the definition of g_optixFunctionTable which is declared
 * as extern in optix_stubs.h
 */

#define OPTIX_STUBS_DEFINE_GLOBAL_TABLE
#include <optix_stubs.h>

// The global function table is defined here
// optix_stubs.h declares it as extern, we define it here
OptixFunctionTable g_optixFunctionTable = {};

