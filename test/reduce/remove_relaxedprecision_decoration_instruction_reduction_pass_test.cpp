// Copyright (c) 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "reduce_test_util.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "source/reduce/remove_relaxedprecision_decoration_instruction_reduction_pass.h"

namespace spvtools {
namespace reduce {
namespace {

TEST(RemoveRelaxedPrecisionDecorationInstructionReductionPassTest,
     NothingToRemove) {
  const std::string source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, source, kReduceAssembleOption);
  const auto pass =
      TestSubclass<RemoveRelaxedPrecisionDecorationInstructionReductionPass>(
          env);
  const auto ops = pass.WrapGetAvailableOpportunities(context.get());
  ASSERT_EQ(0, ops.size());
}

TEST(RemoveOpnameInstructionReductionPassTest, RemoveSingleDecoration) {
  const std::string prologue = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
  )";

  const std::string epilogue = R"(
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpReturn
               OpFunctionEnd
  )";

  const std::string original = prologue + R"(
               OpDecorate %8 RelaxedPrecision
  )" + epilogue;

  const std::string expected = prologue + epilogue;

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, original, kReduceAssembleOption);
  const auto pass =
      TestSubclass<RemoveRelaxedPrecisionDecorationInstructionReductionPass>(
          env);
  const auto ops = pass.WrapGetAvailableOpportunities(context.get());
  ASSERT_EQ(1, ops.size());
  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();

  CheckEqual(env, expected, context.get());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools