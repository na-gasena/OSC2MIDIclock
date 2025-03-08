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
    juce::Label      portNumberLabel{ {}, "Port Number:" };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::Label      connectionStatusLabel{ {}, "Disconnected" };

    // MIDI Outputデバイス選択用
    juce::Label      midiOutputLabel{ {}, "MIDI Output:" };
    juce::ComboBox   midiOutputDeviceBox;

    // 選択されたMIDIデバイスへの出力
    std::unique_ptr<juce::MidiOutput> midiOutput;

    // 現在のBPM値
    int currentBPM = 0;

    // MIDIクロック送信用のタイミング管理
    double nextClockTime = 0.0;  // 次にクロックを送信する時刻（秒単位）
    double lastBPM = 0.0;        // 前回処理時のBPMを記憶

    // GUI 更新・制御メソッド
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    // MIDIデバイス選択ComboBoxが変化したとき
    void midiDeviceBoxChanged();

    // MIDIクロック送信処理
    void sendMidiClockIfNeeded(double currentTime);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
