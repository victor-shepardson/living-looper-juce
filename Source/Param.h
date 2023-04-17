#pragma once

#include <JuceHeader.h>

using namespace juce;
using namespace std;

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

// TODO: `Params` class which contains the map and vts?

template<class T>
class Param {
  public: 
    String mName;
    // atomic<T> *ptr;
    unique_ptr<Slider> mSlider;
    unique_ptr<SliderAttachment> mAttach;
    // optional<LinearSmoothedValue<T>> mSmooth;
    LinearSmoothedValue<T> mSmooth;
    // optional<Label> mLabel;   
    unique_ptr<Label> mLabel;
    std::optional<unique_ptr<ToggleButton>> mToggle;
    int mButtonGroup;

    void setupEditor(
        AudioProcessorValueTreeState& vts,
        AudioProcessorEditor &editor)
    {
        //toggle
        if(mToggle){
            auto& tgl = mToggle.value();
            tgl.reset(new ToggleButton());
            editor.addAndMakeVisible(tgl.get());
            tgl->setButtonText(mName);
            tgl->setRadioGroupId(mButtonGroup);
        }

        //label
        mLabel.reset(new Label());
        // mLabel = make_unique<Label>();
        editor.addAndMakeVisible(mLabel.get());
        mLabel->setText(
            mName, NotificationType::dontSendNotification);
        mLabel->setJustificationType(
            Justification::centred);

        //slider
        mSlider.reset(new Slider());
        // mSlider = make_unique<Slider>();
        editor.addAndMakeVisible(mSlider.get());
        mSlider->setSliderStyle(
            Slider::SliderStyle::RotaryVerticalDrag);
        mSlider->setTextBoxStyle(
            Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);

        const Colour color(0xff00fef2);
        mSlider->setColour(
            Slider::ColourIds::rotarySliderFillColourId, color);
        mSlider->setColour(
            Slider::ColourIds::thumbColourId, Colours::white);

        // create an Attachment which links the
        // Parameter to the Slider via the ValueTreeState
        mAttach.reset(new SliderAttachment(
            vts, mName, *mSlider));
    }

    // sync the parameter value back to the UI
    void updateEditor(
        AudioProcessorValueTreeState& vts)
    {
        auto val = vts.getRawParameterValue(mName)->load();
        mSlider->setValue(val);
    }

    void resizeEditor(
        // AudioProcessorEditor &editor, 
        Rectangle<float> bounds)
    {
        // resize
        // bounds.removeFromBottom(0.5);
        if(mToggle){
            mToggle.value()->setBoundsRelative(bounds.removeFromTop(0.2f));
        }
        mSlider->setBoundsRelative(bounds.removeFromTop(0.3).removeFromBottom(0.3));
        mLabel->setBoundsRelative(
            bounds.removeFromTop(1.0f));
    }

    //UI stuff is short-lived and needs to be freed
    void destroyEditor(){
        // attachment needs to go first
        mAttach.release();
        mSlider.release();
        mLabel.release();
        if(mToggle){
            mToggle.value().release();
        }
    }

    // TODO: parameters, template type
    unique_ptr<AudioParameterFloat> getParameter(){
        return make_unique<AudioParameterFloat>(
            mName, mName, 0.0f, 1.0f, 0.5f);
    }

    void prepare(double sampleRate){
        // if (mSmooth.has_value()){
        //     mSmooth->reset(sampleRate, 0.1);
        // }
        mSmooth.reset(sampleRate, 0.1);
    }

    void processBlock(
            AudioProcessorValueTreeState& vts
            // ,
            // AudioProcessor &processor
            ){
        atomic<T> *
        ptr = vts.getRawParameterValue(mName);
        // if (mSmooth.has_value()){
            // mSmooth->setTargetValue(ptr->load());
        // }
        mSmooth.setTargetValue(ptr->load());
    }

    Param(
            // AudioProcessor &processor, 
            String name, int buttonGroup=0){
        mName = name;
        if (buttonGroup) {
            mToggle.emplace();
            mButtonGroup = buttonGroup;
        }
        // if (smooth) mSmooth.emplace();
        // if (label) mLabel.emplace();
    }
    // ~Param(){
    //     mAttach.release();
    // }
};