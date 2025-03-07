#include "MainComponent.h"

MainComponent::MainComponent()
{
    // OSCReceiver: ポート8000で接続
    if (!oscReceiver.connect(8000))
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Error", "Could not connect to port 8000");
    }
    else
    {
        // アドレス"/avatar/parameters/HeartRate"を監視
        oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    }

    // MIDI 出力デバイスを開く
    auto devices = juce::MidiOutput::getAvailableDevices();
    if (!devices.isEmpty())
        midiOutput = juce::MidiOutput::openDevice(devices[0].identifier);

    // ... そのほか初期化 ...

    // Timer開始
    startTimer(200);

    setSize(600, 400);
}

MainComponent::~MainComponent()
{
    stopTimer();

    // 切断
    oscReceiver.disconnect();

    // ... そのほか終了処理 ...
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    // レイアウト更新
}

void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    if (message.getAddressPattern().toString() == "/avatar/parameters/HeartRate")
    {
        if (message.size() > 0 && message[0].isInt32())
        {
            int value = message[0].getInt32();
            DBG("Received BPM: " << value);

            // ここで BPM を更新 -> MIDI クロックに反映
        }
    }
}

void MainComponent::timerCallback()
{
    // 定期的にUIを更新するなど
}
