/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#define PRINT(S) std::cout << S << std::endl

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LivingLooperAudioProcessor::LivingLooperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
        #endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    #endif
    ), mAVTS(
        *this, nullptr, Identifier("LLValueTree"), createParameterLayout())
#endif
{
    // FloatFifo* inputfifo_p=&mInputFifo;
    // FloatFifo* outputfifo_p=&mOutputFifo;
    // FloatFifo* hostfifo_p=&mHostFifo;
    
    // mHostFifoBuffer.resize(DEFAULT_FIFO_LENGTH);
    // mOutputFifoBuffer.resize(DEFAULT_FIFO_LENGTH * MAX_LOOPS);
    // FifoBuffer_init(hostfifo_p,DEFAULT_FIFO_LENGTH,float, mHostFifoBuffer.data());
    // FifoBuffer_init(inputfifo_p,DEFAULT_FIFO_LENGTH,float, mInputFifoBuffer);
    // FifoBuffer_init(outputfifo_p,DEFAULT_FIFO_LENGTH,float, mOutputFifoBuffer.data());
    inIdx = outIdx = 0;
    first_block_done = false;
    
    setLatencySamples(DEFAULT_FIFO_LENGTH);
    
    // mTemperatureParameterValue = mAVTS.getRawParameterValue(rave_parameters::param_name_temperature);
    mWetGainParameterValue = mAVTS.getRawParameterValue(rave_parameters::param_name_wetgain);
    mDryGainParameterValue = mAVTS.getRawParameterValue(rave_parameters::param_name_drygain);
    // mTogglePriorParameterValue = mAVTS.getRawParameterValue(rave_parameters::param_name_toggleprior);
    
    mEngineThreadPool = std::make_unique<ThreadPool>(1);
    model.reset(new LivingLooper::LLModel());
    
    mAVTS.addParameterListener(rave_parameters::param_name_importbutton, this);

}

LivingLooperAudioProcessor::~LivingLooperAudioProcessor()
{
}

//==============================================================================
const juce::String LivingLooperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LivingLooperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LivingLooperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LivingLooperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LivingLooperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LivingLooperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LivingLooperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LivingLooperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LivingLooperAudioProcessor::getProgramName (int index)
{
    return {};
}

void LivingLooperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LivingLooperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // FloatFifo* hostfifo_p=&mHostFifo;
    // FloatFifo* inputfifo_p=&mInputFifo;
    // FloatFifo* outputfifo_p=&mOutputFifo;
    
    // FifoBuffer_flush(inputfifo_p);
    // FifoBuffer_flush(outputfifo_p);
    mSmoothedWetGain.reset(sampleRate, 0.1);
    mSmoothedDryGain.reset(sampleRate, 0.1);
    mSmoothedFadeInOut.reset(sampleRate, 0.2);

    PRINT('prepareToPlay');
    if(model->loaded){
        PRINT('model loaded');
        PRINT(model->block_size);
        inBuffer.resize(model->block_size);
        outBuffer.resize(model->block_size);
        inBuffer.clear();
        outBuffer.clear();
    }
    
    // mHostFifoBuffer.resize(static_cast<size_t>(samplesPerBlock));
    // FifoBuffer_init(hostfifo_p,samplesPerBlock,float, mHostFifoBuffer.data());
    
    // const int size = (samplesPerBlock<mFifoSize)?mFifoSize:samplesPerBlock;

    // mHostFifoBuffer.resize(static_cast<size_t>(size));
    // FifoBuffer_init(hostfifo_p,size,float, mHostFifoBuffer.data());
    
    // TODO: number of loops...
    // int n_loops = 5;
    // mOutputFifoBuffer.resize(static_cast<size_t>(size*n_loops));
    // FifoBuffer_init(outputfifo_p,size,float, mOutputFifoBuffer.data());
    
    // for (int i = 0; i < mFifoSize; ++i) {
        // mTempBuffer[i] = 0.f;
    // }
    
    // for (int i = 0; i < size*n_loops; ++i) {
        // FifoBuffer_write(outputfifo_p, 0.f);
    // }

    mLoopSelect = 0;
}

void LivingLooperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LivingLooperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LivingLooperAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, 
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // TODO: number of loops...
    // int n_loops = 5;
    
    const int nSamples = buffer.getNumSamples();
    const int nChannels = buffer.getNumChannels();
    
    // PRINT("process");
    if (!model->loaded) return;
    PRINT("model loaded");

    int n_loops = model->n_loops;
    int model_block = model->block_size;
    int host_block = nSamples;

    inBuffer.assign(model_block, 0.0);
    outBuffer.assign(model_block * n_loops, 0.0);

    PRINT("model access");

    for (auto msgd : midiMessages){
        auto msg = msgd.getMessage();
        // std::cout << msg.getDescription() << std::endl;

        if (msg.isNoteOn()){
            // TODO: oneshot
            // auto loop_num = msg.getNoteNumber()-59;
            auto loop_num = (msg.getNoteNumber()+128-59)%(n_loops+1);
            // if (loop_num > 0 && loop_num <= n_loops){
            std::cout << "switch to loop: " << loop_num << std::endl;
            mLoopSelect = loop_num;
            // } else {
            //     std::cout << "out of range: " << loop_num << std::endl;
            // }
        }
        else if (msg.isNoteOff()){
            std::cout << "stop record" << std::endl;
            mLoopSelect = 0;
        }
        qLoopSelect.push(mLoopSelect);
        
    }
    // mActiveLoopLabel.setText("0", NotificationType::dontSendNotification);


    PRINT("MIDI handled");
    
    // for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    //     buffer.clear(i, 0, nSamples);
    // }
    // PRINT("buffer cleared");

    
    // FloatFifo* hostfifo_p=&mHostFifo;
    // FloatFifo* inputfifo_p=&mInputFifo;
    // FloatFifo* outputfifo_p=&mOutputFifo;
    
    float* channelL;
    float* channelR;
    
    if (nChannels > 0 && nChannels < 2) {
        channelL = buffer.getWritePointer(0);
        channelR = buffer.getWritePointer(0);
    } else if(nChannels > 1 && nChannels < 3) {
        channelL = buffer.getWritePointer(0);
        channelR = buffer.getWritePointer(1);
    }
    else {
        buffer.clear();
        PRINT("nChannels unsupported");
        return;
    }
    
    // const float temperatureVal = mTemperatureParameterValue->load();
    // const bool usePrior = static_cast<bool>(mTogglePriorParameterValue->load());
    mSmoothedWetGain.setTargetValue(mWetGainParameterValue->load());
    mSmoothedDryGain.setTargetValue(mDryGainParameterValue->load());
    
    const muting muteConfig = mFadeScheduler.load();

    if(muteConfig == muting::mute)
    {
        mSmoothedFadeInOut.setTargetValue(0.f);
    }
    else if (muteConfig == muting::unmute)
    {
        mSmoothedFadeInOut.setTargetValue(1.f);
        mIsMuted.store(false);
    }
    
    // const int64_t size = {mFifoSize};
    
    // for (int i = 0; i < size; ++i) {
        // mTempBuffer[i] = 0.f;
    // }
    
    // FloatVectorOperations::add(channelL, channelR, nSamples);
    // FloatVectorOperations::multiply(channelL, 0.5f, nSamples);
    // FloatVectorOperations::copy(channelR, channelL, nSamples);
    
    // assume model_block and host_block are powers of two
    // handle case when model_block > host_block and the reverse
    int io_blocks = ceil(float(host_block) / model_block);
    int min_block = std::min(model_block, host_block);

    int hostInIdx = 0;
    int hostOutIdx = 0;

    // PRINT("loop start");

    for (int block = 0; block < io_blocks; ++block){

        for (int i = 0; i < min_block; ++i) {
            // PRINT(inIdx); PRINT(hostInIdx);
            inBuffer[inIdx] = (channelL[hostInIdx] + channelR[hostInIdx])*0.5;
            hostInIdx++;
            inIdx++;
            if(inIdx == model_block){
                //process block
                // PRINT("model call");
                model->forward(&inBuffer[0], mLoopSelect, 1, &outBuffer[0]);
                outIdx = inIdx = 0;
                first_block_done = true;
            }
        }

        PRINT("block processed");

        for (int i = 0; i < min_block; ++i) {
            auto dry_gain = mSmoothedDryGain.getNextValue();
            auto wet_gain = mSmoothedWetGain.getNextValue();
            auto fade = mSmoothedFadeInOut.getNextValue();
            if (fade < EPSILON) mIsMuted.store(true);
            // channelL[hostOutIdx] = channelR[hostOutIdx] = 0;
            channelL[hostOutIdx] *= wet_gain;
            channelR[hostOutIdx] *= wet_gain;

            if (model->loaded && first_block_done){

                for (int j=0; j<n_loops; j++){
                    if (outIdx >= model_block*n_loops) {
                        std::cout<<"indexing error"<<std::endl;
                        outIdx = 0;
                    }
                    // TODO: mixdown
                    // TODO: multichannel out
                    channelL[hostOutIdx] += outBuffer[outIdx] * fade * dry_gain;
                    channelR[hostOutIdx] += outBuffer[outIdx] * fade * dry_gain;
                    outIdx++;
                }
            }
            hostOutIdx++;
        }
    }

    
    // for (int i = 0; i < nSamples; ++i) {
    //     FifoBuffer_write(hostfifo_p, channelL[i]);
    // }

    
    // // while (!FifoBuffer_is_empty(hostfifo_p)) {
            
    //     // float hostSample;
    //     // FifoBuffer_read(hostfifo_p, hostSample);
    
    //     // FifoBuffer_write(inputfifo_p, hostSample);
    //     // std::cout << FifoBuffer_count(hostfifo_p) << " " << size << std::endl;

    // while (FifoBuffer_count(hostfifo_p) >= size) {
        
    //     for (int i = 0; i < size; i++) {
    //         FifoBuffer_read(hostfifo_p, mTempBuffer[i]);
    //         // FifoBuffer_read(inputfifo_p, mTempBuffer[i]);
    //     }
                    
    //     if (model.get() && model->loaded && !mIsMuted.load()) {
    //         std::cout << "Living Looper: processing" << std::endl;
            
    //         auto output = model->forward_tensor(mTempBuffer, mLoopSelect);
            
    //         auto acc = output.accessor<float, 3>();
    //         auto fade = mSmoothedFadeInOut.getNextValue();

    //         // TODO: currently:
    //         // interleaved channels
    //         for(int i=0; i<acc.size(2); i++) {
    //             for(int j=0; j<acc.size(0); j++) {
    //                 FifoBuffer_write(outputfifo_p, (
    //                     acc[j][0][i] * fade
    //                 ));
    //             }
    //         }
            
    //         if (fade < EPSILON) {
    //             mIsMuted.store(true);
    //         }
}

