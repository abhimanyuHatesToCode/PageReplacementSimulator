#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QRegularExpression>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    scene(new QGraphicsScene(this)),
    animTimer(new QTimer(this))
{
    setupUI();
    connect(animTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
}

void MainWindow::setupUI() {
    setWindowTitle("Page Replacement Simulator — Animated");

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Controls row
    QHBoxLayout *controls = new QHBoxLayout();
    controls->setSpacing(10);

    controls->addWidget(new QLabel("Algorithm:"));
    algoBox = new QComboBox();
    algoBox->addItems({"FIFO","LRU","Optimal"});
    controls->addWidget(algoBox);

    controls->addWidget(new QLabel("Frames:"));
    framesBox = new QComboBox();
    framesBox->addItems({"1","2","3","4","5"});
    framesBox->setCurrentIndex(2); // default 3
    controls->addWidget(framesBox);

    controls->addWidget(new QLabel("Reference string:"));
    inputField = new QLineEdit();
    inputField->setPlaceholderText("e.g. 7 0 1 2 0 3 0 4");
    inputField->setMinimumWidth(260);
    controls->addWidget(inputField);

    runButton = new QPushButton("Run Simulation");
    controls->addWidget(runButton);

    mainLayout->addLayout(controls);

    // Animation controls
    QHBoxLayout *animControls = new QHBoxLayout();
    startBtn = new QPushButton("Start");
    pauseBtn = new QPushButton("Pause");
    prevBtn = new QPushButton("Prev");
    nextBtn = new QPushButton("Next");
    restartBtn = new QPushButton("Restart");
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(150, 2000);
    speedSlider->setValue(timerIntervalMs);
    QLabel *speedLabel = new QLabel("Speed (ms):");

    animControls->addWidget(startBtn);
    animControls->addWidget(pauseBtn);
    animControls->addWidget(prevBtn);
    animControls->addWidget(nextBtn);
    animControls->addWidget(restartBtn);
    animControls->addStretch();
    animControls->addWidget(speedLabel);
    animControls->addWidget(speedSlider);

    mainLayout->addLayout(animControls);

    // Graphics view
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(360);
    view->setBackgroundBrush(QColor("#1e1e2a")); // dark background for visibility
    view->setStyleSheet("border: 2px solid #333;");
    mainLayout->addWidget(view);

    // Summary box
    summaryBox = new QTextEdit();
    summaryBox->setReadOnly(true);
    summaryBox->setMinimumHeight(120);
    mainLayout->addWidget(summaryBox);

    setCentralWidget(central);

    // Connections
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runSimulation);
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::startAnimation);
    connect(pauseBtn, &QPushButton::clicked, this, &MainWindow::pauseAnimation);
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(prevBtn, &QPushButton::clicked, this, &MainWindow::prevStep);
    connect(restartBtn, &QPushButton::clicked, this, &MainWindow::restartAnimation);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::adjustSpeed);

    // initial state
    pauseBtn->setEnabled(false);
    startBtn->setEnabled(false);
    nextBtn->setEnabled(false);
    prevBtn->setEnabled(false);
    restartBtn->setEnabled(false);
}

void MainWindow::clearScene() {
    // delete PageItems
    for (auto &kv : activePages) {
        if (kv.second) {
            scene->removeItem(kv.second);
            delete kv.second;
        }
    }
    activePages.clear();

    for (auto r : frameRects) {
        if (r) {
            scene->removeItem(r);
            delete r;
        }
    }
    frameRects.clear();
    frameTargets.clear();
    scene->clear();
}

