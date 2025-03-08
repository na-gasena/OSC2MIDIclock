#pragma once

#include <JuceHeader.h>

//==============================================================================
// OSCメッセージ／バンドルのログ表示用カスタム ListBox
// 宣言部分
//==============================================================================
class OSCLogListBox : public juce::ListBox,
    private juce::ListBoxModel,
    private juce::AsyncUpdater
{
public:
    OSCLogListBox();
    ~OSCLogListBox() override;

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void addOSCMessage(const juce::OSCMessage& message, int level = 0);
    void addOSCBundle(const juce::OSCBundle& bundle, int level = 0);
    void addOSCMessageArgument(const juce::OSCArgument& arg, int level);
    void addInvalidOSCPacket(const char* data, int dataSize);
    void clear();

private:
    juce::String getIndentationString(int level);
    void handleAsyncUpdate() override;

    juce::StringArray oscLogList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCLogListBox)
};


//==============================================================================
// OSC受信とGUI操作を行うメインコンポーネント
//==============================================================================
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

    // MIDI出力（今後実装予定）
    std::unique_ptr<juce::MidiOutput> midiOutput;

    bool isConnected = false;

    // GUI コンポーネント
    juce::Label portNumberLabel{ {}, "UDP Port Number:" };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::TextButton clearButton{ "Clear" };
    juce::Label connectionStatusLabel;

    OSCLogListBox oscLogListBox;

    // GUI 更新・制御メソッド
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void clearButtonClicked();
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
