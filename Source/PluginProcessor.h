/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SimpleFeedbackDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleFeedbackDelayAudioProcessor();
    ~SimpleFeedbackDelayAudioProcessor() override;

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

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // SET PARAMS
    void set_delay_param(float val) { delay_slider = val; }
    void set_gFB_param(float val) { gFB_slider = val; }
    void set_drywet_param(float val) { gDryWet_slider = val; }
    void set_gVolume_param(float val) { gVolume_slider = val; }
    
    //==============================================================================
    // FOR PARAMETERS !
    juce::AudioProcessorValueTreeState apvts;
    
    
    int flagSwitch = 0;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFeedbackDelayAudioProcessor)
    
    
    int BUFFER_SIZE = 262144;
    double gSampleRate, T;
    
    std::vector<double> gDelayBuffer_inSig; // delay buffer from input
    int gWritePointer_inSig; // write pointer for delay buffer from input
    int gReadPointer_inSig; // read pointer for delay buffer from input

    std::vector<double> gDelayBuffer_outSig; // delay buffer from input
    int gWritePointer_outSig; // write pointer for delay buffer from input
    int gReadPointer_outSig; // read pointer for delay buffer from input

    int gInitLatency;
    
    double inSig_delay;
    double outSig_delay;
    double outVal, outValDry;
    double outValPrev;

    double outSig_delay_crossfade;
    double outSig_delay_comb;
    
    double inSig_delay_crossfade;
    double inSig_delay_comb;

    double delay;
    double gFB;
    double gFF;
    
    double delay_crossfade;
    double delay_crossfade_samples;
    double delay_crossfade_frac_part;
    
    double delay_samples;
    double delay_frac_part;
    
    
    double gFactDry, gFactWet;
    double gDryWet;
    double drywet;
    
    double gVolume_slider;
    double delay_slider, delay_prev, delay_slider_smoo;
    double gFB_slider, gFB_prev;
    double gDryWet_slider, gDryWet_prev;

    
    double countSwitch = 0;
    double countSwitchMax = 256;
    
    // AUDIO PARAMS
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        params.push_back(std::make_unique<AudioParameterFloat>("DELAY","Delay",0.0f,2000.0f,100.0f));
        params.push_back(std::make_unique<AudioParameterFloat>("FEEDBACK","Feedback",0.0f,1.0f,0.0f));
        params.push_back(std::make_unique<AudioParameterFloat>("DRY_WET","Dry_Wet",0.0f,1.0f,0.0f)); // in dB
        params.push_back(std::make_unique<AudioParameterFloat>("VOLUME","Volume",-20.0f,20.0f,0.0f)); // in dB

        return { params.begin(), params.end()};
    }
};
