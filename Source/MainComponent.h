#pragma once

#include <JuceHeader.h>
#include "NeuralAmpModelerCore/NAM/dsp.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void setCpuText(float cpu);

private:
    void timerCallback() override;
    
    //==============================================================================
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::Label cpuUsageText;
    juce::Label cpuUsageLabel;
    
    std::unique_ptr<DSP> model;
    
    double * inBuffer;
    double * outBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
