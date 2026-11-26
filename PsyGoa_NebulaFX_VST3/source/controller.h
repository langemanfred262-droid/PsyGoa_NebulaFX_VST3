#pragma once
#include "public.sdk/source/vst/vsteditcontroller.h"
namespace Nebula {
class Controller final : public Steinberg::Vst::EditController {
public:
 static Steinberg::FUnknown* createInstance(void*) { return (Steinberg::Vst::IEditController*)new Controller; }
 Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
};
}
