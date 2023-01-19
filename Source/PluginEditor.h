/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SimpleFeedbackDelayAudioProcessorEditor  : public juce::AudioProcessorEditor,
public juce::Slider::Listener
{
public:
    SimpleFeedbackDelayAudioProcessorEditor (SimpleFeedbackDelayAudioProcessor&);
    ~SimpleFeedbackDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void sliderValueChanged (Slider* slider) override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleFeedbackDelayAudioProcessor& audioProcessor;
    
    
    Slider delay_Slider;
    Label delay_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delay_SliderAttachment;

    Slider gFB_Slider;
    Label gFB_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gFB_SliderAttachment;

    Slider drywet_Slider;
    Label drywet_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> drywet_SliderAttachment;
    
    Slider vol_Slider;
    Label vol_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vol_SliderAttachment;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFeedbackDelayAudioProcessorEditor)
};
