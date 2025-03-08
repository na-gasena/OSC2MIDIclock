#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // ポート番号入力欄
    portNumberField.setText("9001", juce::dontSendNotification);
    portNumberField.setInputRestrictions(5, "0123456789");
    portNumberField.setJustification(juce::Justification::centred);  // テキスト中央ぞろえ
    portNumberField.setFont(juce::Font(10.0f));
    addAndMakeVisible(portNumberField);

    // ラベル
    addAndMakeVisible(portNumberLabel);
    portNumberLabel.setJustificationType(juce::Justification::centredRight);

    // Connectボタン
    addAndMakeVisible(connectButton);
    connectButton.onClick = [this] { connectButtonClicked(); };

    // ステータスラベル
    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    connectionStatusLabel.setJustificationType(juce::Justification::centredLeft);

    //=== MIDI Outputデバイス選択用 ===
    addAndMakeVisible(midiOutputLabel);
    midiOutputLabel.setJustificationType(juce::Justification::centredRight);

    // ComboBox 初期化: 利用可能なデバイスを列挙
    auto devices = juce::MidiOutput::getAvailableDevices();
    int index = 1; // ComboBoxのItem IDは1から
    for (auto& device : devices)
    {
        midiOutputDeviceBox.addItem(device.name, index++);
    }

    // ComboBoxの変化時コールバック
    midiOutputDeviceBox.onChange = [this]
        {
            midiDeviceBoxChanged();
        };
    addAndMakeVisible(midiOutputDeviceBox);

    //=== OSCReceiverの設定 ===
    oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    oscReceiver.registerFormatErrorHandler([this](const char* data, int dataSize)
        {
            DBG("Invalid OSC Packet: " << dataSize << " bytes");
        });

    // 初期ウィンドウサイズ
    setSize(700, 400);

    // タイマー開始 (50msごと)
    startTimer(50);

    // クロック送信用
    nextClockTime = 0.0;
    lastBPM = 0.0;
}

