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

    // OSC コールバック
    void oscMessageReceived(const juce::OSCMessage& message) override;

    // Timer コールバック
    void timerCallback() override;

private:
    // OSC 受信機
    juce::OSCReceiver oscReceiver;

    bool isConnected = false;

    // GUI コンポーネント
    juce::Label portNumberLabel{ {}, "UDP Port Number:" };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::Label connectionStatusLabel{ {}, "Disconnected" };

    // 現在のBPM値を保持
    int currentBPM = 0;

    // GUI 更新・制御メソッド
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
