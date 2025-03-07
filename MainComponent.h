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

    // 接続状態を示すメンバ変数
    bool isConnected = false;

    // GUI コンポーネント
    juce::Label portNumberLabel{ {}, "UDP Port Number: " };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::TextButton clearButton{ "Clear" };
    juce::Label connectionStatusLabel;
    juce::ListBox oscLogListBox;

    // OSC メッセージログ
    juce::StringArray oscLogList;

    // GUI 更新メソッド
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