MainComponent::~MainComponent()
{
    stopTimer();
    oscReceiver.disconnect();

    // MIDIデバイスを閉じる
    midiOutput.reset();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // 背景塗りつぶし
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // 上部GUIの領域を除いた残りに BPM用の四角形を描画
    auto area = getLocalBounds().reduced(10);

    // 上部40ピクセルを取り出す
    area.removeFromTop(40);

    // 中央に配置
    const int boxWidth = 500;
    const int boxHeight = 300;

    int boxX = area.getX() + (area.getWidth() - boxWidth) / 2;
    int boxY = area.getY() + (area.getHeight() - boxHeight) / 2;

    juce::Rectangle<int> boxRect(boxX, boxY, boxWidth, boxHeight);

    // 四角形の背景
    g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(boxRect.toFloat(), 10.0f);

    // 枠線
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(boxRect.toFloat(), 10.0f, 3.0f);

    // 「BPM」文字
    {
        auto labelArea = boxRect.removeFromTop(90);
        g.setFont(juce::Font(90.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText("BPM", labelArea, juce::Justification::centred, 1);
    }

    // BPM数値
    {
        // 垂直方向の位置調整
        int verticalOffset = -20; // ここで垂直方向のオフセットを設定

        boxRect.translate(0, verticalOffset); // 垂直方向にオフセットを適用

        g.setFont(juce::Font(250.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText(juce::String(currentBPM),
            boxRect, juce::Justification::centred, 1);
    }
}

void MainComponent::resized()
{
    // 全体領域
    auto area = getLocalBounds().reduced(10);

    // 上部40ピクセルを取り出す
    auto topArea = area.removeFromTop(40);

    // 左から順に配置
    portNumberLabel.setBounds(topArea.removeFromLeft(120));
    portNumberField.setBounds(topArea.removeFromLeft(80));
    topArea.removeFromLeft(10); // ← Connectボタンとの間に余白（10px）を追加
    connectButton.setBounds(topArea.removeFromLeft(100));

    midiOutputLabel.setBounds(topArea.removeFromLeft(100));
    midiOutputDeviceBox.setBounds(topArea.removeFromLeft(150));

    connectionStatusLabel.setBounds(topArea);
}

//==============================================================================
void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    auto address = message.getAddressPattern().toString();
    DBG("Received OSC Message: " << address);

    if (address == "/avatar/parameters/HeartRate")
    {
        if (message.size() > 0 && message[0].isInt32())
        {
            currentBPM = message[0].getInt32();
            DBG("Received BPM: " << currentBPM);
            repaint(); // BPM表示を更新
        }
    }
}

void MainComponent::timerCallback()
{
    // 画面再描画
    repaint();

    // MIDIクロック送信処理
    double currentTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    sendMidiClockIfNeeded(currentTime);
}

void MainComponent::updateConnectionStatusLabel()
{
    juce::String text;
    if (isConnected)
        text = "Connected to UDP port " + portNumberField.getText();
    else
        text = "Disconnected";

    auto textColour = isConnected ? juce::Colours::green : juce::Colours::red;

    connectionStatusLabel.setText(text, juce::dontSendNotification);
    connectionStatusLabel.setColour(juce::Label::textColourId, textColour);
}

void MainComponent::connectButtonClicked()
{
    if (!isConnected)
    {
        int portToConnect = portNumberField.getText().getIntValue();

        if (!isValidOscPort(portToConnect))
        {
            handleInvalidPortNumberEntered();
            return;
        }

        if (oscReceiver.connect(portToConnect))
        {
            isConnected = true;
            connectButton.setButtonText("Disconnect");
            DBG("Connected to port " << portToConnect);
        }
        else
        {
            handleConnectError(portToConnect);
        }
    }
    else
    {
        if (oscReceiver.disconnect())
        {
            isConnected = false;
            connectButton.setButtonText("Connect");
            DBG("Disconnected from port");
        }
        else
        {
            handleDisconnectError();
        }
    }

    updateConnectionStatusLabel();
}

void MainComponent::midiDeviceBoxChanged()
{
    // ComboBoxの選択ID (1〜) を取得
    auto selectedId = midiOutputDeviceBox.getSelectedId();
    if (selectedId <= 0)
    {
        // 何も選択されていない場合
        midiOutput.reset();
        return;
    }

    // MIDI Outputデバイス一覧を取得して、選択されたものをオープン
    auto devices = juce::MidiOutput::getAvailableDevices();
    int index = selectedId - 1; // IDは1始まりなので -1

    if (juce::isPositiveAndBelow(index, devices.size()))
    {
        auto deviceInfo = devices[index];
        DBG("Opening MIDI device: " << deviceInfo.name);

        // 既存デバイスを閉じる
        midiOutput.reset();

        // 新しいデバイスを開く
        midiOutput = juce::MidiOutput::openDevice(deviceInfo.identifier);
    }
}

void MainComponent::sendMidiClockIfNeeded(double currentTime)
{
    // BPMに応じてクロック送信
    // BPMが0以下の場合は送信しない
    if (currentBPM <= 0 || midiOutput == nullptr)
        return;

    // BPMが変わった場合はタイミングをリセット
    if (static_cast<double>(currentBPM) != lastBPM)
    {
        nextClockTime = currentTime; // 次の送信時刻を"今"にする
        lastBPM = static_cast<double>(currentBPM);
    }

    // 1拍(QuarterNote)あたり24パルス → 1分間で (BPM * 24) パルス
    // 1秒間に (BPM * 24 / 60) パルス
    double pulsesPerSecond = (currentBPM * 24.0) / 60.0;

    // currentTime >= nextClockTime に達したらクロックを送信
    // 1回送信するごとに nextClockTime を 1/pulsesPerSecond 進める
    while (currentTime >= nextClockTime)
    {
        // MIDIクロック送信
        midiOutput->sendMessageNow(juce::MidiMessage::midiClock());

        nextClockTime += 1.0 / pulsesPerSecond;
    }
}

void MainComponent::handleConnectError(int failedPort)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "OSC Connection error",
        "Error: could not connect to port " + juce::String(failedPort),
        "OK");
    DBG("Error: could not connect to port " << failedPort);
}

void MainComponent::handleDisconnectError()
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "Unknown error",
        "An unknown error occurred while trying to disconnect from UDP port.",
        "OK");
    DBG("Unknown error occurred while trying to disconnect from UDP port.");
}

void MainComponent::handleInvalidPortNumberEntered()
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "Invalid port number",
        "Error: you have entered an invalid UDP port number.",
        "OK");
    DBG("Invalid port number entered.");
}

bool MainComponent::isValidOscPort(int port) const
{
    return port > 0 && port < 65536;
}
