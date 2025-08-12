#pragma once
#include <QWidget>
class QLineEdit; class QListWidget; class QPushButton; class QSpinBox; class QDoubleSpinBox; class QCheckBox; class QLabel; class QRadioButton; class QGroupBox;

class AssemblePage : public QWidget{
    Q_OBJECT
public:
    explicit AssemblePage(QWidget* parent=nullptr);
public slots:
    void assembleNow();
signals:
    void status(const QString&);
    void taskStarted(const QString&);
    void taskProgress(int,int);
    void taskSucceeded(const QString&);
    void taskFailed(const QString&);
private slots:
    void chooseFrames();
    void chooseOutput();
    void runGif();
    void runWebp();
    void onWorkerProgress(int,int);
    void onWorkerDone(bool ok, const QString& msg);
private:
    void assembleCommon(bool webp);
    bool currentModeIsWebp() const;
    void updateEnablements(bool webpTarget);
    void updateEffectiveTiming();
    QLineEdit* framesDir_;
    QLineEdit* outFile_;
    QSpinBox* delayMs_;
    QSpinBox* loopCount_;
    // Mode
    QRadioButton* gifRadio_;
    QRadioButton* webpRadio_;

    // Timing
    QCheckBox* useDelay_;
    QCheckBox* useLoop_;
    QCheckBox* optimize_;
    QDoubleSpinBox* fps_;
    QCheckBox* lossless_;
    QSpinBox* quality_;
    QSpinBox* method_;
    QCheckBox* dither_;
    QSpinBox* colors_;
    QSpinBox* width_;
    QSpinBox* height_;
    QGroupBox* gifGroup_;
    QGroupBox* webpGroup_;
    QLabel* effectiveTiming_;
    QListWidget* log_;
    QThread* workerThread_ = nullptr;
};