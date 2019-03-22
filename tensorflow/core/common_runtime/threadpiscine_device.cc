/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/core/common_runtime/threadpiscine_device.h"

#include "tensorflow/core/common_runtime/local_device.h"
#include "tensorflow/core/framework/allocator.h"
#include "tensorflow/core/framework/allocator_registry.h"
#include "tensorflow/core/framework/device_base.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/tensor.pb_text.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/graph/types.h"
#include "tensorflow/core/lib/hash/hash.h"
#include "tensorflow/core/platform/tracing.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session_options.h"

namespace tensorflow {

ThreadPiscineDevice::ThreadPiscineDevice(const SessionOptions& options,
                                   const string& name, Bytes memory_limit,
                                   const DeviceLocality& locality,
                                   Allocator* allocator)
    : LocalDevice(options, Device::BuildDeviceAttributes(
                               name, DEVICE_FPGA, memory_limit, locality)),
      allocator_(allocator) {}

ThreadPiscineDevice::~ThreadPiscineDevice() {}

void ThreadPiscineDevice::Compute(OpKernel* op_kernel, OpKernelContext* context) {
  // When TraceMe profiling is off (which is the default), the
  // following TraceMe constructor is simply a conditional test of
  // false value. Measurements show that its overhead is negligible.
  port::Tracing::TraceMe trace_me(op_kernel->name(), op_kernel->type_string(),
                                  op_kernel->IsExpensive());
  if (port::Tracing::IsActive()) {
    // TODO(pbar) We really need a useful identifier of the graph node.
    const uint64 id = Hash64(op_kernel->name());
    port::Tracing::ScopedActivity region(port::Tracing::EventCategory::kCompute,
                                         id);
    op_kernel->Compute(context);
  } else {
    op_kernel->Compute(context);
  }
}

Allocator* ThreadPiscineDevice::GetAllocator(AllocatorAttributes attr) {
  return allocator_;
}

Status ThreadPiscineDevice::MakeTensorFromProto(
    const TensorProto& tensor_proto, const AllocatorAttributes alloc_attrs,
    Tensor* tensor) {
  if (tensor_proto.dtype() > 0 && tensor_proto.dtype() <= DataType_MAX) {
    Tensor parsed(tensor_proto.dtype());
    if (parsed.FromProto(cpu_allocator(), tensor_proto)) {
      *tensor = std::move(parsed);
      return Status::OK();
    }
  }
  return errors::InvalidArgument("Cannot parse tensor from proto: ",
                                 ProtoDebugString(tensor_proto));
}

}  // namespace tensorflow
