#include <QPushButton>
#include "FrameTimingEditor.hpp"
FrameTimingEditor::FrameTimingEditor(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Per-frame delay");
    auto* lay = new QVBoxLayout(this);
    auto* lbl = new QLabel("Delay (ms):", this);
    spin_ = new QSpinBox(this);
    spin_->setRange(0, 60000);
    spin_->setValue(100);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    auto* btnClear = bb->addButton("Clear", QDialogButtonBox::ResetRole);
    lay->addWidget(lbl);
    lay->addWidget(spin_);
    lay->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnClear, &QPushButton::clicked, this, [this](){ cleared_ = true; accept(); });
}
void FrameTimingEditor::setInitialDelay(std::optional<int> ms) { if (ms.has_value()) spin_->setValue(*ms); }
std::optional<int> FrameTimingEditor::delayMs() const { if (cleared_) return std::nullopt; return spin_->value(); }
