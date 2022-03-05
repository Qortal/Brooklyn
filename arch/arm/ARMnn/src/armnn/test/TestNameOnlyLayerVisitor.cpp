//
// Copyright © 2017 Arm Ltd and Contributors. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TestNameOnlyLayerVisitor.hpp"

#include <Network.hpp>

#include <doctest/doctest.h>

namespace
{

#define TEST_CASE_CHECK_LAYER_VISITOR_NAME(name, testName) \
TEST_CASE(#testName) \
{ \
    Test##name##LayerVisitor visitor("name##Layer"); \
    armnn::NetworkImpl net; \
    armnn::IConnectableLayer *const layer = net.Add##name##Layer("name##Layer"); \
    layer->ExecuteStrategy(visitor); \
}

#define TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(name, testName) \
TEST_CASE(#testName) \
{ \
    Test##name##LayerVisitor visitor; \
    armnn::NetworkImpl net; \
    armnn::IConnectableLayer *const layer = net.Add##name##Layer(); \
    layer->ExecuteStrategy(visitor); \
}

} // anonymous namespace

TEST_SUITE("TestNameOnlyLayerVisitor")
{
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Addition, CheckAdditionLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Addition, CheckAdditionLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Dequantize, CheckDequantizeLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Dequantize, CheckDequantizeLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Division, CheckDivisionLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Division, CheckDivisionLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Floor, CheckFloorLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Floor, CheckFloorLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Maximum, CheckMaximumLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Maximum, CheckMaximumLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Merge, CheckMergeLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Merge, CheckMergeLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Minimum, CheckMinimumLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Minimum, CheckMinimumLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Multiplication, CheckMultiplicationLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Multiplication, CheckMultiplicationLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Prelu, CheckPreluLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Prelu, CheckPreluLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Quantize, CheckQuantizeLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Quantize, CheckQuantizeLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Rank, CheckRankLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Rank, CheckRankLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Subtraction, CheckSubtractionLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Subtraction, CheckSubtractionLayerVisitorNameNullptr)
TEST_CASE_CHECK_LAYER_VISITOR_NAME(Switch, CheckSwitchLayerVisitorName)
TEST_CASE_CHECK_LAYER_VISITOR_NAME_NULLPTR(Switch, CheckSwitchLayerVisitorNameNullptr)

}
