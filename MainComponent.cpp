#include "MainComponent.h"

MainComponent::MainComponent()
{
    // GUI コンポーネントの配置
    portNumberLabel.setBounds(10, 18, 130, 25);
    addAndMakeVisible(portNumberLabel);

    portNumberField.setEditable(true, true, true);
    portNumberField.setBounds(140, 18, 50, 25);
    addAndMakeVisible(portNumberField);

    connectButton.setBounds(210, 18, 100, 25);
    addAndMakeVisible(connectButton);
    connectButton.onClick = [this] { connectButtonClicked(); };

    clearButton.setBounds(320, 18, 60, 25);
    addAndMakeVisible(clearButton);
    clearButton.onClick = [this] { clearButtonClicked(); };

    connectionStatusLabel.setBounds(450, 18, 240, 25);
    updateConnectionStatusLabel();
    addAndMakeVisible(connectionStatusLabel);

    oscLogListBox.setBounds(0, 60, 700, 340);
    addAndMakeVisible(oscLogListBox);

    oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    oscReceiver.registerFormatErrorHandler([this](const char* data, int dataSize)
        {
            oscLogList.add("Invalid OSC Packet: " + juce::String(dataSize) + " bytes");
            oscLogListBox.updateContent();
            oscLogListBox.scrollToEnsureRowIsOnscreen(oscLogList.size() - 1);
            oscLogListBox.repaint();
        });

    setSize(700, 400);
}

MainComponent::~MainComponent()
{
    stopTimer();
    oscReceiver.disconnect();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // 接続状態に応じたテキストを描画
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    if (isConnected)
    {
        g.drawText("OSC Connected", getLocalBounds(), juce::Justification::centred, true);
    }
    else
    {
        g.drawText("OSC Not Connected", getLocalBounds(), juce::Justification::centred, true);
    }
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

    addOSCMessage(message);
}

void MainComponent::timerCallback()
{
    // 定期的にUIを更新するなど
    repaint();
}

void MainComponent::updateConnectionStatusLabel()
{
    juce::String text = "Status: ";

    if (isConnected)
        text += "Connected to UDP port " + juce::String(8000);
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
    oscLogList.clear();
    oscLogListBox.updateContent();
    oscLogListBox.repaint();
}

void MainComponent::addOSCMessage(const juce::OSCMessage& message)
{
    oscLogList.add("OSC Message: " + message.getAddressPattern().toString() + ", " + juce::String(message.size()) + " argument(s)");

    if (!message.isEmpty())
    {
        for (auto* arg = message.begin(); arg != message.end(); ++arg)
            addOSCMessageArgument(*arg, 1);
    }

    oscLogListBox.updateContent();
    oscLogListBox.scrollToEnsureRowIsOnscreen(oscLogList.size() - 1);
    oscLogListBox.repaint();
}

void MainComponent::addOSCMessageArgument(const juce::OSCArgument& arg, int level)
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

    oscLogList.add(juce::String().paddedRight(' ', 2 * level) + "- " + typeAsString.paddedRight(' ', 12) + valueAsString);
}

void MainComponent::handleConnectError(int failedPort)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "OSC Connection error",
        "Error: could not connect to port " + juce::String(failedPort),
        "OK");
}

void MainComponent::handleDisconnectError()
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "Unknown error",
        "An unknown error occurred while trying to disconnect from UDP port.",
        "OK");
}

void MainComponent::handleInvalidPortNumberEntered()
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "Invalid port number",
        "Error: you have entered an invalid UDP port number.",
        "OK");
}

bool MainComponent::isValidOscPort(int port) const
{
    return port > 0 && port < 65536;
}
