#include "MainComponent.h"

//==============================================================================
// OSCLogListBox の実装
//==============================================================================
OSCLogListBox::OSCLogListBox()
{
    setModel(this);
}

OSCLogListBox::~OSCLogListBox() {}

int OSCLogListBox::getNumRows()
{
    return oscLogList.size();
}

void OSCLogListBox::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    juce::ignoreUnused(rowIsSelected);
    if (juce::isPositiveAndBelow(row, oscLogList.size()))
    {
        g.setColour(juce::Colours::white);
        g.drawText(oscLogList[row],
            juce::Rectangle<int>(width, height).reduced(4, 0),
            juce::Justification::centredLeft, true);
    }
}

void OSCLogListBox::addOSCMessage(const juce::OSCMessage& message, int level)
{
    oscLogList.add(getIndentationString(level)
        + "- osc message, address = '"
        + message.getAddressPattern().toString()
        + "', "
        + juce::String(message.size())
        + " argument(s)");

    if (!message.isEmpty())
    {
        for (auto* arg = message.begin(); arg != message.end(); ++arg)
            addOSCMessageArgument(*arg, level + 1);
    }

    triggerAsyncUpdate();
}

void OSCLogListBox::addOSCBundle(const juce::OSCBundle& bundle, int level)
{
    juce::OSCTimeTag timeTag = bundle.getTimeTag();
    oscLogList.add(getIndentationString(level)
        + "- osc bundle, time tag = "
        + timeTag.toTime().toString(true, true, true, true));

    for (auto* element = bundle.begin(); element != bundle.end(); ++element)
    {
        if (element->isMessage())
            addOSCMessage(element->getMessage(), level + 1);
        else if (element->isBundle())
            addOSCBundle(element->getBundle(), level + 1);
    }

    triggerAsyncUpdate();
}

void OSCLogListBox::addOSCMessageArgument(const juce::OSCArgument& arg, int level)
{
    juce::String typeAsString;
    juce::String valueAsString;

    if (arg.isFloat32())
    {
        typeAsString = "float32";
        valueAsString = juce::String(arg.getFloat32());
    }
    else if (arg.isInt32())
    {
        typeAsString = "int32";
        valueAsString = juce::String(arg.getInt32());
    }
    else if (arg.isString())
    {
        typeAsString = "string";
        valueAsString = arg.getString();
    }
    else if (arg.isBlob())
    {
        typeAsString = "blob";
        auto& blob = arg.getBlob();
        valueAsString = juce::String::fromUTF8((const char*)blob.getData(), (int)blob.getSize());
    }
    else
    {
        typeAsString = "(unknown)";
    }

    oscLogList.add(getIndentationString(level + 1) + "- " + typeAsString.paddedRight(' ', 12) + valueAsString);
}

void OSCLogListBox::addInvalidOSCPacket(const char* /*data*/, int dataSize)
{
    oscLogList.add("- (" + juce::String(dataSize) + " bytes with invalid format)");
    triggerAsyncUpdate();
}

void OSCLogListBox::clear()
{
    oscLogList.clear();
    triggerAsyncUpdate();
}

juce::String OSCLogListBox::getIndentationString(int level)
{
    return juce::String().paddedRight(' ', 2 * level);
}

void OSCLogListBox::handleAsyncUpdate()
{
    updateContent();
    scrollToEnsureRowIsOnscreen(oscLogList.size() - 1);
    repaint();
}


//==============================================================================
// MainComponent の実装
//==============================================================================
MainComponent::MainComponent()
{
    // GUI コンポーネントの配置
    portNumberLabel.setBounds(10, 18, 130, 25);
    addAndMakeVisible(portNumberLabel);

    portNumberField.setText("8000", juce::dontSendNotification);
    portNumberField.setBounds(140, 18, 100, 25);
    portNumberField.setInputRestrictions(5, "0123456789");
    addAndMakeVisible(portNumberField);

    connectButton.setBounds(250, 18, 100, 25);
    addAndMakeVisible(connectButton);
    connectButton.onClick = [this] { connectButtonClicked(); };

    clearButton.setBounds(360, 18, 60, 25);
    addAndMakeVisible(clearButton);
    clearButton.onClick = [this] { clearButtonClicked(); };

    connectionStatusLabel.setBounds(430, 18, 240, 25);
    updateConnectionStatusLabel();
    addAndMakeVisible(connectionStatusLabel);

    // OSCLogListBox の配置
    oscLogListBox.setBounds(0, 60, 700, 340);
    addAndMakeVisible(oscLogListBox);

    // OSCReceiver の設定（/avatar/parameters/HeartRate に限定）
    oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    oscReceiver.registerFormatErrorHandler([this](const char* data, int dataSize)
        {
            oscLogListBox.addInvalidOSCPacket(data, dataSize);
        });

    setSize(700, 400);
    startTimer(50); // 必要に応じてタイマーでUI更新
}

MainComponent::~MainComponent()
{
    stopTimer();
    oscReceiver.disconnect();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    if (isConnected)
        g.drawText("OSC Connected", getLocalBounds(), juce::Justification::centred, true);
    else
        g.drawText("OSC Not Connected", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // レイアウト更新（必要に応じてここで再配置可能）
}

void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    DBG("Received OSC Message: " + message.getAddressPattern().toString());

    if (message.getAddressPattern().toString() == "/avatar/parameters/HeartRate")
    {
        if (message.size() > 0 && message[0].isInt32())
        {
            int value = message[0].getInt32();
            DBG("Received BPM: " << value);
            // ここで BPM の値を用いて MIDI クロック送信などの処理を実装可能
        }
    }

    // 受信した OSC メッセージをリストに追加して GUI 表示
    oscLogListBox.addOSCMessage(message);
}

void MainComponent::timerCallback()
{
    // 定期的な再描画など（ここでは paint() を更新）
    repaint();
}

void MainComponent::updateConnectionStatusLabel()
{
    juce::String text = "Status: ";
    if (isConnected)
        text += "Connected to UDP port " + portNumberField.getText();
    else
        text += "Disconnected";

    auto textColour = isConnected ? juce::Colours::green : juce::Colours::red;

    connectionStatusLabel.setText(text, juce::dontSendNotification);
    connectionStatusLabel.setFont(juce::Font(15.00f, juce::Font::bold));
    connectionStatusLabel.setColour(juce::Label::textColourId, textColour);
    connectionStatusLabel.setJustificationType(juce::Justification::centredRight);
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

void MainComponent::clearButtonClicked()
{
    oscLogListBox.clear();
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
