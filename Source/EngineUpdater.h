/*
  ==============================================================================

    EngineUpdater.h
    Author: Victor Shepardson & Andrew Fyfe

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LivingLooperAudioProcessor; // forward declare

//  ==============================================================================

class UpdateEngineJob  : public juce::ThreadPoolJob
{
public:
    explicit UpdateEngineJob(
      LivingLooperAudioProcessor& processor, const std::string modelPath);
    virtual ~UpdateEngineJob();
    virtual auto runJob() -> JobStatus;
    bool waitForFadeOut(size_t waitTimeMs);
    
private:
    LivingLooperAudioProcessor& mProcessor;
    const std::string mModelFile;
    // Prevent uncontrolled usage
    UpdateEngineJob(const UpdateEngineJob&);
    UpdateEngineJob& operator=(const UpdateEngineJob&);
    
};

//  ==============================================================================
