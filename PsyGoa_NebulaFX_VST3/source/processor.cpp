#include "processor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstevents.h"
#include <algorithm>
#include <cmath>

using namespace Steinberg;
using namespace Steinberg::Vst;
namespace Nebula {
enum { Gate=0, GateRate, GateDepth, Cutoff, Resonance, Sweep, Phaser, Flanger, ModRate, Delay, DelayFeedback, Reverb, Wet, FmMix, FmRatio, FmIndex, Output };
static constexpr double PI=3.14159265358979323846;
Processor::Processor() {
    setControllerClass(ControllerUID);
    p={0.0,0.5,1.0,0.72,0.55,0.35,0.2,0.15,0.3,0.25,0.45,0.22,0.35,0.5,0.25,0.35,0.8};
}
tresult PLUGIN_API Processor::initialize(FUnknown* context) {
    auto r=AudioEffect::initialize(context); if(r!=kResultOk)return r;
    addAudioInput(STR16("Stereo In"),SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"),SpeakerArr::kStereo);
    addEventInput(STR16("MIDI In"),16);
    return kResultOk;
}
tresult PLUGIN_API Processor::setupProcessing(ProcessSetup& s) {
    sr=s.sampleRate;
    delayL.assign((size_t)(sr*3.1),0.f); delayR.assign((size_t)(sr*3.1),0.f);
    return AudioEffect::setupProcessing(s);
}
float Processor::readParam(int id,float fallback) const { return id>=0 && id<(int)p.size()?(float)p[id]:fallback; }

void Processor::handleEvents(IEventList* events) {
    if(!events)return;
    Event e;
    for(int32 i=0;i<events->getEventCount();++i) {
        if(events->getEvent(i,e)!=kResultOk)continue;
        if(e.type==Event::kNoteOnEvent && e.noteOn.velocity>0.f) {
            noteHz=(float)(440.0*std::pow(2.0,(e.noteOn.pitch-69)/12.0));
            velocity=e.noteOn.velocity; noteOn=true;
        } else if(e.type==Event::kNoteOffEvent || (e.type==Event::kNoteOnEvent && e.noteOn.velocity<=0.f)) {
            noteOn=false;
        }
    }
}
tresult PLUGIN_API Processor::process(ProcessData& d) {
    if(d.inputParameterChanges) {
        for(int32 i=0;i<d.inputParameterChanges->getParameterCount();++i) {
            auto* q=d.inputParameterChanges->getParameterData(i); if(!q)continue;
            int32 sampleOffset=0; ParamValue v=0;
            if(q->getPoint(q->getPointCount()-1,sampleOffset,v)==kResultOk && q->getParameterId()<p.size())
                p[q->getParameterId()]=std::clamp(v,0.0,1.0);
        }
    }
    if(d.processContext) {
        if(d.processContext->state&ProcessContext::kTempoValid)tempo=d.processContext->tempo;
        playing=(d.processContext->state&ProcessContext::kPlaying)!=0;
    }
    handleEvents(d.inputEvents);
    if(d.numOutputs<1 || !d.outputs[0].channelBuffers32)return kResultOk;
    float* outL=d.outputs[0].channelBuffers32[0]; float* outR=d.outputs[0].channelBuffers32[1];
    float* inL=(d.numInputs&&d.inputs[0].channelBuffers32)?d.inputs[0].channelBuffers32[0]:nullptr;
    float* inR=(d.numInputs&&d.inputs[0].channelBuffers32)?d.inputs[0].channelBuffers32[1]:nullptr;

    const float gate=readParam(Gate), depth=readParam(GateDepth,1.f);
    const int divisions[5]={1,2,4,8,16};
    const int gateDiv=divisions[std::min(4,(int)(readParam(GateRate,.5f)*5))];
    const float cutoff=readParam(Cutoff,.72f), res=readParam(Resonance,.55f);
    const float wet=readParam(Wet,.35f), fb=std::clamp(readParam(DelayFeedback,.45f),0.f,.92f);
    const float delayBeats[5]={.25f,.375f,.5f,.75f,1.f};
    const float delayDiv=delayBeats[std::min(4,(int)(readParam(Delay,.25f)*5))];
    const size_t dlen=std::max<size_t>(1,(size_t)(sr*60.0/std::max(40.0,tempo)*delayDiv));
    const float fc=std::clamp(100.f+cutoff*17500.f,80.f,(float)(sr*.42));
    const float baseA=1.f-std::exp(-2.f*(float)PI*fc/(float)sr);
    const float fmRatio=0.5f+readParam(FmRatio,.25f)*7.5f;
    const float fmIndex=readParam(FmIndex,.35f)*12.f;
    const float fmMix=readParam(FmMix,.5f);
    const float attack=1.f-std::exp(-1.f/(.008f*(float)sr));
    const float release=1.f-std::exp(-1.f/(.18f*(float)sr));

    for(int32 i=0;i<d.numSamples;++i) {
        float l=inL?inL[i]:0.f, r=inR?inR[i]:0.f;
        if(noteOn)ampEnv+=(1.f-ampEnv)*attack; else ampEnv+=(0.f-ampEnv)*release;
        float mod=std::sin((float)modPhase*2.f*(float)PI);
        float fm=std::sin((float)carrierPhase*2.f*(float)PI + mod*fmIndex)*ampEnv*velocity;
        carrierPhase+=noteHz/sr; modPhase+=(noteHz*fmRatio)/sr;
        if(carrierPhase>=1.0)carrierPhase-=std::floor(carrierPhase);
        if(modPhase>=1.0)modPhase-=std::floor(modPhase);
        l+=fm*fmMix*.35f; r+=fm*fmMix*.35f;

        const float gateHz=(float)(tempo/60.0)*gateDiv;
        float gateEnv=1.f;
        if(gate>.5f && playing)gateEnv=(gatePhase<.5)?1.f:1.f-depth;
        gatePhase+=gateHz/sr; if(gatePhase>=1.0)gatePhase-=std::floor(gatePhase);

        float sweep=(.5f+.5f*std::sin((float)lfoPhase*2.f*(float)PI))*readParam(Sweep,.35f);
        float a=std::clamp(baseA*(.4f+cutoff*.75f+sweep*.8f),.0001f,.98f);
        zL+=a*(l-zL); zR+=a*(r-zR);
        float acidL=std::tanh(zL*(1.f+res*16.f)), acidR=std::tanh(zR*(1.f+res*16.f));
        float motion=std::sin((float)lfoPhase*2.f*(float)PI)*readParam(ModRate,.3f);
        phL=.72f*phL+acidL; phR=.72f*phR+acidR;
        float pL=acidL+(acidL-phL)*readParam(Phaser,.2f)*motion;
        float pR=acidR+(acidR-phR)*readParam(Phaser,.2f)*motion;

        float flangeMs=3.0f+2.5f*(1.f+motion);
        const float flangeSeconds=flangeMs*.001f*(float)sr;
        const float maxFlangeOffset=(float)(delayL.size()>1?delayL.size()-1:1);
        size_t flangeOffset=(size_t)std::clamp(flangeSeconds,1.f,maxFlangeOffset);
        size_t flangeRead=((size_t)delayPos+delayL.size()-flangeOffset)%delayL.size();
        float fL=pL+delayL[flangeRead]*readParam(Flanger,.15f)*.65f;
        float fR=pR+delayR[flangeRead]*readParam(Flanger,.15f)*.65f;

        size_t echoRead=((size_t)delayPos+delayL.size()-dlen)%delayL.size();
        float eL=delayL[echoRead], eR=delayR[echoRead];
        float xL=fL+eR*fb, xR=fR+eL*fb;
        delayL[(size_t)delayPos]=fL+eR*fb; delayR[(size_t)delayPos]=fR+eL*fb;
        delayPos+=1.0; if(delayPos>=delayL.size())delayPos=0.0;

        revL=.79f*revL+.21f*(xL+revR*.24f);
        revR=.79f*revR+.21f*(xR+revL*.24f);
        float dryL=fL*(1.f-wet), dryR=fR*(1.f-wet);
        float fxL=(xL*.58f+revL*readParam(Reverb,.22f))*wet;
        float fxR=(xR*.58f+revR*readParam(Reverb,.22f))*wet;
        float gain=readParam(Output,.8f);
        float yL=std::clamp((dryL+fxL)*gateEnv*gain,-1.f,1.f);
        float yR=std::clamp((dryR+fxR)*gateEnv*gain,-1.f,1.f);
        outL[i]=std::isfinite(yL)?yL:0.f; outR[i]=std::isfinite(yR)?yR:0.f;
        lfoPhase+=.05+readParam(ModRate,.3f)*3.5f; lfoPhase/=sr;
        if(lfoPhase>=1.0)lfoPhase-=1.0;
    }
    d.outputs[0].silenceFlags=0;
    return kResultOk;
}
tresult PLUGIN_API Processor::setState(IBStream* s) {
    if(!s)return kInvalidArgument;
    int32 n=0; if(s->read(&n,sizeof(n))!=kResultOk)return kResultFalse;
    if(n==(int32)p.size()) { if(s->read(p.data(),sizeof(double)*p.size())!=kResultOk)return kResultFalse; }
    return kResultOk;
}
tresult PLUGIN_API Processor::getState(IBStream* s) {
    if(!s)return kInvalidArgument;
    int32 n=(int32)p.size(); if(s->write(&n,sizeof(n))!=kResultFalse;
    return s->write(p.data(),sizeof(double)*p.size());
}
}
