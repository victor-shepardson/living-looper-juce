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

    void setupEditor(
            AudioProcessorValueTreeState& vts,
            AudioProcessorEditor &editor
            ){

        //label
        mLabel = make_unique<Label>();
        editor.addAndMakeVisible(mLabel.get());
        mLabel->setText(
            mName, NotificationType::dontSendNotification);
        mLabel->setJustificationType(
            Justification::centred);

        //slider
        mSlider = make_unique<Slider>();

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

    void resizeEditor(
            // AudioProcessorEditor &editor, 
            Rectangle<float> bounds){
        // resize
        mSlider->setBoundsRelative(bounds);
        mLabel->setBoundsRelative(
            bounds.removeFromTop(1.0f));
        // label
        //...
    }

    void destroyEditor(){
        mAttach.release();
        mSlider.release();
        mLabel.release();
    }

    // TODO: template type
    unique_ptr<AudioParameterFloat> getParameter(){
        return make_unique<AudioParameterFloat>(
            mName, mName, 0.0f, 1.0f, 0.5f);
    }

    void prepare(double sampleRate){
        // if (mSmooth.has_value()){
        //     mSmooth->reset(sampleRate, 0.1);
        // }
        mSmooth.reset(sampleRate, 0.1);
        // DBG("prepare");
        // DBG(sampleRate);
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
            String name){
        mName = name;
        // if (smooth) mSmooth.emplace();
        // if (label) mLabel.emplace();
    }
    // ~Param(){
    //     mAttach.release();
    //     mSlider.release();
    //     mLabel.release();
    // }
};