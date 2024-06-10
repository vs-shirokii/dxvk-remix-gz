/*
* Copyright (c) 2023, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include "dxvk_include.h"
#include "dxvk_context.h"
#include "rtx_resources.h"

namespace dxvk {

  class DxvkDevice;

  class DxvkBloom: public RtxPass {
    
  public:
    explicit DxvkBloom(DxvkDevice* device);
    ~DxvkBloom();

    DxvkBloom(const DxvkBloom&) = delete;
    DxvkBloom(DxvkBloom&&) noexcept = delete;
    DxvkBloom& operator=(const DxvkBloom&) = delete;
    DxvkBloom& operator=(DxvkBloom&&) noexcept = delete;

    void dispatch(
      Rc<RtxContext> ctx,
      Rc<DxvkSampler> linearSampler,
      const Resources::Resource& inOutColorBuffer);

    bool isEnabled() const { return enable() && burnIntensity() > 0.f; }

    void showImguiSettings();

  private:
    void dispatchDownsampleStep(
      Rc<DxvkContext> ctx,
      const Rc<DxvkSampler>& linearSampler,
      const Resources::Resource& inputBuffer,
      const Resources::Resource& outputBuffer,
      bool initial);

    void dispatchUpsampleStep(
      Rc<DxvkContext> ctx,
      const Rc<DxvkSampler>& linearSampler,
      const Resources::Resource& inputBuffer,
      const Resources::Resource& outputBuffer);

    void dispatchComposite(
      Rc<DxvkContext> ctx,
      const Rc<DxvkSampler> &linearSampler,
      const Resources::Resource& inOutColorBuffer,
      const Resources::Resource& bloomBuffer);

    virtual void createTargetResource(Rc<DxvkContext>& ctx, const VkExtent3D& targetExtent) override;
    virtual void releaseTargetResource() override;

    virtual bool isActive() override;

    Rc<vk::DeviceFn> m_vkd;

    // Each image is 1/2 resolution of the previous.
    // Here, 5 steps are chosen: so the last image would be 1/(2^5) = 1/32 of the target resolution,
    // and at 4K resolution, it's ~67 pixels height, which is fine enough -- as on other hand,
    // we would like to keep the amount of steps as few as possible.
    Resources::Resource m_bloomBuffer[5] = {};

    RTX_OPTION("rtx.bloom", bool, enable, true, "Enable bloom - glowing halos around intense, bright areas.");
    RTX_OPTION("rtx.bloom", float, burnIntensity, 1.0f, "Amount of bloom to add to the final image.");
    RTX_OPTION("rtx.bloom", float, luminanceThreshold, 0.25f,
               "Adjust the bloom threshold to suppress blooming of the dim areas. "
               "Pixels with luminance lower than the threshold are multiplied by "
               "the weight value that smoothly transitions from 1.0 (at luminance=threshold) to 0.0 (at luminance=0).");
  };
  
}
