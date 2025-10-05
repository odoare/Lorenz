/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LorenzOsc.h"

#define PITCHBUFFERSIZE 2048

//==============================================================================
/**
*/
class LorenzAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    LorenzAudioProcessor();
    ~LorenzAudioProcessor() override;

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

    void requestOscillatorReset();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::AudioProcessorValueTreeState apvts{*this,nullptr,"Parameters",createParameters()};

    //==============================================================================
    // Struct to hold 3D coordinates
    struct Point { float x, y, z; };

    // Thread-safe FIFO for passing points to the GUI
    bool getPointFromFifo(Point& p)
    {
        int start1, size1, start2, size2;
        pointFifo.prepareToRead (1, start1, size1, start2, size2);

        if (size1 > 0)
        {
            p = pointBuffer[start1];
            pointFifo.finishedRead (1);
            return true;
        }

        return false;
    }

    float getPitch();

private:
    void pushPointToFifo(const Point& p)
    {
        int start1, size1, start2, size2;
        pointFifo.prepareToWrite(1, start1, size1, start2, size2);

        if (size1 > 0)
        {
            pointBuffer[start1] = p;
            pointFifo.finishedWrite(1);
        }
        // If size1 is 0, the FIFO is full, so we drop the point.
        // This is better than corrupting the FIFO state.
    }

    static constexpr int fifoSize = 2048;
    juce::AbstractFifo pointFifo { fifoSize };
    std::vector<Point> pointBuffer { fifoSize };

    LorenzOsc lorenzOsc;
    std::atomic<bool> resetRequested { false };

    // For controlling the rate of points sent to the GUI
    int samplesUntilNextPoint = 0;
    int pointGenerationInterval = 0;
    static constexpr int pointsPerSecond = 8000;

    // Duffing parameter pointers
    std::atomic<float>* alphaParam = nullptr;
    std::atomic<float>* betaDuffingParam = nullptr;
    std::atomic<float>* gammaParam = nullptr;
    std::atomic<float>* deltaParam = nullptr;
    std::atomic<float>* frequencyParam = nullptr;
    juce::RangedAudioParameter* timestepRangedParam = nullptr;
    std::atomic<float>* timestepParam = nullptr;
    std::atomic<float>* outputLevelParam = nullptr;
    std::atomic<float>* viewZoomXParam = nullptr;
    std::atomic<float>* viewZoomZParam = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LorenzAudioProcessor)
};
