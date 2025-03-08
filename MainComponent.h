#pragma once

#include <JuceHeader.h>

//==============================================================================
// OSC���b�Z�[�W�^�o���h���̃��O�\���p�J�X�^�� ListBox
// �錾����
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
// OSC��M��GUI������s�����C���R���|�[�l���g
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

    // OSC �R�[���o�b�N
    void oscMessageReceived(const juce::OSCMessage& message) override;

    // Timer �R�[���o�b�N
    void timerCallback() override;

private:
    // OSC ��M�@
    juce::OSCReceiver oscReceiver;

    // MIDI�o�́i��������\��j
    std::unique_ptr<juce::MidiOutput> midiOutput;

    bool isConnected = false;

    // GUI �R���|�[�l���g
    juce::Label portNumberLabel{ {}, "UDP Port Number:" };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::TextButton clearButton{ "Clear" };
    juce::Label connectionStatusLabel;

    OSCLogListBox oscLogListBox;

    // GUI �X�V�E���䃁�\�b�h
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void clearButtonClicked();
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
