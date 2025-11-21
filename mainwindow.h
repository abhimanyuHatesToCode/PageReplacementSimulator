#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSlider>
#include <QTimer>
#include <map>

#include "PageItem.h"
#include "simulators.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override {}

private slots:
    void runSimulation();
    void startAnimation();
    void pauseAnimation();
    void nextStep();
    void prevStep();
    void restartAnimation();
    void adjustSpeed(int value);
    void onTimerTick();

private:
    void setupUI();
    void drawFrames(int count);
    void displayStep(int idx);
    void clearScene();

    // UI
    QGraphicsScene *scene;
    QGraphicsView *view;

    QComboBox *algoBox;
    QComboBox *framesBox;
    QLineEdit *inputField;
    QPushButton *runButton;

    QPushButton *startBtn;
    QPushButton *pauseBtn;
    QPushButton *nextBtn;
    QPushButton *prevBtn;
    QPushButton *restartBtn;
    QSlider *speedSlider;

    QTextEdit *summaryBox;

    // Simulation data
    ResultSummary currentResult;
    std::vector<SimulationStep> steps;
    int currentIndex = 0;
    int frameCount = 3;

    // Animation and display
    QTimer *animTimer;
    int timerIntervalMs = 800;

    // Frame UI elements
    std::vector<QGraphicsRectItem*> frameRects;
    std::vector<QPointF> frameTargets;

    // Active page items by page number
    std::map<int, PageItem*> activePages;
};

#endif // MAINWINDOW_H
