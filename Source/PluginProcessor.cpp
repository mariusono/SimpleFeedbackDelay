/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//------------------------------------------------------------
double linearMapping(float rangeIn_top, float rangeIn_bottom, float rangeOut_top, float rangeOut_bottom, float value) {
    double newValue = rangeOut_bottom + ((rangeOut_top - rangeOut_bottom) * (value - rangeIn_bottom) / (rangeIn_top - rangeIn_bottom));
    return newValue;
}
//------------------------------------------------------------

//==============================================================================
SimpleFeedbackDelayAudioProcessor::SimpleFeedbackDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
}

SimpleFeedbackDelayAudioProcessor::~SimpleFeedbackDelayAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleFeedbackDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleFeedbackDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleFeedbackDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleFeedbackDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleFeedbackDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleFeedbackDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleFeedbackDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleFeedbackDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleFeedbackDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleFeedbackDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleFeedbackDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    gSampleRate = sampleRate;
    T = 1/gSampleRate;
    Logger::getCurrentLogger()->outputDebugString("Sample rate is " + String(sampleRate) + ".");
    
        
    gInitLatency = 8;
    
    // Resizing buffers and preallocating read and write pointers
    gDelayBuffer_inSig.resize(BUFFER_SIZE,0);
    gWritePointer_inSig = gInitLatency;
    gReadPointer_inSig = 0;
    
    gDelayBuffer_outSig.resize(BUFFER_SIZE,0);
    gWritePointer_outSig = gInitLatency;
    gReadPointer_outSig = 0;
    
    outVal = 0;
    outValDry = 0;
    outValPrev = 0;
    
    gDryWet = 0;
    gDryWet_slider = 0;
    gFB = 0;
    gFB_slider = 0;
    gVolume_slider = 0;

    delay = 100;
    delay_slider = 100;
    delay_prev = 100;
    delay_slider_smoo = 100;
    
    delay_crossfade = delay;
}

void SimpleFeedbackDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleFeedbackDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleFeedbackDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float* const outputL = buffer.getWritePointer(0);
    float* const outputR = buffer.getWritePointer(1);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
//    // READ PARAMS
//    delay = (1-0.99)*delay_param + 0.99*delay_param_prev;
//    delay_param_prev = delay;
    
