#include "MainComponent.h"

MainComponent::MainComponent()
{
    // �e�L�X�g�G�f�B�^(�|�[�g�ԍ�)
    portNumberField.setText("9001", juce::dontSendNotification);
    portNumberField.setInputRestrictions(5, "0123456789");
    addAndMakeVisible(portNumberField);

    // ���x��
    addAndMakeVisible(portNumberLabel);
    portNumberLabel.setJustificationType(juce::Justification::centredRight);

    // Connect�{�^��
    addAndMakeVisible(connectButton);
    connectButton.onClick = [this] { connectButtonClicked(); };

    // �X�e�[�^�X���x��
    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    connectionStatusLabel.setJustificationType(juce::Justification::centredLeft);

    // OSCReceiver�̐ݒ�
    oscReceiver.addListener(this, "/avatar/parameters/HeartRate");
    oscReceiver.registerFormatErrorHandler([this](const char* data, int dataSize)
        {
            DBG("Invalid OSC Packet: " << dataSize << " bytes");
        });

    setSize(700, 400);
    startTimer(50); // UI�X�V�p�̃^�C�}�[
}

MainComponent::~MainComponent()
{
    stopTimer();
    oscReceiver.disconnect();
}

void MainComponent::paint(juce::Graphics& g)
{
    // �w�i�h��Ԃ�
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // ��ʂ̍�Ɨ̈���擾�i�㉺���E10�s�N�Z�����]�����Ƃ��j
    auto area = getLocalBounds().reduced(10);

    // �㕔��GUI�v�f�����������c��� centerArea �Ƃ���
    auto topArea = area.removeFromTop(40);
    // ������ resized() �� setBounds() ���Ă��邽�߁Apaint() �ł͕`�悵�Ȃ�

    // �c��̗̈���g���āA������BPM�p�̎l�p�`��`��
    auto centerArea = area; // 40�s�N�Z����菜�����c��

    const int boxWidth = 200;
    const int boxHeight = 120;
    // �����ɔz�u����
    int boxX = centerArea.getX() + (centerArea.getWidth() - boxWidth) / 2;
    int boxY = centerArea.getY() + (centerArea.getHeight() - boxHeight) / 2;

    juce::Rectangle<int> boxRect(boxX, boxY, boxWidth, boxHeight);

    // �l�p�`�̔w�i
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(boxRect.toFloat(), 10.0f);

    // �g��
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(boxRect.toFloat(), 10.0f, 3.0f);

    // BPM�̕�����`��
    {
        // �㕔�ɁuBPM�v
        auto labelArea = boxRect.removeFromTop(40);
        g.setFont(juce::Font(30.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawFittedText("BPM", labelArea, juce::Justification::centred, 1);

        // �����ɐ��l
        g.setFont(juce::Font(40.0f, juce::Font::bold));
        g.setColour(juce::Colours::yellow);
        g.drawFittedText(juce::String(currentBPM),
            boxRect, juce::Justification::centred, 1);
    }
}

void MainComponent::resized()
{
    // �S�̗̈�
    auto area = getLocalBounds().reduced(10);

    // �㕔40�s�N�Z�������o��
    auto topArea = area.removeFromTop(40);

    // �㕔�̃��C�A�E�g: 
    // [ Label(UDP Port Number) ][ TextEditor(portNumberField) ][ ConnectButton ][ StatusLabel ]
    {
        // �����珇�ɔz�u���Ă���
        portNumberLabel.setBounds(topArea.removeFromLeft(120));
        portNumberField.setBounds(topArea.removeFromLeft(80));
        connectButton.setBounds(topArea.removeFromLeft(100));
        connectionStatusLabel.setBounds(topArea);
    }

    // �c��� BPM�\���p�̗̈�ipaint()�ŕ`��j
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
            repaint(); // BPM�X�V
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
