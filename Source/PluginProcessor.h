/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
// #include <torch/torch.h>
// #include <torch/script.h>

#include "../../living-looper-core/LLModel.hpp"

//#include <torch/csrc/jit/runtime/graph_executor.h>
//#include <torch/csrc/jit/serialization/export.h>
#include "EngineUpdater.h"
#include "Param.h"

// TODO: need to set FIFO lengths / size var on engine change, from model block_size...
#define DEFAULT_FIFO_LENGTH 2048
#define MAX_LOOPS 10
#define EPSILON 0.0000001

namespace rave_parameters
{
    // const String param_name_temperature {"temperature"};
    // const String param_name_wetgain {"wet_gain"};
    // const String param_name_drygain {"dry_gain"};
    // const String param_name_toggleprior {"toggle_prior"};
    const String param_name_importbutton {"import_button"};
}
//==============================================================================
/**
*/
class LivingLooperAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    LivingLooperAudioProcessor();
    ~LivingLooperAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    auto createParameterLayout() -> AudioProcessorValueTreeState::ParameterLayout;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void parameterChanged (const String& parameterID, float newValue) override;
    
    auto mute() -> void;
    auto unmute() -> void;
    auto getIsMuted() -> const bool;
    void updateEngine(const std::string modelFile);
    
    std::unique_ptr<LivingLooper::LLModel> model;

    int mLoopSelect;
    std::queue<int> qLoopSelect;
    std::queue<int> qNLoops;

    // std::vector<Param<float>*> mParams;
    std::map<String, Param<float>*> mParams;

    juce::AudioProcessorValueTreeState mAVTS;

private:
    
    mutable CriticalSection mEngineUpdateMutex;
    std::unique_ptr<FileChooser> fc;
    std::unique_ptr<juce::ThreadPool> mEngineThreadPool;
    
    std::vector<float> inBuffer;
    std::vector<float> outBuffer;
    size_t inIdx, outIdx;

    bool first_block_done;

    // std::atomic<float>* mWetGainParameterValue;
    // std::atomic<float>* mDryGainParameterValue;
    std::atomic<bool> mIsMuted { true };
    
    enum class muting : int
    {
        ignore = 0,
        mute,
        unmute
    };
    
    std::atomic<muting> mFadeScheduler { muting::mute };
    LinearSmoothedValue<float> mSmoothedFadeInOut;
    
    // LinearSmoothedValue<float> mSmoothedWetGain;
    // LinearSmoothedValue<float> mSmoothedDryGain;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LivingLooperAudioProcessor)
};
