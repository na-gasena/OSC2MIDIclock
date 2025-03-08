#pragma once

#include <JuceHeader.h>

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
    // OSC ��M�@
    juce::OSCReceiver oscReceiver;

    bool isConnected = false;

    // GUI �R���|�[�l���g
    juce::Label portNumberLabel{ {}, "UDP Port Number:" };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::Label connectionStatusLabel{ {}, "Disconnected" };

    // ���݂�BPM�l��ێ�
    int currentBPM = 0;

    // GUI �X�V�E���䃁�\�b�h
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
