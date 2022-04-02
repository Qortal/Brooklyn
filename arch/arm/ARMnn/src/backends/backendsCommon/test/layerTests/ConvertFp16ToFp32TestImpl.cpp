//
// Copyright © 2017 Arm Ltd and Contributors. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "ConvertFp16ToFp32TestImpl.hpp"

#include <Half.hpp>


#include <armnnTestUtils/TensorCopyUtils.hpp>
#include <armnnTestUtils/WorkloadTestUtils.hpp>

#include <armnnTestUtils/TensorHelpers.hpp>

LayerTestResult<float, 4> SimpleConvertFp16ToFp32Test(
    armnn::IWorkloadFactory& workloadFactory,
    const armnn::IBackendInternal::IMemoryManagerSharedPtr& memoryManager,
    const armnn::ITensorHandleFactory& tensorHandleFactory)
{
    IgnoreUnused(memoryManager);
    using namespace half_float::literal;

    const armnn::TensorInfo inputTensorInfo({1, 3, 2, 3}, armnn::DataType::Float16);
    const armnn::TensorInfo outputTensorInfo({1, 3, 2, 3}, armnn::DataType::Float32);

    std::vector<armnn::Half> input =
    {
        -37.5_h, -15.2_h, -8.76_h, -2.0_h, -1.5_h, -1.3_h, -0.5_h, -0.4_h, 0.0_h,
        1.0_h, 0.4_h, 0.5_h, 1.3_h, 1.5_h, 2.0_h, 8.76_h, 15.2_h, 37.5_h
    };

    std::vector<float> expectedOutput =
    {
        -37.5f, -15.2f, -8.76f, -2.0f, -1.5f, -1.3f, -0.5f, -0.4f, 0.0f,
        1.0f, 0.4f, 0.5f, 1.3f, 1.5f, 2.0f, 8.76f, 15.2f, 37.5f
    };

    std::vector<float> actualOutput(outputTensorInfo.GetNumElements());

    std::unique_ptr<armnn::ITensorHandle> inputHandle = tensorHandleFactory.CreateTensorHandle(inputTensorInfo);
    std::unique_ptr<armnn::ITensorHandle> outputHandle = tensorHandleFactory.CreateTensorHandle(outputTensorInfo);

    armnn::ConvertFp16ToFp32QueueDescriptor data;
    armnn::WorkloadInfo info;
    AddInputToWorkload(data, info, inputTensorInfo, inputHandle.get());
    AddOutputToWorkload(data, info, outputTensorInfo, outputHandle.get());

    std::unique_ptr<armnn::IWorkload> workload = workloadFactory.CreateWorkload(armnn::LayerType::ConvertFp16ToFp32,
                                                                                data,
                                                                                info);

    inputHandle->Allocate();
    outputHandle->Allocate();

    CopyDataToITensorHandle(inputHandle.get(), input.data());

    workload->Execute();

    CopyDataFromITensorHandle(actualOutput.data(), outputHandle.get());

    return LayerTestResult<float, 4>(actualOutput,
                                     expectedOutput,
                                     outputHandle->GetShape(),
                                     outputTensorInfo.GetShape());
}
