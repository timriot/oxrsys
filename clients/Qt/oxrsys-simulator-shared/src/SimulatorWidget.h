// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <QElapsedTimer>
#include <QHostAddress>
#include <QPointF>
#include <QSet>
#include <QWidget>
#include <QByteArray>
#include <QVector>

#include <oxrsys/protocol/Protocol.h>

#if OXRSYS_QT_SIMULATOR_HAS_FFMPEG
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
#endif

class QLabel;
class QPushButton;
class QTimer;
class QUdpSocket;
class SimulatorPreviewWidget;

class SimulatorWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorWidget(QWidget* parent = nullptr);
    ~SimulatorWidget() override;

    QString stateText() const;
    bool isConnected() const;

public slots:
    void startDiscovery();
    void disconnectFromRuntime();

signals:
    void stateTextChanged(const QString& state);

private slots:
    void readPendingDiscoveryDatagrams();
    void readPendingVideoDatagrams();
    void connectToDiscoveredRuntime();
    void sendTrackingSample();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    enum class State
    {
        Disconnected,
        Discovering,
        Discovered,
        Streaming,
    };

    void buildUi();
    void setState(State state, const QString& status);
    void updateControls();
    void updateServerSummary();
    void updateTelemetrySummary();
    void advanceSimulation(float deltaTime);
    void fillTrackingPacket(oxr::protocol::TrackingPacket& packet) const;
    bool startVideoReceiver();
    void stopVideoReceiver();
    void resetPendingVideoFrame();
    void handleVideoPacket(const oxr::protocol::VideoPacketHeader& header,
                           const char* payload,
                           qsizetype payloadSize);
    void deliverPendingVideoFrame();
#if OXRSYS_QT_SIMULATOR_HAS_FFMPEG
    bool ensureVideoDecoder();
    void resetVideoDecoder();
    bool decodeVideoFrame(const QByteArray& nalUnit);
#endif
    void setMouseCaptured(bool captured);
    void toggleMouseCaptured();
    void accumulateMouseDelta(const QPointF& delta);
    void setKeyPressed(int key, bool pressed);
    void resetInputState();
    QString discoveredServerName() const;

    QLabel* titleLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* serverLabel_ = nullptr;
    QLabel* telemetryLabel_ = nullptr;
    QLabel* hintLabel_ = nullptr;
    SimulatorPreviewWidget* previewWidget_ = nullptr;
    QPushButton* searchButton_ = nullptr;
    QPushButton* connectButton_ = nullptr;
    QPushButton* disconnectButton_ = nullptr;
    QUdpSocket* discoverySocket_ = nullptr;
    QUdpSocket* videoSocket_ = nullptr;
    QUdpSocket* controlSocket_ = nullptr;
    QUdpSocket* trackingSocket_ = nullptr;
    QTimer* trackingTimer_ = nullptr;

    QHostAddress serverAddress_;
    oxr::protocol::ServerAnnounce discoveredServer_{};
    State state_ = State::Disconnected;
    quint64 trackingPacketsSent_ = 0;
    quint64 videoPacketsReceived_ = 0;
    quint64 videoFramesDecoded_ = 0;
    quint64 videoFramesDropped_ = 0;
    quint64 decodeErrors_ = 0;
    uint32_t pendingVideoFrameIndex_ = UINT32_MAX;
    uint16_t pendingVideoTotalPackets_ = 0;
    uint16_t pendingVideoReceivedPackets_ = 0;
    int64_t pendingVideoPresentationTimeNs_ = 0;
    QByteArray pendingVideoFrameData_;
    QVector<uint16_t> pendingVideoPacketSizes_;
    QVector<uint8_t> pendingVideoPacketReceived_;
#if OXRSYS_QT_SIMULATOR_HAS_FFMPEG
    AVCodecContext* videoDecoder_ = nullptr;
    AVFrame* decodedFrame_ = nullptr;
    AVPacket* decodePacket_ = nullptr;
    SwsContext* swsContext_ = nullptr;
#endif
    QElapsedTimer poseClock_;
    QSet<int> pressedKeys_;
    QPointF lastMousePosition_;
    QPointF pendingMouseDelta_;
    bool hasLastMousePosition_ = false;
    bool mouseCaptured_ = false;
    float yaw_ = 0.0f;
    float pitch_ = 0.0f;
    float roll_ = 0.0f;
    float headPosition_[3] = {0.0f, 1.6f, 0.0f};
    float leftControllerPosition_[3] = {-0.2f, 1.3f, -0.4f};
    float rightControllerPosition_[3] = {0.2f, 1.3f, -0.4f};
};