void MainWindow::drawFrames(int count) {
    // center frames horizontally
    const qreal frameW = 70;
    const qreal frameH = 46;
    const qreal spacing = 18;

    qreal totalW = count * frameW + (count - 1) * spacing;
    qreal startX = (view->viewport()->width() - totalW) / 2.0;
    if (startX < 20) startX = 20;

    qreal y = 120;

    frameRects.clear();
    frameTargets.clear();

    for (int i = 0; i < count; ++i) {
        qreal x = startX + i * (frameW + spacing);
        QGraphicsRectItem *r = scene->addRect(x, y, frameW, frameH, QPen(Qt::white, 2), QBrush(Qt::NoBrush));
        frameRects.push_back(r);
        frameTargets.push_back(QPointF(x + (frameW - 60)/2.0, y + (frameH - 36)/2.0)); // PageItem target (centered)
    }
}

static QStringList splitWhitespace(const QString &s) {
    return s.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
}

void MainWindow::runSimulation() {
    // parse frames and input
    QStringList parts = splitWhitespace(inputField->text().trimmed());
    if (parts.isEmpty()) {
        QMessageBox::warning(this, "Input error", "Please enter a reference string (space-separated integers).");
        return;
    }

    std::vector<int> refs;
    for (const QString &p : parts) {
        bool ok = false;
        int val = p.toInt(&ok);
        if (!ok || val < 0) {
            QMessageBox::warning(this, "Input error", "Reference string must contain non-negative integers only.");
            return;
        }
        refs.push_back(val);
    }

    frameCount = framesBox->currentText().toInt();
    if (frameCount <= 0) frameCount = 3;

    // run simulator
    QString algo = algoBox->currentText();
    if (algo == "FIFO") {
        FIFO_Simulator sim(frameCount, refs);
        currentResult = sim.run();
    } else if (algo == "LRU") {
        LRU_Simulator sim(frameCount, refs);
        currentResult = sim.run();
    } else {
        Optimal_Simulator sim(frameCount, refs);
        currentResult = sim.run();
    }

    steps = currentResult.steps;
    currentIndex = 0;

    // prepare scene
    clearScene();
    drawFrames(frameCount);

    // enable controls
    startBtn->setEnabled(!steps.empty());
    pauseBtn->setEnabled(false);
    nextBtn->setEnabled(!steps.empty());
    prevBtn->setEnabled(false);
    restartBtn->setEnabled(false);

    // show summary top
    summaryBox->clear();
    summaryBox->append(QString("Total references: %1").arg((int)refs.size()));
    summaryBox->append(QString("Page faults: %1").arg(currentResult.page_faults));
    summaryBox->append(QString("Page hits: %1").arg(currentResult.page_hits));
    double hitRatio = (refs.size() > 0) ? (double)currentResult.page_hits / refs.size() : 0.0;
    summaryBox->append(QString("Hit ratio: %1").arg(QString::number(hitRatio, 'f', 3)));
    summaryBox->append("\nStep-by-step:");

    // display first step (not animated) to show initial state
    displayStep(0);
}

void MainWindow::startAnimation() {
    if (steps.empty()) return;
    animTimer->start(timerIntervalMs);
    startBtn->setEnabled(false);
    pauseBtn->setEnabled(true);
    nextBtn->setEnabled(false);
    prevBtn->setEnabled(false);
    restartBtn->setEnabled(true);
}

void MainWindow::pauseAnimation() {
    animTimer->stop();
    startBtn->setEnabled(true);
    pauseBtn->setEnabled(false);
    nextBtn->setEnabled(true);
    prevBtn->setEnabled(currentIndex > 0);
}

void MainWindow::onTimerTick() {
    if (currentIndex < (int)steps.size() - 1) {
        ++currentIndex;
        displayStep(currentIndex);
    } else {
        animTimer->stop();
        startBtn->setEnabled(false);
        pauseBtn->setEnabled(false);
        nextBtn->setEnabled(false);
        prevBtn->setEnabled(currentIndex > 0);
        restartBtn->setEnabled(true);
        QMessageBox::information(this, "Simulation", "Animation finished.");
    }
}

