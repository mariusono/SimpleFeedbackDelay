/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleFeedbackDelayAudioProcessorEditor::SimpleFeedbackDelayAudioProcessorEditor (SimpleFeedbackDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    addAndMakeVisible(delay_Slider);
    delay_Slider.setTextValueSuffix(" [ms]");
    delay_Slider.addListener(this);
    delay_Slider.setRange(0.0,2000.0);
    delay_Slider.setValue(100.0);
    addAndMakeVisible(delay_Label);
    delay_Label.setText("Delay", juce::dontSendNotification);
    delay_Label.attachToComponent(&delay_Slider, true);
    
    delay_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"DELAY",delay_Slider);
    
    audioProcessor.set_delay_param(delay_Slider.getValue());

    
    addAndMakeVisible(gFB_Slider);
    gFB_Slider.setTextValueSuffix(" [-]");
    gFB_Slider.addListener(this);
    gFB_Slider.setRange(0.0,1.0);
    gFB_Slider.setValue(0.0);
    addAndMakeVisible(gFB_Label);
    gFB_Label.setText("Feedback", juce::dontSendNotification);
    gFB_Label.attachToComponent(&gFB_Slider, true);
    
    gFB_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"FEEDBACK",gFB_Slider);
    
    audioProcessor.set_gFB_param(gFB_Slider.getValue());

    
    addAndMakeVisible(drywet_Slider);
    drywet_Slider.setTextValueSuffix(" [-]");
    drywet_Slider.addListener(this);
    drywet_Slider.setRange(0.0,1.0);
    drywet_Slider.setValue(0.0);
    addAndMakeVisible(drywet_Label);
    drywet_Label.setText("Dry Wet", juce::dontSendNotification);
    drywet_Label.attachToComponent(&drywet_Slider, true);
    
    drywet_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"DRY_WET",drywet_Slider);
    
    audioProcessor.set_drywet_param(drywet_Slider.getValue());
    
    addAndMakeVisible(vol_Slider);
    vol_Slider.setTextValueSuffix(" [-]");
    vol_Slider.addListener(this);
    vol_Slider.setRange(-20.0,20.0);
    vol_Slider.setValue(0.0);
    addAndMakeVisible(vol_Label);
    vol_Label.setText("Volume", juce::dontSendNotification);
    vol_Label.attachToComponent(&vol_Slider, true);
    
    vol_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"VOLUME",vol_Slider);
    
    audioProcessor.set_gVolume_param(vol_Slider.getValue());

}

SimpleFeedbackDelayAudioProcessorEditor::~SimpleFeedbackDelayAudioProcessorEditor()
{
}

//==============================================================================
void SimpleFeedbackDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleFeedbackDelayAudioProcessorEditor::resized()
{
    auto sliderLeft = 120;

    delay_Slider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
    gFB_Slider.setBounds(sliderLeft, 20+40, getWidth() - sliderLeft - 10, 20);
    drywet_Slider.setBounds(sliderLeft, 20+40+40, getWidth() - sliderLeft - 10, 20);
    vol_Slider.setBounds(sliderLeft, 20+40+40+40, getWidth() - sliderLeft - 10, 20);
    
}


void SimpleFeedbackDelayAudioProcessorEditor::sliderValueChanged(Slider* slider)
{

    if (slider == &delay_Slider)
    {
//        if (audioProcessor.flagSwitchInProgress == 0)
//        {
//            audioProcessor.set_delay_param(delay_Slider.getValue());
//            audioProcessor.flagSwitch = 1;
//        }
        auto sliderVal = delay_Slider.getValue();
        
        audioProcessor.set_delay_param(delay_Slider.getValue());
    }
    else if (slider == &gFB_Slider)
    {
        audioProcessor.set_gFB_param(gFB_Slider.getValue());
    }
    else if (slider == &drywet_Slider)
    {
        audioProcessor.set_drywet_param(drywet_Slider.getValue());
    }
    else if (slider == &vol_Slider)
    {
        audioProcessor.set_gVolume_param(vol_Slider.getValue());
    }
}
