#pragma once

#include <array>

/**
 * Allows to get any STL-suitable container from a constexpr function
 * and convert it into std::array of same size at compile-time.
 *
 * Create wrappers of use it directly like this:
 * constexpr auto CubeVertsArray = ConstexprDynamicContainerToArray<GenerateCubeMeshVertices, false>();
 */
template<auto* ContainerGenFunc, auto... Args>
constexpr auto ConstexprDynamicContainerToArray()
{
    auto Container = ContainerGenFunc(Args...);
    constexpr auto ContainerSize = ContainerGenFunc(Args...).size();

    std::array<typename decltype(ContainerGenFunc(Args...))::value_type, ContainerSize> Result{};
    std::copy(Container.begin(), Container.end(), Result.begin());

    return Result;
}