//    Logger::getCurrentLogger()->outputDebugString("delay is " + String(delay) + ".");
//    Logger::getCurrentLogger()->outputDebugString("delay_samples is " + String(delay_samples) + ".");
//    Logger::getCurrentLogger()->outputDebugString("delay_crossfade_samples is " + String(delay_crossfade_samples) + ".");
//    Logger::getCurrentLogger()->outputDebugString("delay_frac_part is " + String(delay_frac_part) + ".");
//    Logger::getCurrentLogger()->outputDebugString("gFB is " + String(gFB) + ".");
//    Logger::getCurrentLogger()->outputDebugString("gDryWet is " + String(gDryWet) + ".");
//
//    Logger::getCurrentLogger()->outputDebugString("outVal is " + String(outVal) + ".");

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        for (int channel = 0; channel < 2; ++channel)
        {
            if (channel == 0)
            {
                auto* input = buffer.getWritePointer (channel);
                
                // Current input sample
                float in = input[i];
                

                // READ PARAMS
//              // if using a one-pole filter:
//                delay = delay_param;
//                delay = (1-0.95)*delay_param + 0.95*delay_param_prev;
                
                // add a smoother on the slider
                delay_slider_smoo = (1-0.95)*delay_slider + 0.95*delay_prev;
                delay_prev = delay_slider_smoo;
                
                if (flagSwitch == 0)
                {
                    if (delay_slider != delay)
                    {
                        flagSwitch = 1;
                        delay = delay_slider_smoo;
                    }
                }
                
                if (delay == 0) {delay = 1;}
                
                // Smoothing of inputs: one-pole filter
                gFB = (1-0.8)*gFB_slider + 0.8*gFB_prev;
                gFB_prev = gFB;
                gDryWet = (1-0.8)*gDryWet_slider + 0.8*gDryWet_prev;
                gDryWet_prev = gDryWet_slider;
 
                // delays in samples
                delay_samples = floor(delay*gSampleRate/1000);
                delay_frac_part = delay*gSampleRate/1000 - delay_samples;
                
                delay_crossfade_samples = floor(delay_crossfade*gSampleRate/1000);
                delay_crossfade_frac_part = delay_crossfade*gSampleRate/1000 - delay_crossfade_samples;

                outValDry = in; // Dry output

                gDelayBuffer_inSig[gWritePointer_inSig] = in;
                    
                int index = (gReadPointer_inSig - static_cast<int>(delay_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                int index_m1 = (gReadPointer_inSig - static_cast<int>(delay_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_p1 = (gReadPointer_inSig  - static_cast<int>(delay_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_p2 = (gReadPointer_inSig - static_cast<int>(delay_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                double alpha = delay_frac_part;
                
                int index_crossfade = (gReadPointer_inSig - static_cast<int>(delay_crossfade_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                int index_crossfade_m1 = (gReadPointer_inSig - static_cast<int>(delay_crossfade_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_crossfade_p1 = (gReadPointer_inSig  - static_cast<int>(delay_crossfade_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_crossfade_p2 = (gReadPointer_inSig - static_cast<int>(delay_crossfade_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                double alpha_crossfade = delay_crossfade_frac_part;
                
//                inSig_delay = delay_frac_part*gDelayBuffer_inSig[index_m1] + (1-delay_frac_part)*gDelayBuffer_inSig[index];
                
                inSig_delay = (
                               alpha*(alpha-1)*(alpha-2)*gDelayBuffer_inSig[index_m1]/(-6)
                               +(alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_inSig[index]/2
                               +alpha*(alpha+1)*(alpha-2)*gDelayBuffer_inSig[index_p1]/(-2)
                               +alpha*(alpha+1)*(alpha-1)*gDelayBuffer_inSig[index_p2]/(6)
                               );
                
                inSig_delay_crossfade = (
                                         alpha_crossfade*(alpha_crossfade-1)*(alpha_crossfade-2)*gDelayBuffer_inSig[index_crossfade_m1]/(-6)
                               +(alpha_crossfade-1)*(alpha_crossfade+1)*(alpha_crossfade-2)*gDelayBuffer_inSig[index_crossfade]/2
                               +alpha_crossfade*(alpha_crossfade+1)*(alpha_crossfade-2)*gDelayBuffer_inSig[index_crossfade_p1]/(-2)
                               +alpha_crossfade*(alpha_crossfade+1)*(alpha_crossfade-1)*gDelayBuffer_inSig[index_crossfade_p2]/(6)
                               );

                
                gDelayBuffer_outSig[gWritePointer_outSig] = outVal;
                    
                index = (gReadPointer_outSig - static_cast<int>(delay_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                index_m1 = (gReadPointer_outSig - static_cast<int>(delay_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p1 = (gReadPointer_outSig  - static_cast<int>(delay_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p2 = (gReadPointer_outSig - static_cast<int>(delay_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                alpha = delay_frac_part;

                index_crossfade = (gReadPointer_outSig - static_cast<int>(delay_crossfade_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                index_crossfade_m1 = (gReadPointer_outSig - static_cast<int>(delay_crossfade_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_crossfade_p1 = (gReadPointer_outSig  - static_cast<int>(delay_crossfade_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_crossfade_p2 = (gReadPointer_outSig - static_cast<int>(delay_crossfade_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                alpha_crossfade = delay_crossfade_frac_part;
                
                
//                outSig_delay = delay_frac_part*gDelayBuffer_outSig[index_m1] + (1-delay_frac_part)*gDelayBuffer_outSig[index];
                
                outSig_delay = (
                                alpha*(alpha-1)*(alpha-2)*gDelayBuffer_outSig[index_m1]/(-6)
                                + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_outSig[index]/2
                                + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_outSig[index_p1]/(-2)
                                + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_outSig[index_p2]/(6)
                                );
                
                outSig_delay_crossfade = (
                                          alpha_crossfade*(alpha_crossfade-1)*(alpha_crossfade-2)*gDelayBuffer_outSig[index_crossfade_m1]/(-6)
                                + (alpha_crossfade-1)*(alpha_crossfade+1)*(alpha_crossfade-2)*gDelayBuffer_outSig[index_crossfade]/2
                                + alpha_crossfade*(alpha_crossfade+1)*(alpha_crossfade-2)*gDelayBuffer_outSig[index_crossfade_p1]/(-2)
                                + alpha_crossfade*(alpha_crossfade+1)*(alpha_crossfade-1)*gDelayBuffer_outSig[index_crossfade_p2]/(6)
                                );
                
                if (flagSwitch==1)
                {
                    
                    Logger::getCurrentLogger()->outputDebugString("flagSwitch is " + String(flagSwitch) + ".");
                    
                    double fac1 = sqrt((1 - countSwitch/countSwitchMax));
                    double fac2 = sqrt((countSwitch/countSwitchMax));
                    
                    inSig_delay_comb = inSig_delay_crossfade * fac1 + inSig_delay*fac2;
                    outSig_delay_comb = outSig_delay_crossfade * fac1 + outSig_delay*fac2;
                    
//                    inSig_delay_comb = inSig_delay_crossfade;
//                    outSig_delay_comb = outSig_delay_crossfade;
                    
                    countSwitch = countSwitch + 1;
                    if (countSwitch == countSwitchMax)
                    {
                        countSwitch = 0;
                        flagSwitch = 0;
                        delay_crossfade = delay;
                        
                        Logger::getCurrentLogger()->outputDebugString("flagSwitch is " + String(flagSwitch) + ".");
                        Logger::getCurrentLogger()->outputDebugString("delay_crossfade is " + String(delay_crossfade) + ".");
                        Logger::getCurrentLogger()->outputDebugString("delay is " + String(delay) + ".");
                        Logger::getCurrentLogger()->outputDebugString("delay_slider_smoo is " + String(delay_slider_smoo) + ".");
                        Logger::getCurrentLogger()->outputDebugString("delay_slider is " + String(delay_slider) + ".");
                        
                    }
                }
                else if (flagSwitch==0)
                {
                    inSig_delay_comb = inSig_delay;
                    outSig_delay_comb = outSig_delay;
                    
//                    inSig_delay_comb = inSig_delay_crossfade;
//                    outSig_delay_comb = outSig_delay_crossfade;
                }
                
                
                
                

                
                outVal = in + (1-gFB)*inSig_delay_comb + gFB*outSig_delay_comb;
                
                outValPrev = outVal;
                
                // update gWritePointer
                gWritePointer_inSig = gWritePointer_inSig + 1;
                if (gWritePointer_inSig >= BUFFER_SIZE)
                    gWritePointer_inSig = 0;

                // update gReadPointer
                gReadPointer_inSig = gReadPointer_inSig + 1;
                if (gReadPointer_inSig >= BUFFER_SIZE)
                    gReadPointer_inSig = 0;
                
                // update gWritePointer
                gWritePointer_outSig = gWritePointer_inSig + 1;
                if (gWritePointer_outSig >= BUFFER_SIZE)
                    gWritePointer_outSig = 0;

                // update gReadPointer
                gReadPointer_outSig = gReadPointer_outSig + 1;
                if (gReadPointer_outSig >= BUFFER_SIZE)
                    gReadPointer_outSig = 0;
                
                // Output
                drywet = linearMapping(1.0f, 0.0f, 1.0f, -1.0f, gDryWet);
                
                if(drywet<-1.0)
                {
                    drywet = -1.0;
                }else if (drywet>0.99)
                {
                    drywet = 1.0;
                }
                
                gFactDry = (powf(0.5*(1.0-drywet),0.5));
                gFactWet = (powf(0.5*(1.0+drywet),0.5));
                
                outputL[i] = (outVal * gFactWet + outValDry * gFactDry) * pow(10,(gVolume_slider/20));
                

                if (abs(outputL[i]) > 1)
                {
                    Logger::getCurrentLogger()->outputDebugString("Output left is too loud!");
                }
                outputR[i] = outputL[i];
            }
        }
    }
    
}

//==============================================================================
bool SimpleFeedbackDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleFeedbackDelayAudioProcessor::createEditor()
{
    return new SimpleFeedbackDelayAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleFeedbackDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleFeedbackDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleFeedbackDelayAudioProcessor();
}
