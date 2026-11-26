#pragma once
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "plugincids.h"
#include "pluginterfaces/vst/ivstevents.h"
#include <array>
#include <vector>
#include <cmath>

namespace Nebula {
class Processor final : public Steinberg::Vst::AudioEffect {
public:
    Processor();
    static Steinberg::FUnknown* createInstance(void*) { return (Steinberg::Vst::IAudioProcessor*)new Processor; }
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
private:
    double sr=44100.0, gatePhase=0.0, lfoPhase=0.0, delayPos=0.0;
    float zL=0, zR=0, phL=0, phR=0, revL=0, revR=0;
    std::vector<float> delayL, delayR;
    std::array<double,17> p{};
    double tempo=128.0;
    bool playing=false, noteOn=false;
    double carrierPhase=0.0, modPhase=0.0;
    float noteHz=440.f, velocity=0.f, ampEnv=0.f;
    float readParam(int id,float fallback=0.f) const;
    void handleEvents(Steinberg::Vst::IEventList* events);
};
}
