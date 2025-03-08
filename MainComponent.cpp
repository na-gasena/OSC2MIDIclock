#include "MainComponent.h"

MainComponent::MainComponent()
{
    // テキストエディタ(ポート番号)
    portNumberField.setText("9001", juce::dontSendNotification);
    portNumberField.setInputRestrictions(5, "0123456789");
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

    // OSCReceiverの設定
    oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    oscReceiver.registerFormatErrorHandler([this](const char* data, int dataSize)
        {
            DBG("Invalid OSC Packet: " << dataSize << " bytes");
        });

    setSize(700, 400);
    startTimer(50); // UI更新用のタイマー
}

MainComponent::~MainComponent()
{
    stopTimer();
    oscReceiver.disconnect();
}

void MainComponent::paint(juce::Graphics& g)
{
    // 背景塗りつぶし
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // 画面の作業領域を取得（上下左右10ピクセルずつ余白をとる例）
    auto area = getLocalBounds().reduced(10);

    // 上部のGUI要素分を除いた残りを centerArea とする
    auto topArea = area.removeFromTop(40);
    // ここは resized() で setBounds() しているため、paint() では描画しない

    // 残りの領域を使って、中央にBPM用の四角形を描画
    auto centerArea = area; // 40ピクセル取り除いた残り

    const int boxWidth = 200;
    const int boxHeight = 120;
    // 中央に配置する
    int boxX = centerArea.getX() + (centerArea.getWidth() - boxWidth) / 2;
    int boxY = centerArea.getY() + (centerArea.getHeight() - boxHeight) / 2;

    juce::Rectangle<int> boxRect(boxX, boxY, boxWidth, boxHeight);

    // 四角形の背景
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(boxRect.toFloat(), 10.0f);

    // 枠線
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(boxRect.toFloat(), 10.0f, 3.0f);

    // BPMの文字を描画
    {
        // 上部に「BPM」
        auto labelArea = boxRect.removeFromTop(40);
        g.setFont(juce::Font(30.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText("BPM", labelArea, juce::Justification::centred, 1);

        // 下部に数値
        g.setFont(juce::Font(40.0f, juce::Font::bold));
        g.setColour(juce::Colours::yellow);
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

    // 上部のレイアウト: 
    // [ Label(UDP Port Number) ][ TextEditor(portNumberField) ][ ConnectButton ][ StatusLabel ]
    {
        // 左から順に配置していく
        portNumberLabel.setBounds(topArea.removeFromLeft(120));
        portNumberField.setBounds(topArea.removeFromLeft(80));
        connectButton.setBounds(topArea.removeFromLeft(100));
        connectionStatusLabel.setBounds(topArea);
    }

    // 残りは BPM表示用の領域（paint()で描画）
}

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
            repaint(); // BPM更新
        }
    }
}

void MainComponent::timerCallback()
{
    repaint();
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