void MainWindow::displayStep(int idx) {
    if (idx < 0 || idx >= (int)steps.size()) return;
    const SimulationStep &s = steps[idx];

    // For pages that are present in this frame set, ensure PageItem exists & animate it to correct frame.
    // For pages that are no longer present, animate them out and delete.

    // Mark seen pages this step
    std::map<int, bool> present;
    for (int p : s.frames) if (p != -1) present[p] = true;

    // Remove pages not present
    std::vector<int> toRemove;
    for (auto &kv : activePages) {
        int page = kv.first;
        if (!present.count(page)) {
            toRemove.push_back(page);
        }
    }
    for (int page : toRemove) {
        PageItem *item = activePages[page];
        if (!item) continue;
        QPropertyAnimation *anim = new QPropertyAnimation(item, "y");
        anim->setDuration(420);
        anim->setEndValue(scene->height() + 80);
        connect(anim, &QPropertyAnimation::finished, [this, item, page]() {
            scene->removeItem(item);
            activePages.erase(page);
            delete item;
        });
        anim->start(QPropertyAnimation::DeleteWhenStopped);
    }

    // Place/animate pages to their target frames
    for (int i = 0; i < (int)s.frames.size(); ++i) {
        int page = s.frames[i];
        if (page == -1) continue;
        QPointF target = frameTargets[i];

        PageItem *item = nullptr;
        if (activePages.count(page)) {
            item = activePages[page];
            // animate position if not there
            if (!qFuzzyCompare(item->x(), target.x()) || !qFuzzyCompare(item->y(), target.y())) {
                QPropertyAnimation *ax = new QPropertyAnimation(item, "x");
                ax->setDuration(360);
                ax->setEndValue(target.x());
                ax->start(QPropertyAnimation::DeleteWhenStopped);

                QPropertyAnimation *ay = new QPropertyAnimation(item, "y");
                ay->setDuration(360);
                ay->setEndValue(target.y());
                ay->start(QPropertyAnimation::DeleteWhenStopped);
            }
        } else {
            // create off-screen above
            item = new PageItem(page);
            item->setPos(target.x(), -80);
            scene->addItem(item);
            activePages[page] = item;

            QPropertyAnimation *ay = new QPropertyAnimation(item, "y");
            ay->setDuration(420);
            ay->setEndValue(target.y());
            ay->setEasingCurve(QEasingCurve::OutBounce);
            ay->start(QPropertyAnimation::DeleteWhenStopped);
        }
    }

    // Highlight referenced page (hit -> green, fault -> red)
    if (s.reference != -1 && activePages.count(s.reference)) {
        PageItem *refItem = activePages[s.reference];
        QColor c = s.hit ? QColor(0, 200, 0) : QColor(220, 40, 40);
        refItem->flashColor(c);
    }

    // Append to summary step-by-step
    QString line = QString("Ref %1: %2").arg(s.reference).arg(s.hit ? "HIT" : "FAULT");
    summaryBox->append(line);

    // Update control state
    prevBtn->setEnabled(currentIndex > 0);
    nextBtn->setEnabled(currentIndex < (int)steps.size() - 1 && !animTimer->isActive());
    restartBtn->setEnabled(true);
}

void MainWindow::nextStep() {
    if (currentIndex < (int)steps.size() - 1) {
        ++currentIndex;
        displayStep(currentIndex);
    }
}

void MainWindow::prevStep() {
    if (currentIndex > 0) {
        --currentIndex;
        displayStep(currentIndex);
    }
}

void MainWindow::restartAnimation() {
    animTimer->stop();
    // clear all existing page items
    for (auto &kv : activePages) {
        if (kv.second) {
            scene->removeItem(kv.second);
            delete kv.second;
        }
    }
    activePages.clear();
    currentIndex = 0;
    summaryBox->append("\n--- Restarted ---\n");
    displayStep(0);
    startBtn->setEnabled(true);
    pauseBtn->setEnabled(false);
}

void MainWindow::adjustSpeed(int value) {
    timerIntervalMs = value;
    if (animTimer->isActive()) {
        animTimer->setInterval(timerIntervalMs);
    }
}
