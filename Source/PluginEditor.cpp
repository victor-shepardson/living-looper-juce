/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LivingLooperAudioProcessorEditor::LivingLooperAudioProcessorEditor (
  LivingLooperAudioProcessor& p, AudioProcessorValueTreeState& vts
  ) : AudioProcessorEditor(&p), audioProcessor(p), mAVTS(vts){
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    mLogo.reset(new ImageComponent());
    mLogo->setImage(
      ImageCache::getFromMemory(
        BinaryData::wiggly_png, 
        BinaryData::wiggly_pngSize)
        .getClippedImage(Rectangle<int>(0, 400, 3200, 2000))
        );
    addAndMakeVisible(mLogo.get());
    
    for (auto pair: audioProcessor.mParams){
      pair.second->setupEditor(mAVTS, *this);
    }
    
    addAndMakeVisible(&mImportButton);
    mImportButton.setButtonText("IMPORT");
    mImportButton.setClickingTogglesState(true);
    mImportButtonAttachment.reset(new ButtonAttachment(
      mAVTS, rave_parameters::param_name_importbutton, mImportButton));
    
    addAndMakeVisible(&mActiveLoopLabel);
    mActiveLoopLabel.setText(
      "0", NotificationType::dontSendNotification);
    mActiveLoopLabel.setJustificationType(
      Justification::centred);
    
    setResizable(true, true);
    getConstrainer()->setMinimumSize(400, 250);
    
    setSize(800, 500);
}

LivingLooperAudioProcessorEditor::~LivingLooperAudioProcessorEditor()
{
  for(auto p: audioProcessor.mParams){
    p.second->destroyEditor();
  }
}

//==============================================================================
void LivingLooperAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(APPLE_BLACK);

    // Loop Select
    auto qls = audioProcessor.qLoopSelect;
    int loop_select = -1;
    while (!qls.empty()){
      loop_select = qls.front();
      qls.pop();
    }
    if (loop_select >= 0){
      mActiveLoopLabel.setText(
        std::to_string(loop_select), NotificationType::dontSendNotification);
    }

    // N Loops
    auto qnl = audioProcessor.qNLoops;
    int n_loops = 0;
    while (!qnl.empty()){
      n_loops = qnl.front();
      qnl.pop();
    }
    /// create per-loop elements

}

void LivingLooperAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    mLogo->setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    
    Rectangle<float> mRelBounds { 0.f, 0.f, 1.f, 1.f };
    mRelBounds.removeFromBottom(0.075f);
    auto bottomBounds = mRelBounds.removeFromBottom(0.3f);
    // auto wetLabelBounds = mRelBounds.removeFromBottom(0.1f);
    // auto temperatureLabelBounds = wetLabelBounds;
    // auto dryLabelBounds = temperatureLabelBounds;
    // auto dryLabelBounds = wetLabelBounds;
    // auto activeLoopLabelBounds = wetLabelBounds;
    auto activeLoopLabelBounds = mRelBounds.removeFromBottom(0.1f);
    
    // wetLabelBounds.removeFromLeft(0.6666666f);
    // mWetGainLabel.setBoundsRelative(wetLabelBounds);
    
    // temperatureLabelBounds.removeFromRight(0.3333333f);
    // temperatureLabelBounds.removeFromLeft(0.3333333f);
    // mTemperatureLabel.setBoundsRelative(temperatureLabelBounds);
    activeLoopLabelBounds.removeFromRight(0.3333333f);
    activeLoopLabelBounds.removeFromLeft(0.3333333f);
    mActiveLoopLabel.setBoundsRelative(activeLoopLabelBounds);
    
    // dryLabelBounds.removeFromRight(0.6666666f);
    // mDryGainLabel.setBoundsRelative(dryLabelBounds);
    
    // mRelBounds.removeFromTop(0.05f);
    auto topBounds = mRelBounds.removeFromTop(0.05f);
    auto toggleBounds = topBounds;
    topBounds.removeFromLeft(0.85f);
    topBounds.removeFromRight(0.05f);
    mImportButton.setBoundsRelative(topBounds);
    
    // toggleBounds.removeFromLeft(0.05f);
    // toggleBounds.removeFromRight(0.8f);
    // mTogglePrior.setBoundsRelative(toggleBounds);
    
    // mWetGainSlider.setBoundsRelative(bottomBounds.removeFromRight(0.3333333f));
    // mDryGainSlider.setBoundsRelative(bottomBounds.removeFromRight(0.3333333f));

    audioProcessor.mParams["wet"]->resizeEditor(
      bottomBounds.removeFromRight(0.3333333f));
    audioProcessor.mParams["dry"]->resizeEditor(
      bottomBounds.removeFromRight(0.3333333f));

    
}
