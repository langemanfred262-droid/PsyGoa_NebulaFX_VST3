#include "controller.h"
#include "processor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/main/pluginfactory.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
namespace Nebula {
enum { Gate=0, GateRate, GateDepth, Cutoff, Resonance, Sweep, Phaser, Flanger, ModRate, Delay, DelayFeedback, Reverb, Wet, FmMix, FmRatio, FmIndex, Output };

tresult PLUGIN_API Controller::initialize(FUnknown* c) {
    tresult r=EditController::initialize(c); if(r!=kResultOk)return r;
    parameters.addParameter(STR16("Trace Gate"),nullptr,1,Gate,ParameterInfo::kCanAutomate,0.0);
    parameters.addParameter(STR16("Gate Division"),nullptr,4,GateRate,ParameterInfo::kCanAutomate,0.5);
    parameters.addParameter(STR16("Gate Depth"),nullptr,0,GateDepth,ParameterInfo::kCanAutomate,1.0);
    parameters.addParameter(STR16("Acid Cutoff"),nullptr,0,Cutoff,ParameterInfo::kCanAutomate,0.72);
    parameters.addParameter(STR16("Resonance"),nullptr,0,Resonance,ParameterInfo::kCanAutomate,0.55);
    parameters.addParameter(STR16("Sweep Amount"),nullptr,0,Sweep,ParameterInfo::kCanAutomate,0.35);
    parameters.addParameter(STR16("Phaser"),nullptr,0,Phaser,ParameterInfo::kCanAutomate,0.2);
    parameters.addParameter(STR16("Flanger"),nullptr,0,Flanger,ParameterInfo::kCanAutomate,0.15);
    parameters.addParameter(STR16("Mod Rate"),nullptr,0,ModRate,ParameterInfo::kCanAutomate,0.3);
    parameters.addParameter(STR16("Delay Division"),nullptr,4,Delay,ParameterInfo::kCanAutomate,0.25);
    parameters.addParameter(STR16("Delay Feedback"),nullptr,0,DelayFeedback,ParameterInfo::kCanAutomate,0.45);
    parameters.addParameter(STR16("Reverb"),nullptr,0,Reverb,ParameterInfo::kCanAutomate,0.22);
    parameters.addParameter(STR16("FX Wet"),nullptr,0,Wet,ParameterInfo::kCanAutomate,0.35);
    parameters.addParameter(STR16("FM Lead Mix"),nullptr,0,FmMix,ParameterInfo::kCanAutomate,0.5);
    parameters.addParameter(STR16("FM Ratio"),nullptr,0,FmRatio,ParameterInfo::kCanAutomate,0.25);
    parameters.addParameter(STR16("FM Index"),nullptr,0,FmIndex,ParameterInfo::kCanAutomate,0.35);
    parameters.addParameter(STR16("Output"),nullptr,0,Output,ParameterInfo::kCanAutomate,0.8);
    return kResultOk;
}
}

BEGIN_FACTORY_DEF("Nebula Audio Labs","https://example.invalid","support@example.invalid")
    DEF_CLASS2(INLINE_UID_FROM_FUID(Nebula::ProcessorUID),
               PClassInfo::kManyInstances,
               kVstAudioEffectClass,
               "PsyGoa NebulaFX",
               Vst::kDistributable,
               "Fx|Filter",
               "Nebula Audio Labs",
               "2.0.0",
               kVstVersionString,
               Nebula::Processor::createInstance)
    DEF_CLASS2(INLINE_UID_FROM_FUID(Nebula::ControllerUID),
               PClassInfo::kManyInstances,
               kVstComponentControllerClass,
               "PsyGoa NebulaFX Controller",
               0,
               "",
               "Nebula Audio Labs",
               "2.0.0",
               kVstVersionString,
               Nebula::Controller::createInstance)
END_FACTORY
