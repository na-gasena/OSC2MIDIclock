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

    // �ڑ���Ԃ����������o�ϐ�
    bool isConnected = false;

    // GUI �R���|�[�l���g
    juce::Label portNumberLabel{ {}, "UDP Port Number: " };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::TextButton clearButton{ "Clear" };
    juce::Label connectionStatusLabel;
    juce::ListBox oscLogListBox;

    // OSC ���b�Z�[�W���O
    juce::StringArray oscLogList;

    // GUI �X�V���\�b�h
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void clearButtonClicked();
    void addOSCMessage(const juce::OSCMessage& message);
    void addOSCMessageArgument(const juce::OSCArgument& arg, int level);
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

