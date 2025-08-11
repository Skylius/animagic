#include <QPushButton>
#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <optional>
#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
class FrameTimingEditor : public QDialog {
    Q_OBJECT
public:
    explicit FrameTimingEditor(QWidget* parent = nullptr);
    void setInitialDelay(std::optional<int> ms);
    std::optional<int> delayMs() const;
private:
    QSpinBox* spin_ = nullptr;
    bool cleared_ = false;
};
