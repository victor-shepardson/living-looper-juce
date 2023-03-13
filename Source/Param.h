#include <JuceHeader.h>

using namespace std;

template<class T>
class Param {
  public: 
    String mName;
    atomic<T> *ptr;
    Slider mSlider;
    unique_ptr<SliderParameterAttachment> mAttach;
    optional<LinearSmoother<T>> mSmooth;
    optional<Label> mLabel;

    void setupEditor(AudioProcessorEditor &editor){

        //slider
        editor.addAndMakeVisible(&mSlider);
        mSlider.setSliderStyle(
            Slider::SliderStyle::RotaryVerticalDrag);
        mSlider.setTextBoxStyle(
            Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);

        const Colour color(0xff00fef2);
        mSlider.setColour(
            Slider::ColourIds::rotarySliderFillColourId, color);
        mSlider.setColour(
            Slider::ColourIds::thumbColourId, Colours::white);
        //label
        // editor.addAndMakeVisible(&mLabel);
        // mLabel.setText(
            // mName, NotificationType::dontSendNotification);
        // mLabel.setJustificationType(
            // Justification::centred);

        // create an Attachment which links the
        // Parameter to the Slider via the ValueTreeState
        mAttach.reset(new SliderAttachment(
            editor.mAVTS, mName, mSlider));
    }

    void resizeEditor(
            AudioProcessorEditor &editor, 
            Rectangle<float> bounds){
        // resize
        mSlider.setBoundsRelative(bounds);
        // label
        //...
    }

    void processBlock(){
        mSmooth.setTargetValue(ptr->load());
    }

    // construct from the AudioProcessor
    Param(AudioProcessor &processor, String name, bool smooth, bool label){
        nName = name;
    }
    ~Param(){
        delete ptr;
    }
};