//==============================================================================
bool LivingLooperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LivingLooperAudioProcessor::createEditor()
{
    return new LivingLooperAudioProcessorEditor(*this, mAVTS);
}

//==============================================================================
void LivingLooperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    auto state = mAVTS.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LivingLooperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (mAVTS.state.getType()))
            mAVTS.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LivingLooperAudioProcessor();
}

auto LivingLooperAudioProcessor::createParameterLayout() -> AudioProcessorValueTreeState::ParameterLayout
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<AudioParameterFloat> (rave_parameters::param_name_wetgain, rave_parameters::param_name_wetgain, 0.0f, 1.0f, 1.f));
    
    params.push_back(std::make_unique<AudioParameterFloat> (rave_parameters::param_name_drygain, rave_parameters::param_name_drygain, 0.0f, 1.0f, 0.5f));
    
    // NormalisableRange<float> normrange{-15.f, 15.f, 0.01f};
    // params.push_back (std::make_unique<AudioParameterFloat> (rave_parameters::param_name_temperature, rave_parameters::param_name_temperature, normrange, 0.f));
    
    // params.push_back (std::make_unique<AudioParameterBool> (rave_parameters::param_name_toggleprior, rave_parameters::param_name_toggleprior, false));
    
    params.push_back (std::make_unique<AudioParameterBool> (rave_parameters::param_name_importbutton, rave_parameters::param_name_importbutton, false));
    
    return { params.begin(), params.end() };
    
}

auto LivingLooperAudioProcessor::mute() -> void
{
    mFadeScheduler.store(muting::mute);
}

auto LivingLooperAudioProcessor::unmute() -> void
{
    mFadeScheduler.store(muting::unmute);
}

auto LivingLooperAudioProcessor::getIsMuted() -> const bool
{
    return mIsMuted.load();
}

void LivingLooperAudioProcessor::updateEngine(const std::string modelFile)
{
    juce::ScopedLock irCalculationlock(mEngineUpdateMutex);
    if (mEngineThreadPool)
    {
        mEngineThreadPool->removeAllJobs(true, 100);
    }

    mEngineThreadPool->addJob(new UpdateEngineJob(*this, modelFile), true);
}

void LivingLooperAudioProcessor::parameterChanged (const String& parameterID, float newValue)
{
    if(parameterID == rave_parameters::param_name_importbutton)
    {
        fc.reset(new FileChooser(
            "Choose a file to open...", 
            File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory),
            "*.ts;*.ckpt", true));

        fc->launchAsync(
            FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
            [this] (const FileChooser& chooser){
                File chosen;
                auto results = chooser.getURLResults();

                for (auto result : results) {
                    if (result.isLocalFile()) {
                        chosen = result.getLocalFile();
                    }
                    else{
                        std::cout << "not a local file" << std::endl;
                        return;
                    }                    
                }                        
                // AlertWindow::showAsync(MessageBoxOptions()
                //     .withIconType (MessageBoxIconType::InfoIcon)
                //     .withTitle ("File Chooser...")
                //     .withMessage ("You picked: " + chosen.getFileName())
                //     .withButton ("OK"),
                //     nullptr);
            if (chosen.getFileExtension() == ".ts" && chosen.getSize() != 0) {
                this->updateEngine( chosen.getFullPathName().toStdString() );
            }
            
        });
    }

}
