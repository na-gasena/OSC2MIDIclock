#pragma once

#include <JuceHeader.h>

// MainComponent �� OSCReceiver, Timer �̋@�\���g��
class MainComponent : public juce::Component,
    public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>,
    public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // OSC �R�[���o�b�N
    void oscMessageReceived(const juce::OSCMessage& message) override;

    // Timer �R�[���o�b�N
    void timerCallback() override;

private:
    // juce::OSCReceiver �C���X�^���X
    juce::OSCReceiver oscReceiver;

    // MIDI �o�͂Ȃǂ̃����o
    std::unique_ptr<juce::MidiOutput> midiOutput;
    // ... etc ...

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
