#pragma once

#include <JuceHeader.h>

// MainComponent は OSCReceiver, Timer の機能を使う
class MainComponent : public juce::Component,
    public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>,
    public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // OSC コールバック
    void oscMessageReceived(const juce::OSCMessage& message) override;

    // Timer コールバック
    void timerCallback() override;

private:
    // juce::OSCReceiver インスタンス
    juce::OSCReceiver oscReceiver;

    // MIDI 出力などのメンバ
    std::unique_ptr<juce::MidiOutput> midiOutput;
    // ... etc ...

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
