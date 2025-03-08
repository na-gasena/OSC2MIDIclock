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
    juce::Label      portNumberLabel{ {}, "Port Number:" };
    juce::TextEditor portNumberField;
    juce::TextButton connectButton{ "Connect" };
    juce::Label      connectionStatusLabel{ {}, "Disconnected" };

    // MIDI Output�f�o�C�X�I��p
    juce::Label      midiOutputLabel{ {}, "MIDI Output:" };
    juce::ComboBox   midiOutputDeviceBox;

    // �I�����ꂽMIDI�f�o�C�X�ւ̏o��
    std::unique_ptr<juce::MidiOutput> midiOutput;

    // ���݂�BPM�l
    int currentBPM = 0;

    // MIDI�N���b�N���M�p�̃^�C�~���O�Ǘ�
    double nextClockTime = 0.0;  // ���ɃN���b�N�𑗐M���鎞���i�b�P�ʁj
    double lastBPM = 0.0;        // �O�񏈗�����BPM���L��

    // GUI �X�V�E���䃁�\�b�h
    void updateConnectionStatusLabel();
    void connectButtonClicked();
    void handleConnectError(int failedPort);
    void handleDisconnectError();
    void handleInvalidPortNumberEntered();
    bool isValidOscPort(int port) const;

    // MIDI�f�o�C�X�I��ComboBox���ω������Ƃ�
    void midiDeviceBoxChanged();

    // MIDI�N���b�N���M����
    void sendMidiClockIfNeeded(double currentTime);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
