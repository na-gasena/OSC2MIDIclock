#include "MainComponent.h"

MainComponent::MainComponent()
{
    // OSCReceiver: �|�[�g8000�Őڑ�
    if (!oscReceiver.connect(8000))
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Error", "Could not connect to port 8000");
    }
    else
    {
        // �A�h���X"/avatar/parameters/HeartRate"���Ď�
        oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    }

    // MIDI �o�̓f�o�C�X���J��
    auto devices = juce::MidiOutput::getAvailableDevices();
    if (!devices.isEmpty())
        midiOutput = juce::MidiOutput::openDevice(devices[0].identifier);

    // ... ���̂ق������� ...

    // Timer�J�n
    startTimer(200);

    setSize(600, 400);
}

MainComponent::~MainComponent()
{
    stopTimer();

    // �ؒf
    oscReceiver.disconnect();

    // ... ���̂ق��I������ ...
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    // ���C�A�E�g�X�V
}

void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    if (message.getAddressPattern().toString() == "/avatar/parameters/HeartRate")
    {
        if (message.size() > 0 && message[0].isInt32())
        {
            int value = message[0].getInt32();
            DBG("Received BPM: " << value);

            // ������ BPM ���X�V -> MIDI �N���b�N�ɔ��f
        }
    }
}

void MainComponent::timerCallback()
{
    // ����I��UI���X�V����Ȃ�
}
