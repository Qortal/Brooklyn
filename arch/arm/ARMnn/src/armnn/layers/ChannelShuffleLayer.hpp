//
// Copyright © 2021 Arm Ltd and Contributors. All rights reserved.
// SPDX-License-Identifier: MIT
//
#pragma once

#include "LayerWithParameters.hpp"

namespace armnn
{
class ChannelShuffleLayer : public LayerWithParameters<ChannelShuffleDescriptor>
{
public:
    ARMNN_NO_DEPRECATE_WARN_BEGIN
    void Accept(ILayerVisitor& visitor) const override;
    ARMNN_NO_DEPRECATE_WARN_END

    /// Creates a dynamically-allocated copy of this layer.
    /// @param graph The graph into which this layer is being cloned
    ChannelShuffleLayer* Clone(Graph& graph) const override;

    /// Makes a workload for the ChannelShuffle type.
    /// @param factory The workload factory which will create the workload
    /// @return A pointer to the created workload, or nullptr if not created.
    virtual std::unique_ptr<IWorkload> CreateWorkload(const IWorkloadFactory& factory) const override;

    /// Check if the input tensor shape(s)
    /// will lead to a valid configuration of @ref ChannelShuffleLayer.
    /// @param [in] shapeInferenceMethod Indicates if output shape shall be overwritten or just validated.
    void ValidateTensorShapesFromInputs() override;

    // TODO Do you need to create an InferOutputShapes function for ChannelShuffle?
protected:
    ChannelShuffleLayer(const ChannelShuffleDescriptor& param, const char* name);

    ~ChannelShuffleLayer() = default;
};

} // namespace