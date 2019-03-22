#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/user_ops/fpga_mag.h"

using namespace tensorflow;

REGISTER_OP("VectAdd")
.Input("a: int32")
.Input("b: int32")
.Output("ret: int32")
.SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
                c->set_output(0, c->input(0));
                return Status::OK();
                });

using namespace tensorflow;

class VectAddOp : public OpKernel {
        public:
                explicit VectAddOp(OpKernelConstruction* context) : OpKernel(context) {}

                void Compute(OpKernelContext* context) override {
                        const Tensor& input_tensor_a = context->input(0);
                        auto input_a = input_tensor_a.flat<int32>();
                        const Tensor& input_tensor_b = context->input(1);
                        auto input_b = input_tensor_b.flat<int32>();

                        Tensor* output_tensor = NULL;
                        OP_REQUIRES_OK(context, context->allocate_output(0, input_tensor_a.shape(),
                                                &output_tensor));
                        auto output_flat = output_tensor->flat<int32>();

                        const int N = input_a.size();
                        printf("N= %d\n", N);
                        init_device();
                        for (int i = 0; i < N; i++) {
                                printf("input_a(%d)=%d\n", i, input_a(i));
                                printf("input_b(%d)=%d\n", i, input_b(i));
                                output_flat(i) = calc_vector_add(input_a(i), input_b(i));
                                printf("output_flat(%d)=%d\n", i, output_flat(i));
                        }

                        // Preserve the first input value if possible.
                        close_device();
                }
};

REGISTER_KERNEL_BUILDER(Name("VectAdd").Device(DEVICE_FPGA), VectAddOp);
