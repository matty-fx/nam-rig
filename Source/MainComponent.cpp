#include "NeuralAmpModelerCore/NAM/activations.h"
#include "MainComponent.h"
#include "architecture.h"

//==============================================================================
MainComponent::MainComponent() : audioSetupComp (deviceManager,
                                    0,     // minimum input channels
                                    256,   // maximum input channels
                                    0,     // minimum output channels
                                    256,   // maximum output channels
                                    false, // ability to select midi inputs
                                    false, // ability to select midi output device
                                    false, // treat channels as stereo pairs
                                    false) // hide advanced options
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
    
    addAndMakeVisible(audioSetupComp);
    
    cpuUsageLabel.setText ("CPU Usage", juce::dontSendNotification);
    cpuUsageText.setJustificationType (juce::Justification::right);
    addAndMakeVisible (&cpuUsageLabel);
    addAndMakeVisible (&cpuUsageText);
    
    startTimer(250);
    
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    juce:: String path = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName() +"/Development/NAM Models/test_model/test_model.nam";
    auto dspPath = std::filesystem::u8path(path.toStdString());
    model = get_dsp(dspPath);

    if(!model)
        DBG("No model!");
    else
        DBG(model->GetExpectedSampleRate());
    
    inBuffer = new double[samplesPerBlockExpected];
    outBuffer = new double[samplesPerBlockExpected];
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    std::fenv_t fe_state;
    std::feholdexcept(&fe_state);
    disable_denormals();
    
    juce::AudioBuffer buffer (bufferToFill.buffer->getArrayOfWritePointers(),
                        bufferToFill.buffer->getNumChannels(),
                        bufferToFill.startSample,
                        bufferToFill.numSamples);
    
    // Can I configure JUCE to pass buffer of type double?
    for (int i = 0 ; i < buffer.getNumSamples(); i++)
        inBuffer[i] = (double) buffer.getSample(0, i);
    
    model->process(inBuffer, outBuffer, buffer.getNumSamples());
    model->finalize_(buffer.getNumSamples());

    for (int i = 0 ; i < buffer.getNumSamples(); i++)
    {
        buffer.setSample(0, i, (float) outBuffer[i]);
        buffer.setSample(1, i, (float) outBuffer[i]);
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    audioSetupComp.setBounds(0, 0, 800, 800);
    cpuUsageLabel.setBounds (0, 0, 75, 100);
    cpuUsageText.setBounds (80, 0, 100, 100);
}

void MainComponent::setCpuText(float cpu)
{
    cpuUsageText.setText (juce::String (cpu, 6) + " %", juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    setCpuText(deviceManager.getCpuUsage() * 100);
}
